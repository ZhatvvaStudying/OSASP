#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>

#define SLEEP_TIME 100 * 1000
#define CHILDREN_COUNT 2
#define GMT_OFFSET 3
#define TO_GROUP 0


int counter = 1;
pid_t children[CHILDREN_COUNT];

void PrintMsgAndExit(char *str)
{
    perror(str);
    exit(-1);
}

struct sigaction CreateAction(void (*handler) (int, siginfo_t*, void*), sigset_t signals, int flags)
{
    struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_sigaction = handler;	
	action.sa_mask = signals;
	action.sa_flags = flags;
    return action;
}

suseconds_t GetCurrentTime() 
{
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) == -1) 
    {
		perror("Cannot get current time\n");
		return -1;
	}	
		
	return tv.tv_sec * 1000 + tv.tv_usec / 1000; 
}

int GetChildNumber(pid_t pid)
{
    int childNum = -1;
    for (int i = 0; i < CHILDREN_COUNT; ++i)
    {
        if (children[i] == pid)
        {
            childNum = i + 1;
            break;
        }
    }
    return childNum;
}

void ParentHandler(int sig, siginfo_t *siginfo, void *code) 
{	
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("[%d] PID: %d PPID: %d Time: %ld PARENT get SIGUSR2 from CHILD with PID %d\n", counter++, getpid(), getppid(), GetCurrentTime(), siginfo->si_pid);
	usleep(SLEEP_TIME);
	
	kill(TO_GROUP, SIGUSR1);
}

void ChildHandler(int sig, siginfo_t *siginfo, void *code) 
{
	pid_t pid = getpid();
	pid_t ppid = getppid();

    int childNum = GetChildNumber(pid);

	printf("[%d] PID: %d PPID: %d Time: %ld CHILD%d get SIGUSR1\n", counter++, pid, ppid, GetCurrentTime(), childNum);
	
	kill(ppid, SIGUSR2);
}

void main(void){

	sigset_t childSignalSet, parentSignalSet;

	if (sigemptyset(&childSignalSet) == -1) 
    {
		PrintMsgAndExit("Cannot create child signal set\n");
	}
	if (sigaddset(&childSignalSet, SIGUSR1) == -1) 
    {
		PrintMsgAndExit("Cannot add SIGUSR1 to child signal set\n");
	}
	
	if (sigemptyset(&parentSignalSet) == -1) 
    {
		PrintMsgAndExit("Cannot create parent signal set\n");
	}
	if (sigaddset(&parentSignalSet, SIGUSR2) == -1) 
    {
		PrintMsgAndExit("Cannot add SIGUSR2 to parent signal set\n");
	}
	
	struct sigaction parentAction = CreateAction(ParentHandler, parentSignalSet, SA_SIGINFO);

	if (sigaction(SIGUSR2, &parentAction, 0) == -1) 
    {
		PrintMsgAndExit("Cannot set parent SIGUSR2 signal handler\n");
	}
	
	struct sigaction childAction = CreateAction(ChildHandler, childSignalSet, SA_SIGINFO);
		
	for (int i = 0; i < CHILDREN_COUNT; i++) 
    {
		children[i] = fork();
		switch (children[i]) 
        {
			case -1:
				PrintMsgAndExit("Cannot create a child process\n");
            
            case 0:
				children[i] = getpid();
				if (sigprocmask(SIG_SETMASK, &parentSignalSet, 0) == -1) 
                {
					PrintMsgAndExit("Cannot set child blocked signal list\n");
				}

				if (sigaction(SIGUSR1, &childAction, 0) == -1) 
                {
					PrintMsgAndExit("Cannot set child SIGUSR1 signal hanlder\n");
				}
				
                printf("PID: %d PPID: %d Time: %ld CHILD%d <CREATED>\n", getpid(), getppid(), GetCurrentTime(), i + 1);
				
                while (1) {};
		}
	}
	
	printf("PID: %d PPID: %d Time: %ld PARENT\n", getpid(), getppid(), GetCurrentTime());
	if (sigprocmask(SIG_SETMASK, &childSignalSet, 0) == -1) 
    {
		PrintMsgAndExit("Cannot set parent blocked signal list\n");
	}
	
	sleep(1);
	
	if (kill(TO_GROUP, SIGUSR1) == -1) 
    {
		PrintMsgAndExit("Cannot send signal\n");
	}
	
	while (1) {}
	return;
}