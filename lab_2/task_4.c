#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

//ctrl+F to close
const int StopSymb = 6;

int PrintBlock(FILE *file, int linesCount)
{
    int symb = getc(file), line = 0;
    while (symb != EOF && (linesCount == 0 || line < linesCount))
    {
        if (putc(symb, stdout) == EOF)
        {
            perror("Cannot put char into stdout");
            return EOF;
        }
        if (symb == '\n')
        {
            ++line;
        }
        if (linesCount == 0 || line < linesCount)
        {
            symb = getc(file);
        }
    }
    return symb == EOF ? symb : getc(stdin);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Error! Invalid operands count!\n1-file name, 2-strings count");
        return -1;
    }
    int linesCount;
    char **error;
    if ((linesCount = strtol(argv[2], error, 10)) == 0)
    {
        perror("Cannot convert string to integer");
    }
    int descriptor;
    if ((descriptor = open(argv[1], O_RDONLY)) != -1)
    {
        FILE *file;
        if ((file = fdopen(descriptor, "r")) != NULL)
        {
            int symb;
            do
            {
                symb = PrintBlock(file, linesCount);
                if (putc('\n', stdout) == EOF)
                {
                    perror("Cannot put char into stdout");
                    return EOF;
                }
            }
            while (symb != EOF && symb != StopSymb);
        }
        else
        {
            perror("Cannot open file");
        }
    }
    else
    {
        perror("Cannot get file descriptor");
    }
}
