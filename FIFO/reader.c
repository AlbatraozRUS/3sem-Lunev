#include "libs.h"

int main(){

    errno = 0;
    int create_fifo = mkfifo("transfer.p", 00600);
        if (create_fifo && errno != EEXIST)
            PRINTERROR("Error in creating fifo of writer\n")

    int fifo_id = open("transfer.p", O_RDONLY);
    if (fifo_id < 0)
        PRINTERROR("Could not open fifo\n");

    char* buffer = (char*) calloc(4096, 1);

    read(fifo_id, buffer, 4096);

    printf("%s\n", buffer);
    close(fifo_id);
    unlink("transfer.p");

    return 0;
}
