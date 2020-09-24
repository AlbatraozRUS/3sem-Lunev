#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define PRINTERROR(errorString) {fprintf(stderr, errorString); exit(EXIT_FAILURE);}

//TODO Нужно написать две программы, один читает, второй пишет. файл не должен быть испорчен???,
// можно пользоваться только файловым ip, нельзя lockами для синхронизации и прочем,один раз можно использовать на крайняк sleep.

char* ReadFile(char* pathToFile);

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("Invalid number of input args\n")


    char* buffer = ReadFile(argv[1]);
    size_t length = strlen(buffer);

    int file_op = open(argv[1], O_RDONLY);
    if (file_op < 0)
        PRINTERROR("Can`t open file\n")

    errno = 0;
    int fifo_id = mkfifo("transferPipe_From", O_WRONLY);
        if (fifo_id && errno != EEXIST) {
            printf("ERRNO - %d\n", errno);
            PRINTERROR("Error in creating fifo of writer\n")
        }

    errno = 0;
    if (write(fifo_id, buffer, length) != length){
        printf("ERRNO - %d\n", errno);
        PRINTERROR("Could not write down text\n")
    }

    close(fifo_id);
    unlink("transferPipe_From");

    return 0;
}

char* ReadFile(char* pathToFile)
{
    FILE* text = fopen(pathToFile, "r");
    if (!text){
        fprintf(stderr, "Could not open file \"%s\"\n", pathToFile);
        abort();
    }

    fseek(text , 0 , SEEK_END);
    size_t nSymbols = ftell(text);
    fseek(text, 0 , SEEK_SET );

    char* buffer = (char*) calloc(nSymbols, 1);

    fread(buffer, 1, nSymbols, text);

    fclose(text);

    return buffer;
}
