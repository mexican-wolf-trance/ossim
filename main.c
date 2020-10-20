#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

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
	long long nsec;
	pid_t shmPID;
} Clock;

int shmid, msgqid;
struct Clock *sim_clock;

void sigint(int sig)
{
	printf("\nTime ended: %d seconds, %lli nanoseconds\n", sim_clock->sec, sim_clock->nsec);
	if (msgctl(msgqid, IPC_RMID, 0) < 0)
	{
		perror("msgctl");
		exit(0);
	}

        shmdt(sim_clock);
        shmctl(shmid, IPC_RMID, NULL);

	write(1, "Interrupt!\n", 12);
	write(1, "Now killing the kiddos\n", 23);
	kill(0, SIGQUIT);
	exit(0);
}

int main (int argc, char **argv)
{
	signal(SIGINT, sigint);

	int option, max_child = 5, max_time = 20, con_proc = 20, counter = 0, tot_proc = 0;
	char file[64], *exec[] = {"./user", NULL};
	pid_t child = 0;
	FILE *fp;

	shmid = shmget(SEC_KEY, sizeof(Clock), 0644 | IPC_CREAT);
	if (shmid == -1)
	{
		perror("shmid get failed");
		return 1;
	}

	sim_clock = (Clock *) shmat(shmid, NULL, 0);
	if (sim_clock == (void *) -1)
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

	sim_clock->shmPID = 0;

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

	if ((fp = fopen("log.out", "w")) == 0)
	{
               perror("log.out");
                sigint(0);
        }

	while(1)
	{
		msgrcv(msgqid, &message, sizeof(message), 1, IPC_NOWAIT);
		if (strcmp(message.mtext, "1") == 0)
		{
			if (sim_clock->shmPID)
			{
				fprintf(fp, "%s %ld %s %i%s%lli %s", "Child pid", (long) sim_clock->shmPID, "is terminating at system time", sim_clock->sec, ".", sim_clock->nsec, "\n");
				tot_proc++;
				max_child--;
				wait(NULL);
				sim_clock->shmPID = 0;
				counter--;
			}
			if (sim_clock->nsec >= 1000000000)
			{
				sim_clock->sec++;
				sim_clock->nsec = 0;
			}
			sim_clock->nsec += 500;
			
			if((sim_clock->sec == 2) || (tot_proc == 100) || (max_child == 0))
			{
				fclose(fp);
				break;
			}
		
			if ((con_proc != counter) && (max_child > 0))
			{
				fprintf(fp, "%s %li %s %i%s%lli%s", "Creating new child pid", (long) getpid(), "at my time", sim_clock->sec, ".", sim_clock->nsec, "\n");

				if ((child = fork()) == 0)
				{
		                        execvp(exec[0], exec);
		                        perror("exec failed");
				}
	
				if (child < 0)
				{
					perror("Failed to fork");
					sigint(0);
				}
				counter++;
	                }
			strcpy(message.mtext, "1");
                        msgsnd(msgqid, &message, sizeof(message), 0);

		}
	}
	if(sim_clock->sec == 2)
		printf("Clock finish! Two system seconds have passed! Check the log for details\n");
	if(tot_proc == 100)
		printf("100 Processes reach!Check the log for details.\n");
	if(max_child == 0);
		printf("All proccess finished! Check the log for details.\n");
	printf("Time: %i.%lli, Processes: %d\n", sim_clock->sec, sim_clock->nsec, tot_proc);
        if (msgctl(msgqid, IPC_RMID, 0) < 0)
        {
                perror("msgctl");
                return 1;
        }
	shmdt(sim_clock);
	shmctl(shmid, IPC_RMID, NULL);

	kill(0, SIGQUIT);
}
