#include "AsyncReadWrite.h"

#include <aio.h>
#include <iostream>
#include <csignal>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <exception>
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <signal.h>

using namespace async_read_write;
using namespace std;

short AsyncReadWrite::kSetArray[] = { 0, 0 };
const int AsyncReadWrite::kWriteSemaphoreIndex = 1;
const int AsyncReadWrite::kReadSemaphoreIndex = 0;


AsyncReadWrite::AsyncReadWrite()
{
   semaphoreKey = ftok("/dev", 30);             //каталог dev

   if (semaphoreKey == -1)
   {
       throw runtime_error("ftok() ошибка...");
   }

   semaphoreId = CreateSemaphoreSet(semaphoreKey);
   if (semaphoreId == -1)
   {
      throw runtime_error("Ошибка создания семафора...");
   }
}

int AsyncReadWrite::CreateSemaphoreSet(key_t key)
{
    int id;
    int check = 0;

    id = semget(key , 2,  0666 | IPC_CREAT);//2 семафора
    if (id != -1)
    {
        check = semctl(id, 0, SETALL,const_cast<short*>(this->kSetArray));        //4-{0,0}
    }

    return (check == -1) ? check : id;
}

AsyncReadWrite::~AsyncReadWrite()
{
   DeleteSemaphoreSet(this->semaphoreId);
}

void AsyncReadWrite::DeleteSemaphoreSet(int semid)
{
    semctl(semid, 0, IPC_RMID, NULL);
}

void AsyncReadWrite::ReleaseData()
{
    for(auto &read_data : this->read_files_data_)
    {
        delete read_data->data;
        delete read_data;
    }

    for (auto &threadData : this->thread_data_vector_)
    {
        delete threadData;
    }
    this->thread_data_vector_.clear();

    this->read_files_data_.clear();

    for (auto &read_data : this->read_files_data_)
    {
        delete read_data;
    }
    this->read_files_data_.clear();

    this->reading_threads_.clear();

    this->writing_thread_ = 0;
}

int AsyncReadWrite::ConcatFiles(std::vector<std::string> source_files, std::string output_file_path)
{
    this->output_file_ = output_file_path;
    this->source_file_names_ = source_files;

    int number = 0;
    int offset = 0;

    for (auto &file_name : this->source_file_names_)                 //обход по каждому файлу вектора
    {
        this->thread_data_vector_.push_back(new aiocb);
        aiocb empty = { 0 };
        *this->thread_data_vector_.back() = empty;
        FILE* file = fopen(file_name.c_str(), "r");                  //открыть файл для чтения
        if(file == NULL)
        {
            cout << "Ошибка загрузки файла..." << endl;
            continue;
        }
        this->thread_data_vector_.back()->aio_fildes = fileno(file); //заполняем структуру дескриптором файла указанного потока(file)

        struct stat file_info = { 0 };
        fstat(thread_data_vector_.back()->aio_fildes, &file_info);   //получение структуры stat с информацией о файле

	if(file_info.st_size == 0)                                       //общий размер в байтах
	{
		delete this->thread_data_vector_.back();
		this->thread_data_vector_.pop_back();
		fclose(file);
		continue;
	}
        //Заполнение структуры ReadFilesData:
        this->read_files_data_.push_back(new ReadFileData);
        this->read_files_data_.back()->offset = offset;
        this->read_files_data_.back()->size = file_info.st_size;
        this->read_files_data_.back()->data = new char[file_info.st_size];
        //memset( this->read_files_data_.back()->data, 0, sizeof(char));//
        this->read_files_data_.back()->data[0] = '\0';

        this->thread_data_vector_.back()->aio_buf = this->read_files_data_.back()->data;                //расположение буфера чтения/записи
        this->thread_data_vector_.back()->aio_nbytes = file_info.st_size;                               //длина передаваемой последовательности
        this->thread_data_vector_.back()->aio_offset = 0;                                               //смещение

        this->thread_data_vector_.back()->aio_sigevent.sigev_notify = SIGEV_THREAD;                     //уведомить процесс с пом-щью вызова sigev_notify_function
        this->thread_data_vector_.back()->aio_sigevent.sigev_notify_function = AsyncReadWrite::ReadEndRoutine;//обработчик семафора
        this->thread_data_vector_.back()->aio_sigevent.sigev_notify_attributes = NULL;
        this->thread_data_vector_.back()->aio_sigevent.sigev_value.sival_int = this->semaphoreId;       //аргумент sigev_notify_function

        pthread_t id;
        pthread_create(&id, NULL, AsyncReadWrite::Read, reinterpret_cast<void*>(this->thread_data_vector_.back()));
        this->reading_threads_.push_back(id);   //добавляем новый поток чтения в вектор потоков

        offset += file_info.st_size;
        number++;
    }
    pthread_create(&this->writing_thread_, NULL, AsyncReadWrite::Write, reinterpret_cast<void*>(this));

    pthread_join(this->writing_thread_, NULL);

    this->ReleaseData();    //освобождение памяти

    return this->thread_data_vector_.size();    //функция возврвщает размер результирующего файла
}

