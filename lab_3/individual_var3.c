#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

long maxProcessesCount;
long currentProcessesCount;

char *firstDirectoryPath;
char *secondDirectoryPath;

long ParseLong(const char *str, bool *isError)
{
    errno = 0;
    char *error;
    long val = strtol(str, &error, 0);
    if (error == str || *error != '\0' || ((val == LONG_MIN || val == LONG_MAX) && errno == ERANGE))
    {
        fprintf(stderr, "Could not convert '%s' to long or value is out of range\n", str);
        *isError = true;
    }
    else
    {
        *isError = false;
    }
    return val;
}

char *GetFullPath(struct dirent *de, const char *path)
{
    char *result = malloc((strlen(de->d_name) + strlen(path) + 2) * sizeof(char));
    strcpy(result, path);
    strcat(result, "/");
    strcat(result, de->d_name);
    return result;
}

int CompareFiles(char *firstFileName, char *secondFileName)
{
	FILE* aFile = fopen(firstFileName, "r");
	if (!aFile)
    {
        perror("Cannot open first file");
        return -1;
    }
		
	FILE* bFile  = fopen(secondFileName, "r");
	if (!bFile)
    {
        perror("Cannot open second file");
        return -1;
    }
		
	char aSymb, bSymb; 
    int symbCount = 0; 
	while ((aSymb = fgetc(aFile)) == (bSymb = fgetc(bFile)))
	{
		symbCount++;
		if ((aSymb == EOF) && (aSymb== EOF))
		{
			printf("Process %d: %s == %s (bytes readed: %d)\n", getpid(), firstFileName, secondFileName, symbCount);
			return 0;
		}
	}
	
    printf("Process %d: %s != %s (comparison stopped at byte %d)\n", getpid(), firstFileName, secondFileName, symbCount);
	return 0;
}

int LaunchComparison(struct dirent *firstDirent, struct dirent *secondDirent)
{
	if (currentProcessesCount >= maxProcessesCount)
	{
		if (wait(NULL) == -1)
		{
			perror("Cannot wait for process");
			return -1;
		}
		currentProcessesCount--;
	}
	currentProcessesCount++;
	
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Cannot create a fork");
        return -1;
    }
	if (pid == 0)
	{
		char *firstFileFullPath = GetFullPath(firstDirent, firstDirectoryPath);
        char *secondFileFullPath = GetFullPath(secondDirent, secondDirectoryPath); 
        CompareFiles(firstFileFullPath, secondFileFullPath);
        free(firstFileFullPath);
        free(secondFileFullPath);
        exit(0);
	}	

    return 0;
}

void WaitForAllProcesses()
{
	for (int i = currentProcessesCount; i > 0; i--)
	{
		if (wait(NULL) == -1)
		{
			perror("Cannot wait for process");
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc != 4) 
    {
        fprintf(stderr, "Error! Invalid operands count!\n\t1 - First directory name\n\t2 - Second directory name\n\t3 - max processes count\n");
        return -1;
    }

	bool isError;
    maxProcessesCount = ParseLong(argv[3], &isError);
    if (isError)
    {
        return -1;
    }

    if (maxProcessesCount <= 0) 
    {
        fprintf(stderr, "Max processes count should be positive\n");
        return -1;
    }
	
	DIR *firstDir = opendir(argv[1]);  
	if (!firstDir)
    {
        perror("Cannot open first directory");
        return -1;
    }
		
	DIR *secondDir = opendir(argv[2]);  
	if (!secondDir) 
	{
		perror("Cannot open second directory");
        return -1;	
	}

    firstDirectoryPath = malloc((strlen(argv[1]) + 1) * sizeof(char));
    secondDirectoryPath = malloc((strlen(argv[2]) + 1) * sizeof(char));	
	strcpy(firstDirectoryPath, argv[1]);
	strcpy(secondDirectoryPath, argv[2]);
			
	currentProcessesCount = 0;
    for (struct dirent *firstDirent = readdir(firstDir); firstDirent != NULL; firstDirent = readdir(firstDir))
    {
        if (firstDirent->d_type == DT_REG)
        {
            for (struct dirent *secondDirent = readdir(secondDir); secondDirent != NULL; secondDirent = readdir(secondDir))
            {
                if (secondDirent->d_type == DT_REG)
                {
                    LaunchComparison(firstDirent, secondDirent);
                }
            }
            rewinddir(secondDir);
        }
    }
				
	WaitForAllProcesses();
	free(firstDirectoryPath);
    free(secondDirectoryPath);

	if (closedir(firstDir))
    {
        perror("Cannot close first directory");
    }
	if (closedir(secondDir))
    {
        perror("Cannot close second directory");
    }
		
	return 0;
}

