#include "libs.h"

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("Invalid number of input args\n")

    errno = 0;
    int create_common_fifo = mkfifo("common_fifo.p", 00600);
    if (create_common_fifo && errno != EEXIST)
        PRINTERROR("Can`t create common_fifo\n")

    int common_fifo_id = open("common_fifo.p", O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open common_fifo\n")

    char* fifo_name = (char*) calloc(20, sizeof(char));
    int rd_common_st = read(common_fifo_id, fifo_name, 20);
    printf("# > Scanned fifo_name [%s]\n", fifo_name);
    if (rd_common_st <= 0)
        PRINTERROR("Can`t read unique fifo_name to common fifo\n")

    close(common_fifo_id);

    printf("I am here #1\n");

    errno = 0;
    int create_fifo = mkfifo(fifo_name, 00600);
    if (create_fifo && errno != EEXIST)
        PRINTERROR("Error in creating fifo of writer\n")

    printf("I am here #2\n");

    errno = 0;
    int fifo_id = open(fifo_name, O_WRONLY);
    if (fifo_id < 0)
        PRINTERROR("Could not open fifo\n")

    printf("I am here #3\n");

    char* buffer = (char*) calloc (4096, sizeof(char));
    if (buffer == NULL)
        PRINTERROR("Can`t allocate memory\n")

    errno = 0;
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("Can`t open file with text\n")

    // int ret_fcntl = fcntl(fifo_id, F_SETFL, O_WRONLY);
    // if (ret_fcntl < 0)
    //     PRINTERROR("Error in fcntl\n");

    int read_st = 0;
    errno = 0;
    while ((read_st = read(file_id, buffer, 4096)) > 0){
        int write_status = write(fifo_id, buffer, read_st);
        printf("\n# > Written to fifo [%d]\n", write_status);

        if (write_status < 0 && errno == EPIPE)
            PRINTERROR("Transfer fifo died\n")
        if (write_status < 0)
            PRINTERROR("Can`t write down text :(\n")

        if (write_status < 4096)
                write(fifo_id, "\0", 1);
    }

    close(file_id);

    close(common_fifo_id);
    //unlink("common_fifo.p");

    close(fifo_id);
    unlink(fifo_name);

    free(buffer);
    free(fifo_name);

    return 0;
}
