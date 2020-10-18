#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>

#define SEC_KEY 0x1234
#define MSG_KEY 0x2345

typedef struct Clock
{
        int sec;
        long nsec;
} Clock;

struct msgbuf
{
        long mtype;
        char mtext[100];
} message;


int main()
{
	int shmid, msgqid;
	struct Clock *clock;

        msgqid = msgget(MSG_KEY, 0644 | IPC_CREAT);
        if (msgqid == -1)
        {
                perror("msgqid get failed");
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
	
//	msgrcv(msgqid, &message, sizeof(message), 1, 0);	
	
	while(strcmp(message.mtext, "1") != 0)
	{	
		msgrcv(msgqid, &message, sizeof(message), 1, IPC_NOWAIT);

		if (strcmp(message.mtext, "1") == 0)
		{
			
//			printf("Child clock seconds: %d\n", ++clock->sec);
//			printf("Message from queue: %s\n", message.mtext);
			printf("In the critical section!\n");
			strcpy(message.mtext, "1");
			sleep(2);
			msgsnd(msgqid, &message, sizeof(message), 0);
//			sleep(2);	
			break;
		}
		printf("Waiting for my turn (%ld)...\n", (long) getpid());
		printf("Child clock seconds: %d\n", ++clock->sec);
		sleep(1);
	}
	printf("Child %ld is finished!\n", (long) getpid());
}

