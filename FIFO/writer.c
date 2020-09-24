#include "libs.h"

char* ReadFile(char* pathToFile);

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("Invalid number of input args\n")


    char* buffer = ReadFile(argv[1]);
    size_t length = strlen(buffer);

    if (length > 4095)
        fprintf(stderr, "WARNING:\nFile is bigger than 1 page");

    errno = 0;
    int create_fifo = mkfifo("transfer.p", 00600);
        if (create_fifo && errno != EEXIST)
            PRINTERROR("Error in creating fifo of writer\n")

    errno = 0;
    int fifo_id = open("transfer.p", O_WRONLY);
    if (fifo_id < 0)
        PRINTERROR("Could not open fifo\n");

    errno = 0;
    int writer_pid = 0;
    int ret_read = read(fifo_id, &writer_pid, sizeof(writer_pid));
    if (ret_read <= 0)
        PRINTERROR("Can`t read pid from transfer fifo\n")


    errno = 0;
    int write_status = write(fifo_id, buffer, length);
    if (write_status <= 0 && errno == EPIPE)
            PRINTERROR("Transfer fifo died\n")

    close(fifo_id);
    unlink("transfer.p");

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
