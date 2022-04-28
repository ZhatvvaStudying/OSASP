#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#define GMT_OFFSET 3

void PrintTimeAndPID(const char* name)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	int msecs = time.tv_usec / 1000;
	int secs  = time.tv_sec % 60;
	int mins  = (time.tv_sec / 60) % 60;
	int hrs   = (time.tv_sec / 3600 + GMT_OFFSET) % 24; 
    pid_t processPID = getpid(), parentPID = getppid();
	printf("Process name: %s PID: %d Parent PID: %d Time: %02d:%02d:%02d:%03d\n\n", name, processPID, parentPID,
		hrs, mins, secs, msecs );
}


int WaitForCompletion(pid_t childPID)
{
	if (childPID > 0)
	{
		waitpid(childPID, NULL, 0);
	}
}

int CreateAndHandleProcess(const char* processName)
{
    int result;
    switch (result = fork()) 
    {
        case -1:
            perror("Error during attempt to start another process\n");
            return 0;
        case 0:
            PrintTimeAndPID(processName);
            exit(0);
        default:
            return result;
    }
}

int main()
{
	pid_t firstChildPID = CreateAndHandleProcess("first child"), secondChildPID = CreateAndHandleProcess("second child");
    system("ps -x");
    WaitForCompletion(firstChildPID);
    WaitForCompletion(secondChildPID);
    PrintTimeAndPID("parent");    
	return 0;
}