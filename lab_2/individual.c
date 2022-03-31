#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/vfs.h>

char* GetFullName(char *buffer, char *folder, char *Dir)
{
	strcpy(buffer, folder);
	
	if (buffer[strlen(buffer) - 1] != '/')
    {
        strcat(buffer, "/");
    }
	
	strcat(buffer, Dir);
	return buffer;
}

long long GetDirSize(char* dirName, long capacity)
{
	DIR *dirp;		
	struct dirent *de;
	struct stat buf;
	long long size = 0;
	char fullPath[1024];
		
	if ((dirp = opendir(dirName)) == NULL)
    {
        perror("Cannot open directory");
    }
	else 
    {
		for (de = readdir(dirp); de != NULL; de = readdir(dirp))
        {
			GetFullName(fullPath,dirName,de->d_name);
			
			if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") !=0 && de->d_type == DT_DIR)
            {
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
                        perror("Cannot get stats");
                    }
                }

            }
		}	
		printf("'%s'\t%lld bytes\t Disk usage %f%%\n", dirName, size, (float)size / capacity * 100);
		if (closedir(dirp) == -1)
        {
            perror("Cannot close directory");
        }
	}
	return size;
}


int main(int argc, char* argv[]) {

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