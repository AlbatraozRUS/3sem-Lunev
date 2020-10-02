#include "libs.h"

void Print_Buffer(const char* buffer);
char* GenerateName(const pid_t pid);

int main(){

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
        PRINTERROR("Can`t mkfifo common_fifo\n")
    DBG fprintf(stderr, "READER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_WRONLY /*| O_NONBLOCK*/);
    if (common_fifo_id < 0)
        PRINTERROR("READER: Can`t open common_fifo\n")
    DBG fprintf(stderr, "READER: open(common_fifo)\n");

    pid_t pid = getpid();
    char* fifo_name = GenerateName(pid);
    DBG fprintf(stderr, " >> #Genetated name [%s]\n", fifo_name);

    //Write to common fifo unique name
    int write_st = write(common_fifo_id, fifo_name, strlen(fifo_name));
    if (write_st != strlen(fifo_name))
        PRINTERROR("READER: Error in writing fifo_name to common_fifo\n")
    DBG fprintf(stderr, "READER: write(common_fifo)\n");

    //Make unique fifo
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
        PRINTERROR("READER: Can`t mkfifo fifo_name\n")
    DBG fprintf(stderr, "READER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_RDONLY /*| O_NONBLOCK*/);
    if (fifo_id < 0)
        PRINTERROR("Can`t open fifo_name\n")
    DBG fprintf(stderr, "READER: open(fifo_name)\n");

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
    DBG fprintf(stderr, "READER: select(fifo_name)\n");

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
    DBG fprintf(stderr, "READER: remove(fifo_name)\n");

    DBG fprintf(stderr, "\n\n\nSUCCESS\n");

    return 0;
}

void Print_Buffer(const char* buffer)
{
    if (buffer == NULL)
        PRINTERROR("READER: Pointer to buffer is null in Print_Buffer\n")

    int write_st = write(STDOUT_FILENO, buffer, strlen(buffer));
    if (write_st != strlen(buffer))
        PRINTERROR("READER: Error while writing to stdout\n")
    DBG fprintf(stderr, "READER: write(fifo_id)\n");
}

char* GenerateName(const pid_t pid)
{
    char* fifo_name = (char*) calloc(22, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("WRITER: Can`t allocate memory for fifo_name\n")

    strcat(fifo_name, "transfer_fifo");
    sprintf(fifo_name + 13, "%d", pid);
    strcat(fifo_name, ".f");

    return fifo_name;
}
