#include <stdlib.h>
#include <sys/sem.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <vector>
#include <iostream>
#include <string>

const int MAX_SIZE_PROCESS = 5;
std::vector<pid_t> clients;
int sem_id;
char semID[20];
int pipeWrite;

int WaitSemaphore(int sem_id, int num)
{
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_flg = IPC_NOWAIT;
    buf.sem_num = num;
    return semop(sem_id, &buf, 1);
}

int ReleaseSemaphore(int sem_id, int num)
{
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_flg = IPC_NOWAIT;
    buf.sem_num = num;
    return semop(sem_id, &buf, 1);
}

bool kbhit()
{
   termios term;
   tcgetattr(0, &term);

   termios term2 = term;
   term2.c_lflag &= ~ICANON;
   tcsetattr(0, TCSANOW, &term2);

   int byteswaiting;
   ioctl(0, FIONREAD, &byteswaiting);

   tcsetattr(0, TCSANOW, &term);

   return byteswaiting > 0;
}

void addClient(int id){
	pid_t pid = fork();
	char ID[20],pipe[20];
	sprintf(semID,"%d",sem_id);
	sprintf(ID,"%d",id);
	sprintf(pipe,"%d",pipeWrite);

	if(pid < 0){
		printf("Error creating client");
	}
	else if(pid > 0){
		sleep(1);
		execlp("./client", ID,semID,pipe,NULL);
		printf("Error.");
	}
	else{
		clients.push_back(pid);
		if(clients.size() == 1)
			ReleaseSemaphore(sem_id,11);
	}
}

void removeClient(int id){
	ReleaseSemaphore(sem_id,id + MAX_SIZE_PROCESS);
	waitpid(clients.back(),NULL,0);
	clients.pop_back();
	if(id == 0) while(WaitSemaphore(sem_id,11)!= 0);
}

int main(int argc,char* argv[]){
	
	unsigned int id = -1,i = -1;
	char key;
	sem_id = atoi(argv[0]);
	pipeWrite = atoi(argv[1]);

	while(true){
		if(clients.size() != 0){ 
			if(WaitSemaphore(sem_id,11)!= -1){
				if(i == clients.size()-1)
					i = -1;

				ReleaseSemaphore(sem_id,++i);		
			}
		}
		if(kbhit()){
			key = getchar();
			if(key == '+' && clients.size() < MAX_SIZE_PROCESS ){
				addClient(++id);
			}	
			else if(key == '-' && !clients.empty()){
				if(i == clients.size() -1)
					i = -1;
				removeClient(id--);
			}
			else if(key == 'q'){
				while(!clients.empty())
					removeClient(id--);
				ReleaseSemaphore(sem_id,12);
				break;	
			}
		}
	}
	
	return 0;
}