void* AsyncReadWrite::Write(void* arg)
{
    AsyncReadWrite* caller_instance = reinterpret_cast<AsyncReadWrite*>(arg);

    vector<aiocb*> aio_data;

    int files_to_write = caller_instance->source_file_names_.size();            //число файлов для чтения

    FILE* output_file = fopen(caller_instance->output_file_.c_str(), "w");      //открытие результирующего файла для записи

    while (files_to_write)
    {
        //Ожидаем, пока хотя бы один поток чтения завершится
        sembuf semaphoreSetr = { 0 };

        semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex; //номер семафора в наборе
        semaphoreSetr.sem_op = -1;                                    //уменьшить счётчик семафора на 1
        semaphoreSetr.sem_flg = 0;
        semop(caller_instance->semaphoreId, &semaphoreSetr, 1);

        semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex;
        semaphoreSetr.sem_op = 1;
        semaphoreSetr.sem_flg = 0;
        semop(caller_instance->semaphoreId, &semaphoreSetr, 1);


        for (auto &element : caller_instance->read_files_data_)     //обход по структурам (смещение, размер данных, данные)
        {
            if(element->data[0] != '\0')                            //если строка не пустая
            {
                aiocb empty = { 0 };
                aio_data.push_back(new aiocb);                      //структура для асинхронного ввода/вывода
                *aio_data.back() = empty;
                aio_data.back()->aio_buf = element->data;
                aio_data.back()->aio_fildes = fileno(output_file);
                aio_data.back()->aio_nbytes = element->size;
                aio_data.back()->aio_offset = element->offset;

                aio_data.back()->aio_sigevent.sigev_notify = SIGEV_THREAD;//уведомить процесс с пом-щью вызова sigev_notify_function
                aio_data.back()->aio_sigevent.sigev_notify_function = AsyncReadWrite::WriteEndRoutine;
                aio_data.back()->aio_sigevent.sigev_notify_attributes = NULL;
                aio_data.back()->aio_sigevent.sigev_value.sival_int = caller_instance->semaphoreId;//аргумент sigev_notify_function

                aio_write(aio_data.back());//старт асинхронной записи

                //Уменьшаем счётчик семафора чтения на 1 (счетчик семафора чтения наращивался при завершении чтения каждым потоком)
                semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex;
                semaphoreSetr.sem_op = -1;
                semaphoreSetr.sem_flg = 0;
                semop(caller_instance->semaphoreId, &semaphoreSetr, 1);

                files_to_write--;

                //element->data[0] = '\0';
            }
        }
    }

    sembuf semaphoreSetw = { 0 };
    semaphoreSetw.sem_num = caller_instance->kWriteSemaphoreIndex;
    semaphoreSetw.sem_op = (int)((-1) * caller_instance->thread_data_vector_.size());
    semaphoreSetw.sem_flg = 0;
    semop(caller_instance->semaphoreId, &semaphoreSetw, 1);

    for (auto &data : aio_data)
    {
        delete data;
    }

    fclose(output_file);

    return 0;
}

void AsyncReadWrite::WriteEndRoutine(sigval_t arg)
{
    sembuf semaphoreSet = { 0 };
    semaphoreSet.sem_num = AsyncReadWrite::kWriteSemaphoreIndex;
    semaphoreSet.sem_op = 1;
    semaphoreSet.sem_flg = 0;
    semop(arg.sival_int, &semaphoreSet, 1);//1-id
}

void AsyncReadWrite::ReadEndRoutine(sigval_t arg)
{
    sembuf semaphoreSet = { 0 };
    semaphoreSet.sem_num = AsyncReadWrite::kReadSemaphoreIndex;
    semaphoreSet.sem_op = 1;
    semaphoreSet.sem_flg = 0;
    semop(arg.sival_int, &semaphoreSet, 1);
}

