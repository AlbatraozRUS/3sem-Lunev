#include "libs.h"

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Open file with text
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    DBG fprintf(stderr, "WRITER: open(file)\n");

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo common_fifo\n")
    DBG fprintf(stderr, "WRITER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("WRITER: Can`t open common_fifo\n")
    DBG fprintf(stderr, "WRITER: open(common_fifo)\n");

    char* fifo_name = (char*) calloc(22, sizeof(char));

    //Reading unique name from common fifo
    int read_common_st = read(common_fifo_id, fifo_name, 20);
    if (read_common_st < 0)
        PRINTERROR("WRITER: Error in reading fifo_name\n")
    DBG fprintf(stderr, "WRITER: read(common_fifo)\n");

    DBG fprintf(stderr, ">> #Scanned fifo_name [%s]\n", fifo_name);

    close(common_fifo_id);

    //Make fifo with unique name
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo fifo_name\n")
    DBG fprintf(stderr, "WRITER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_WRONLY);
    if (fifo_id < 0)
        PRINTERROR("WRITER: Can`t open fifo_name\n")
    DBG fprintf(stderr, "WRITER: open(fifo_name)\n");

    fd_set rset, wset, eset;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO (&rset);
    FD_ZERO (&wset);
    FD_ZERO (&eset);
    FD_SET(fifo_id, &wset);

    //Wait for respond of reader
    int ret_select = select (fifo_id + 1, &rset, &wset, &eset, &tv);
    if (ret_select == 0)
        PRINTERROR("WRITER: Waited too long for reader\n")
    DBG fprintf(stderr, "WRITER: select(fifo_name)\n");

    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("WRITER: Can`t allocate memory for buffer\n")

    //Reading text from file and writing to unique fifo
    int read_st = -1;
    while ((read_st = read(file_id, buffer, 4096)) != 0){
        int write_st = write(fifo_id, buffer, read_st);
        if (write_st <= 0)
            PRINTERROR("WRITER: Can`t write to fifo_name\n")
        DBG fprintf(stderr, "WRITER: write(fifo_name)\n");
    }

    if (errno == EPIPE)
        PRINTERROR("WRITER: Fifo died\n")

    close(file_id);

    close(fifo_id);

    DBG fprintf(stderr, "\n\n\nSUCCESS\n");

    return 0;
}
