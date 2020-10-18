#include <getopt.h>
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

#define BUFSIZE 1024
#define SEC_KEY 0x1234
#define MSG_KEY 0x2345

struct msgbuf
{
	long mtype;
	char mtext[100];
} message;

typedef struct Clock
{
	int sec;
	long nsec;
} Clock;

int shmid, msgqid;
struct Clock *clock;

void sigint(int sig)
{
	if (msgctl(msgqid, IPC_RMID, 0) < 0)
	{
		perror("msgctl");
		exit(0);
	}

        shmdt(clock);
        shmctl(shmid, IPC_RMID, NULL);

	write(1, "\nInterrupt!\n", 12);
	write(1, "Now killing the kiddos\n", 23);
	kill(0, SIGQUIT);
	exit(0);
}

int main( int argc, char **argv)
{
	signal(SIGINT, sigint);

	int option, max_child = 5, max_time = 20, con_proc = 15, counter = 0;
	char file[64], *exec[] = {"./exe", NULL};
	pid_t child = 0;

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

	msgqid = msgget(MSG_KEY, 0644 | IPC_CREAT);
        if (msgqid == -1)
        {
                perror("shmid get failed");
                return 1;
        }

	if (argc < 2)
	{
		printf("Invalid usage! Check the readme\nUsage: oss [-c x] [-f filename] [-t time]\n"); 
		return 0;
	}

	while ((option = getopt(argc, argv, "hc:f:t:")) != -1)
	switch (option)
	{
		case 'h':
			printf("This is the operating system simulator!\n");
			printf("Usage: oss [-c x] [-f filename] [-t x]\n");
			printf("-c is the maximum number of processes to be run\n");
			printf("-f is the filename of the program to be run\n");
			printf("-t is the maximum amount of time before the simulator ends\n");
			printf("Enjoy!\n");
			return 0;
		case 'c':
			max_child = atoi(optarg);
			break;
		case 'f':
			strcpy(file, optarg);
			break;
		case 't':
			max_time = atoi(optarg);
			break;
		case '?':
			printf("%c is not an option. Use -h for usage details\n", optopt);
			return 0;
	}

	printf("You have chosen the following options: -c %d -f %s -t %d\n", max_child, file, max_time);

        message.mtype = 1;
	strcpy(message.mtext,"1");
	msgsnd(msgqid, &message, sizeof(message), 0);	

	if (con_proc > max_child)
		con_proc = max_child;

	alarm(max_time);
	signal(SIGALRM, sigint);

	while (max_child > 0)
	{
		if (con_proc == counter)
		{
			wait(NULL);
			counter--;
		}
		counter++;

		if ((child = fork()) == 0)
		{
			execvp(exec[0], exec);
			perror("exec failed");
		}

		if (child < 0)
		{
			perror("Failed to fork");
			exit(1);
		}

		if (waitpid(-1, NULL, WNOHANG) > 0)
			counter--;
		max_child--;
	}
	if (child > 0)
	{
			printf("PARENT!\n");
			while (max_child > 0)
			{
				printf("Parent clock seconds: %d", clock->sec++);
				max_child--;
				sleep(1);
			}
			while(wait(NULL) > 0);
	}

	printf("Clock: %d\n", clock->sec);

        if (msgctl(msgqid, IPC_RMID, 0) < 0)
        {
                perror("msgctl");
                return 1;
        }
	shmdt(clock);
	shmctl(shmid, IPC_RMID, NULL);

	printf("Finished\n");
}
