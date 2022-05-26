#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

int PrintStr(FILE *inputFile, FILE *outputFile)
{
    int ch = getc(inputFile);
    while (ch != EOF && ch != '\n') 
    {
        if (putc(ch, outputFile) == EOF)
        {
            return EOF;
        }
        ch = getc(inputFile);
    }
    if (ch == '\n' && putc(ch, outputFile) == EOF)
    {
        return EOF;
    }
    return ch;
}

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

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        fprintf(stderr, "Error! Invalid operands count!\n\t1 - input file's name\n\t2 - output group size\n");
        return -1;
    }
    
    bool isError;
    long groupSize = ParseLong(argv[2], &isError);
    if (!isError)
    {
        if (groupSize < 0) 
        {
            fprintf(stderr, "Group size cannot be negative\n");
            return -1;
        }
        FILE *file;
        if ((file = fopen(argv[1], "r")) == NULL) 
        {
            perror("Cannot open file\n");
            return -1;
        }

        int eof = 1;
        long counter;
        while (eof != EOF) 
        {
            counter = 0;
            do 
            {
                eof = PrintStr(file, stdout);
            } 
            while (eof != EOF && ++counter != groupSize);
            if (putc('\n', stdout) == EOF)
            {
                perror("Cannot put char\n");
                return -1;
            }
            if (eof != EOF) 
            {
                while (getchar() != '\n');
            }
        }
        
        if (fclose(file) == EOF) 
        {
            perror("Cannot close file\n");
            return -1;
        }

        return 0;
    }
    return -1;
}