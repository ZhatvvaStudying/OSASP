#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        fprintf(stderr, "Error! Invalid operands count!\n\t1 - input file's name\n\t2 - output group size\n");
        return -1;
    }
    int groupSize = atoi(argv[2]);
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
    int counter;
    while (eof != EOF) 
    {
        counter = 0;
        do 
        {
            eof = PrintStr(file, stdout);
        } 
        while (eof != EOF && ++counter != groupSize);
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