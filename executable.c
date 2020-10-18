#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types>
#include <signal.h>

#define SEC_KEY 0x1234
#define MSG_KEY 0x2345

typedef struct Clock
{
        int sec;
        int nsec;
} Clock;

typedef struct msgbuf
{
        long mtype;
        char mtext[10];
};


int main()
{
	int shmid, msgqid;
	struct Clock *clock, msgbuf msgbuf;
        msgqid = msgget(MSG_KEY, 0644 | IPC_CREAT);
        if (msgqid == -1)
        {
                perror("shmid get failed");
                return 1;
        }


	shmid = shmget(SEC_KEY, sizeof(Clock), 0644 | IPC_CREAT);
        if (shmid == -1)
        {
                perror("shmid get failed");
                return 1;
        }

        clock = (Clock *) shmat(shmid, NULL, 0);
        if (clock == (void *) -1)
        {
                perror("clock get failed");
                return 1;
        }
	
	printf("Child clock seconds: %d\n", clock->sec);
	sleep(2);
}

