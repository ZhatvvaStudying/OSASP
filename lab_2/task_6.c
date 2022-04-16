#include <stdio.h>
#include <dirent.h>

int PrintDirectory(const char *directoryName)
{
    DIR *directory;
    printf("Content of %s:\n", directoryName);
    
    if ((directory = opendir(directoryName)) == NULL) 
    {
        perror("Cannot open directory\n");
        return -1;
    }
    
    struct dirent *dirent;
    while ((dirent = readdir(directory)) != NULL) 
    {
        if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
        {
            printf("\t%s\n", dirent->d_name);
        }
    }
    
    if (closedir(directory)) 
    {
        perror("Cannot close directory\n");
        return -1;
    }
    return 0;
}

int main()
{
    return PrintDirectory("./") | PrintDirectory("/");
}