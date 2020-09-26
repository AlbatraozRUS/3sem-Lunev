#include "libs.h"

int main(){

    char* fifo_name = (char*) calloc(20, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("Can`t allocate memory for fifo_name\n")


    errno = 0;
    int create_common_fifo = mkfifo("common_fifo.p", 00600);
    if (create_common_fifo && errno != EEXIST)
    PRINTERROR("Can`t create common_fifo\n")

    int common_fifo_id = open("common_fifo.p", O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open common_fifo\n")

    int read_common_st = read(common_fifo_id, fifo_name, 19);
    if (read_common_st <= 0)
        PRINTERROR("Can`t read fifo_name from common fifo\n")


    printf("# > Scanned fifo_name [%s]\n", fifo_name);
    errno = 0;
    int create_fifo = mkfifo(fifo_name, 00600);
    if (create_common_fifo && errno != EEXIST)
        PRINTERROR("Can`t create fifo\n")

    int fifo_id = open(fifo_name, O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open fifo\n")

    char* buffer = (char*) calloc(4096, sizeof(char));
    if (buffer == NULL)
        PRINTERROR("Can`t allocate memory for buffer in reader\n")

    errno = 0;
    int read_st = 0;
    while ((read_st = read(fifo_id, buffer, 4096)) > 0)
        printf("%s", buffer);

    if (errno == EPIPE)
        PRINTERROR("Transfer fifo died\n")


    unlink("common_fifo.p");
    unlink(fifo_name);
    close(fifo_id);
    close(read_common_st);

    free(fifo_name);
    free(buffer);

    return 0;
}
