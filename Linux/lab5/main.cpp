#include <iostream>
#include <dlfcn.h>
#include <vector>

//#include <string>

using namespace std;

typedef void (*PerformConcatFunction)(std::vector<std::string> source_files, std::string output_file_path);

int main()
{

    int freeResult, runTimeLinkSuccess = 0;

    void* soHandle = NULL;

    PerformConcatFunction PerformConcat = NULL;

    //Загрузить библиотеку .so и сохранить её дескриптор

    //все необходимые перемещения выполнены при первой загрузке объекта, все символы ссылки будут доступны во время выполнения
    soHandle = dlopen("libasyncrw.so", RTLD_NOW);//путь до библиотеки


    //Если дескриптор действительный
    //if (NULL != soHandle)
    if (soHandle != NULL)
    {

        //Получить указатель на функцию:
        PerformConcat = (PerformConcatFunction)dlsym(soHandle, "PerformConcat");//возвращает адрес, откуда загружается PerformConcat


        //Вызвать функцию, если она действительная
        if (runTimeLinkSuccess = (PerformConcat != NULL))
        {

            vector<string> file_names;

            file_names.push_back(string("read_files/first.txt"));
            file_names.push_back(string("read_files/second.txt"));
            file_names.push_back(string("read_files/third.txt"));
            file_names.push_back(string("read_files/fourth.txt"));
            file_names.push_back(string("read_files/fifth.txt"));
            file_names.push_back(string("read_files/1.z"));

            cout << "Начало работы..." << endl;

            PerformConcat(file_names, "output.txt");

            cout << "Работа завершена..." << endl;
            cout << "Для выхода нажмите любую клавишу..." << endl;

            cin.ignore();
        }

        //Выгрузить библиотеку из памяти:

        freeResult = dlclose(soHandle);

    }

    if (!runTimeLinkSuccess)

        cout << "Ошибка вызова .so функции" << endl;

    if(soHandle == NULL)

        cout << dlerror() << endl;


    return 0;
}
