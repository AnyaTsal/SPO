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
#include <string.h>

const int MAX_SIZE_PROCCESS = 5;
const int SIZE = 20;

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


int main(int argc,char* argv[]){
	char str[20];
	int id = atoi(argv[0]),sem_id = atoi(argv[1]),pipe = atoi(argv[2]);

	memset(str, '\0', SIZE);
	sprintf(str, "Client#%d", id);
	
	while(true){
		if(WaitSemaphore(sem_id,id)!= -1){
			write(pipe,str,sizeof(str));
			ReleaseSemaphore(sem_id,10);			
		}
		if(WaitSemaphore(sem_id,id + MAX_SIZE_PROCCESS)!= -1){
			break;
		}
	}
	return 0;
}