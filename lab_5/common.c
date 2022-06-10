#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 256
#define THREADS_COUNT 2

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

void Exit(char *message)
{
    perror(message);
    exit(0);
}

void *PrintTimeAndPID(void *data)
{
    
    char buffer[BUFFER_SIZE];
    struct timespec ts;
    
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        Exit("Cannot get time");
    }

    struct tm *time = localtime(&ts.tv_sec);
    if (time == NULL)
    {
        Exit("Cannot parse time");
    }

    if (strftime(buffer, BUFFER_SIZE, "%T", time) == 0)
    {
        Exit("Cannot write more symbols than buffer size");
    }

    printf("ID [%lu] PID [%d] Time [%s:%ld]\n", pthread_self(), getpid(), buffer, ts.tv_nsec / 1000000);

    return 0;
}

int main() 
{
    PrintTimeAndPID(NULL);
    pthread_t threads[THREADS_COUNT];

    for (int i = 0; i < THREADS_COUNT; ++i)
    {
        pthread_t thread;
        if (pthread_create(&thread, NULL, PrintTimeAndPID, NULL) != 0)
        {
            Exit("Cannot start thread");
        }
        threads[i] = thread;
    }

    for (int i = 0; i < THREADS_COUNT; ++i)
    {
        if (pthread_join(threads[i], NULL))
        {
            Exit("Cannot join thread");
        }
    }

    return 0;
}