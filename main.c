#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#define BUFSIZE 1024
#define SHM_KEY 0x1234

int main( int argc, char **argv)
{
	int option, max_child = 5, max_time = 20, con_proc = 20, counter = 0;
	char file[64], *exec[] = {"./exe", NULL};
	pid_t child = 0;

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
	
	if (con_proc > max_child)
		con_proc = max_child;
	

	while (max_child > 0)
	{
		if (con_proc == counter)
		{
			wait(NULL);
			counter--;
		}
		counter++;

		if ((child = fork()) == 0)
			execvp(exec[0], exec);

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
			while(wait(NULL) >0);
	}

	printf("Finished\n");
}
