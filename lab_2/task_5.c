#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int CloseFile(FILE *file)
{
    if (fclose(file) == EOF) 
    {
        perror("Cannot close file. May cause data loss\n");
        return -1;
    }
    return 0;
}

int CloseFiles(FILE *inpFile, FILE *outFile)
{
    return CloseFile(inpFile) | CloseFile(outFile);
}

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        fprintf(stderr, "Error! Invalid operands count!\n\t1 - input file's name\n\t2 - output file's name");
        return -1;
    }

    const char *inputFileName = argv[1];
    const char *outputFileName = argv[2];
    FILE *inputFile, *outputFile;
    
    if ((inputFile = fopen(inputFileName, "r")) == NULL) 
    {
        perror("Cannot open input file\n");
        return -1;
    }
    if ((outputFile = fopen(outputFileName, "w")) == NULL) 
    {
        perror("Cannot open output file\n");
        return -1;
    }

    struct stat buf;
    
    if (stat(inputFileName, &buf)) 
    {
        perror("Cannot access input file information\n");
        CloseFiles(inputFile, outputFile);
        return -1;
    }
    if (chmod(outputFileName, buf.st_mode)) 
    {
        perror("Cannot give access right to input file\n");
        CloseFiles(inputFile, outputFile);
        return -1;
    }

    char ch;
    while ((ch = getc(inputFile)) != EOF) 
    {
        if (putc(ch, outputFile) == EOF) 
        {
            perror("Cannot write to output file\n");   
            break;
        }
    }

    return CloseFiles(inputFile, outputFile);
}