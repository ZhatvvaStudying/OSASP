#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/vfs.h>

void SetFullPath(char *fullName, char *folder, char *dir)
{
	strcpy(fullName, folder);
	
	if (fullName[strlen(fullName) - 1] != '/')
    {
        strcat(fullName, "/");
    }
	
	strcat(fullName, dir);
}

long long GetDirSize(char* dirName, long capacity)
{
	DIR *dir;		
	struct dirent *de;
	struct stat buf;
	long long size = 0;
	char fullPath[1024];
		
	if ((dir = opendir(dirName)) == NULL)
    {
        perror("Cannot open directory");
    }
	else 
    {
		for (de = readdir(dir); de != NULL; de = readdir(dir))
        {
			if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
            {
                SetFullPath(fullPath, dirName, de->d_name);
                
                if (de->d_type == DT_DIR)
                {
                    size += GetDirSize(fullPath, capacity);
                }
                else
                {
                    if (stat(fullPath, &buf) == 0)
                    {
                        size += buf.st_size;
                    }
                    else
                    {
                        perror("Cannot get stats for ");
                    }
                }
            }
		}	
		printf("'%s'\t%lld bytes\t Disk usage %f%%\n", dirName, size, (float)size / capacity * 100);
		if (closedir(dir) == -1)
        {
            perror("Cannot close directory");
        }
	}
	return size;
}


int main(int argc, char* argv[]) {

	argv[1] = ".";
    argc = 2;
    if (argc != 2) 
    {
		fprintf(stderr,"Error! Invalid operands count!\n\t1 - directory name");
		return 0;
	}

	struct statfs buf;
	if (statfs("/", &buf) == -1) 
    {
        perror("Cannot get all dirs stats");
    }
	else 
    {
		long capacity = buf.f_blocks * buf.f_frsize;
		printf("Disk capacity: %ld bytes\n", capacity);
        GetDirSize(argv[1], capacity);
	}	
	return 0;
}