void* AsyncReadWrite::Read(void* arg)
{
    aiocb* thisAiocb = reinterpret_cast<aiocb*>(arg);

    aio_read(thisAiocb);        //старт асинхронного чтения

    return 0;
}

extern "C" void PerformConcat(std::vector<std::string> source_files, std::string output_file_path)
{
	AsyncReadWrite* object = new AsyncReadWrite;
	object->ConcatFiles(source_files, output_file_path);
	delete object;	
}

/*#include "AsyncReadWrite.h"
#include <aio.h>
#include <iostream>
#include <csignal>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <exception>
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <signal.h>

using namespace async_read_write;
using namespace std;

short AsyncReadWrite::kSetArray[] = { 0, 0 };
const int AsyncReadWrite::kWriteSemaphoreIndex = 1;
const int AsyncReadWrite::kReadSemaphoreIndex = 0;


AsyncReadWrite::AsyncReadWrite()
{
    semaphoreKey = ftok("/dev/null", 38);
    if (semaphoreKey == (key_t)-1)
    {
        throw runtime_error("ftok() error...");
    }

    semaphoreId = CreateSemaphoreSet(semaphoreKey);
    if (semaphoreId == -1)
    {
        throw runtime_error("Semaphore creation error...");
    }
}

int AsyncReadWrite::CreateSemaphoreSet(key_t key)
{
    int id;
    int check = 0;

    id = semget(key , 2,  0666 | IPC_CREAT);
    if (id != -1)
    {
        check = semctl(id, 0, SETALL,
                       const_cast<short*>(this->kSetArray));        // SETALL ignores
        // semnum (second) argument
    }

    return (check == -1) ? check : id;
}

AsyncReadWrite::~AsyncReadWrite()
{
    DeleteSemaphoreSet(this->semaphoreId);
}

void AsyncReadWrite::DeleteSemaphoreSet(int semid)
{
    semctl(semid, 0, IPC_RMID, NULL);
}

void AsyncReadWrite::ReleaseData()
{
    for(auto &read_data : this->read_files_data_)
    {
        delete read_data->data;
        delete read_data;
    }

    for (auto &threadData : this->thread_data_vector_)
    {
        delete threadData;
    }
    this->thread_data_vector_.clear();

    this->read_files_data_.clear();

    for (auto &read_data : this->read_files_data_)
    {
        delete read_data;
    }
    this->read_files_data_.clear();

    this->reading_threads_.clear();

    this->writing_thread_ = 0;
}

int AsyncReadWrite::ConcatFiles(std::vector<std::string> source_files, std::string output_file_path)
{
    this->output_file_ = output_file_path;
    this->source_file_names_ = source_files;

    int number = 0;
    int offset = 0;
    for (auto &file_name : this->source_file_names_)
    {

        this->thread_data_vector_.push_back(new aiocb);
        aiocb empty = { 0 };
        *this->thread_data_vector_.back() = empty;
        FILE* file = fopen(file_name.c_str(), "r");
        if(file == NULL)
        {
            cout << "Error while loading file..." << endl;
            continue;
        }
        this->thread_data_vector_.back()->aio_fildes = fileno(file);

        struct stat file_info = { 0 };
        fstat(thread_data_vector_.back()->aio_fildes, &file_info);

        if(file_info.st_size == 0)
        {
            delete this->thread_data_vector_.back();
            this->thread_data_vector_.pop_back();
            fclose(file);
            continue;
        }

        this->read_files_data_.push_back(new ReadFileData);;
        this->read_files_data_.back()->offset = offset;
        this->read_files_data_.back()->size = file_info.st_size;
        this->read_files_data_.back()->data = new char[file_info.st_size];
        memset( this->read_files_data_.back()->data, 0, sizeof(char));
        this->read_files_data_.back()->data[0] = '\0';

        this->thread_data_vector_.back()->aio_buf = this->read_files_data_.back()->data;
        this->thread_data_vector_.back()->aio_nbytes = file_info.st_size;
        this->thread_data_vector_.back()->aio_reqprio = 0;
        this->thread_data_vector_.back()->aio_offset = 0;
        this->thread_data_vector_.back()->aio_sigevent.sigev_notify = SIGEV_THREAD;
        this->thread_data_vector_.back()->aio_sigevent.sigev_notify_function = AsyncReadWrite::ReadEndRoutine;
        this->thread_data_vector_.back()->aio_sigevent.sigev_notify_attributes = NULL;
        this->thread_data_vector_.back()->aio_sigevent.sigev_value.sival_int = this->semaphoreId;

        pthread_t id;
        pthread_create(&id, NULL, AsyncReadWrite::Read, reinterpret_cast<void*>(this->thread_data_vector_.back()));
        this->reading_threads_.push_back(id);

        offset += file_info.st_size;
        number++;
    }
    pthread_create(&this->writing_thread_, NULL, AsyncReadWrite::Write, reinterpret_cast<void*>(this));

    void** threadReciever = new void*;
    pthread_join(this->writing_thread_, threadReciever);

    this->ReleaseData();

    return this->thread_data_vector_.size();
}

void* AsyncReadWrite::Write(void* arg)
{
    AsyncReadWrite* caller_instance = reinterpret_cast<AsyncReadWrite*>(arg);

    vector<aiocb*> aio_data;

    int files_to_write = caller_instance->source_file_names_.size();

    FILE* output_file = fopen(caller_instance->output_file_.c_str(), "w");

    while (files_to_write)
    {
        sembuf semaphoreSetr = { 0 };
        semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex;
        semaphoreSetr.sem_op = -1;
        semaphoreSetr.sem_flg = 0;
        semop(caller_instance->semaphoreId, &semaphoreSetr, 1);

        semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex;
        semaphoreSetr.sem_op = 1;
        semaphoreSetr.sem_flg = 0;
        semop(caller_instance->semaphoreId, &semaphoreSetr, 1);

        for (auto &element : caller_instance->read_files_data_)
        {
            if(element->data[0] != '\0')
            {
                aiocb empty = { 0 };
                aio_data.push_back(new aiocb);
                *aio_data.back() = empty;
                aio_data.back()->aio_buf = element->data;
                aio_data.back()->aio_fildes = fileno(output_file);
                aio_data.back()->aio_nbytes = element->size;
                aio_data.back()->aio_reqprio = 0;
                aio_data.back()->aio_offset = element->offset;
                aio_data.back()->aio_sigevent.sigev_notify = SIGEV_THREAD;
                aio_data.back()->aio_sigevent.sigev_notify_function = AsyncReadWrite::WriteEndRoutine;
                aio_data.back()->aio_sigevent.sigev_notify_attributes = NULL;
                aio_data.back()->aio_sigevent.sigev_value.sival_int = caller_instance->semaphoreId;

                aio_write(aio_data.back());

                semaphoreSetr.sem_num = caller_instance->kReadSemaphoreIndex;
                semaphoreSetr.sem_op = -1;
                semaphoreSetr.sem_flg = 0;
                semop(caller_instance->semaphoreId, &semaphoreSetr, 1);
                files_to_write--;
            }
        }
    }

    sembuf semaphoreSetw = { 0 };
    semaphoreSetw.sem_num = caller_instance->kWriteSemaphoreIndex;
    semaphoreSetw.sem_op = (int)((-1) * caller_instance->thread_data_vector_.size());
    semaphoreSetw.sem_flg = 0;
    semop(caller_instance->semaphoreId, &semaphoreSetw, 1);

    for (auto &data : aio_data)
    {
        delete data;
    }

    fclose(output_file);

    return 0;
}

void AsyncReadWrite::WriteEndRoutine(sigval_t arg)
{
    sembuf semaphoreSet = { 0 };
    semaphoreSet.sem_num = AsyncReadWrite::kWriteSemaphoreIndex;
    semaphoreSet.sem_op = 1;
    semaphoreSet.sem_flg = 0;
    semop(arg.sival_int, &semaphoreSet, 1);
}

void AsyncReadWrite::ReadEndRoutine(sigval_t arg)
{
    sembuf semaphoreSet = { 0 };
    semaphoreSet.sem_num = AsyncReadWrite::kReadSemaphoreIndex;
    semaphoreSet.sem_op = 1;
    semaphoreSet.sem_flg = 0;
    semop(arg.sival_int, &semaphoreSet, 1);
}

void* AsyncReadWrite::Read(void* arg)
{
    aiocb* thisAiocb = reinterpret_cast<aiocb*>(arg);

    aio_read(thisAiocb);

    return 0;
}

extern "C" void PerformConcat(std::vector<std::string> source_files, std::string output_file_path)
{
    AsyncReadWrite* object = new AsyncReadWrite;
    object->ConcatFiles(source_files, output_file_path);
    delete object;
}*/