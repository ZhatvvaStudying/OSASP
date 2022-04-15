#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//ctrl+F to close
const int StopSymbol = 6;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Error, invalid operands count!\nOnly file name must be passed\n");
        return 0;
    }
    int descriptor;
    if (descriptor = open(argv[1], O_WRONLY | O_CREAT, 0644))
    {
        FILE* file;
        if (file = fdopen(descriptor, "w"))
        {
            int symb;
            while ((symb = getchar()) != StopSymbol)
            {
                if (putc(symb, file) == EOF)
                {
                    fprintf(stderr, "Error! Cannot write a symbol");
                }
            }
        }
        else
        {
            perror("Error while opening");
        }
        if (pclose(file))
        {
            perror("Error while closing");
        }
    }
    else
    {
        perror("Error while opening");
    }
    return 0;
}