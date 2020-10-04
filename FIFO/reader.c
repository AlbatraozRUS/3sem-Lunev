#include "libs.h"

void Print_Buffer(const char* buffer);

int main(){

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
        PRINTERROR("Can`t mkfifo <common_fifo>\n")
    DBG fprintf(stderr, "READER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("READER: Can`t open <common_fifo>\n")
    DBG fprintf(stderr, "READER: open(common_fifo)\n");

    char* fifo_name = (char*) calloc(20, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("READER: Can`t allocate memory for <fifo_name>\n")

    //Read to common fifo unique name
    int read_st = read(common_fifo_id, fifo_name, 19);
    if (read_st < 0)
        PRINTERROR("READER: Error in writing <fifo_name> to common_fifo\n")
    DBG fprintf(stderr, "READER: read(common_fifo)\n");

    DBG fprintf(stderr, " >> #Scanned name [%s]\n", fifo_name);

    close(common_fifo_id);

    //Make unique fifo
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
        PRINTERROR("READER: Can`t mkfifo <fifo_name>\n")
    DBG fprintf(stderr, "READER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_RDONLY | O_NONBLOCK);
    if (fifo_id < 0)
        PRINTERROR("Can`t open <fifo_name>\n")
    DBG fprintf(stderr, "READER: open(fifo_name)\n");

    //Remove fcntl of unique fifo
    int ret_fcntl = fcntl(fifo_id, F_SETFL, O_RDONLY);
    if (ret_fcntl)
        PRINTERROR("READER: Error in fcntl\n")
    DBG fprintf(stderr, "READER: fcntl(fifo_name)\n");

    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("READER: Can`t allocate memory for buffer\n")

    sleep(3);

    if (read(fifo_id, &buffer[0], 1) == 0){
        if (strncmp(fifo_name, "emptybuf", 8) != 0)
            PRINTERROR("READER: Reader has not pair\n")
    }
    else {
        Print_Buffer(buffer);

        //Reading from unique fifo and printing text
        errno = 0;
        int read_st = -1;
        while ((read_st = read(fifo_id, buffer, 4096)) > 0)
            Print_Buffer(buffer);

        if (errno == EPIPE)
            PRINTERROR("WRITER: Fifo died\n")
    }

    close(fifo_id);

    //Delete unique fifo
    int rm_st = remove(fifo_name);
    if (rm_st)
        PRINTERROR("READER: Can`t remove <fifo_name>\n")
    DBG fprintf(stderr, "READER: remove(fifo_name)\n");

    free(buffer);
    free(fifo_name);

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
