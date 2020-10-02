#include "libs.h"

void Print_Buffer(const char* buffer);

int main(){

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
        PRINTERROR("Can`t mkfifo common_fifo\n")
    fprintf(stderr, "READER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_RDONLY /*| O_NONBLOCK*/);
    if (common_fifo_id < 0)
        PRINTERROR("READER: Can`t open common_fifo\n")
    fprintf(stderr, "READER: open(common_fifo)\n");

    char* fifo_name = (char*) calloc(22, sizeof(char));

    //Reading unique name from common fifo
    int read_common_st = read(common_fifo_id, fifo_name, 20);
    if (read_common_st != 20)
        PRINTERROR("READER: Name was not scanned fully\n")
    fprintf(stderr, "READER: read(common_fifo)\n");

    fprintf(stderr, ">> #Scanned fifo_name [%s]\n", fifo_name);


    //Make unique fifo
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
        PRINTERROR("READER: Can`t mkfifo fifo_name\n")
    fprintf(stderr, "READER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_RDONLY | O_NONBLOCK);
    if (fifo_id < 0)
        PRINTERROR("Can`t open fifo_name\n")
    fprintf(stderr, "READER: open(fifo_name)\n");


    close(common_fifo_id);

    fd_set rset, wset, eset;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO (&rset);
    FD_ZERO (&wset);
    FD_ZERO (&eset);
    FD_SET(fifo_id, &rset);

    //Wait for respond of reader
    int ret_select = select (fifo_id + 1, &rset, &wset, &eset, &tv);
    if (ret_select == 0)
        PRINTERROR("READER: Waited too long for writer\n")
    fprintf(stderr, "READER: select(fifo_name)\n");

    int ret_fctnl = fcntl(fifo_id, F_SETFL, O_RDONLY);
    if (ret_fctnl == -1)
        PRINTERROR("READER: Can`t change in fcntl\n")
    fprintf(stderr, "READER: fcntl(common_fifo)\n");

    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("READER: Can`t allocate memory for buffer\n")

    //Reading from unique fifo and printing text
    int read_st = -1;
    while ((read_st = read(fifo_id, buffer, 4096)) > 0)
        Print_Buffer(buffer);

    close(fifo_id);

    //Delete unique fifo
    int rm_st = remove(fifo_name);
    if (rm_st)
        PRINTERROR("READER: Can`t remove fifo_name\n")
    fprintf(stderr, "READER: remove(fifo_name)\n");

    fprintf(stderr, "\n\n\nSUCCESS\n");

    return 0;
}

void Print_Buffer(const char* buffer)
{
    if (buffer == NULL)
        PRINTERROR("READER: Pointer to buffer is null in Print_Buffer\n")

    int write_st = write(STDOUT_FILENO, buffer, strlen(buffer));
    if (write_st != strlen(buffer))
        PRINTERROR("READER: Error while writing to stdout\n")
    fprintf(stderr, "READER: write(fifo_id)\n");
}
