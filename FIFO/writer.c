#include "libs.h"

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Open file with text
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    DBG fprintf(stderr, "WRITER:[1] open(file)\n");

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo <common_fifo>\n")
    DBG fprintf(stderr, "WRITER:[2] mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_RDONLY);    
    if (common_fifo_id < 0)
        PRINTERROR("WRITER: Can`t open <common_fifo>\n")
    DBG fprintf(stderr, "WRITER:[3] open(common_fifo)\n");

    char* fifo_name = (char*) calloc(20, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("READER: Can`t allocate memory for <fifo_name>\n")

    //Reading unique name from common fifo
    int read_common_st = read(common_fifo_id, fifo_name, 20);
    if (read_common_st <= 0)
        PRINTERROR("WRITER: Error in read <fifo_name>\n")
    DBG fprintf(stderr, "WRITER:[4] read(common_fifo)\n");

    DBG fprintf(stderr, " >> #Scanned name [%s]\n", fifo_name);

    close(common_fifo_id);


    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("WRITER, Can`t allocate memory for buffer\n")

    //Open unique fifo
    int fifo_id = open(fifo_name, O_WRONLY | O_NONBLOCK);
    if (fifo_id < 0)
        PRINTERROR("WRITER: Can`t open <fifo_name>\n")
    DBG fprintf(stderr, "WRITER:[6] open(fifo_name)\n");

    //Remove fcntl of unique fifo
    int ret_fcntl = fcntl(fifo_id, F_SETFL, O_WRONLY);
    if (ret_fcntl)
        PRINTERROR("WRITER: Error in fcntl\n")
    DBG fprintf(stderr, "WRITER:[7] fcntl(fifo_name)\n");    

    //Reading text from file and writing to unique fifo
    errno = 0;
    int read_st = -1;
    while ((read_st = read(file_id, buffer, 4096)) != 0){        

        int write_st = write(fifo_id, buffer, read_st);
        if (write_st <= 0)
            PRINTERROR("WRITER: Can`t write to <fifo_name>\n")
        DBG fprintf(stderr, "WRITER:[8] write(fifo_name)\n");
    }
    if (read_st < 0)
            PRINTERROR("READER: Error in reading file\n")

    if (errno == EPIPE)
        PRINTERROR("WRITER: Fifo died\n")

    close(file_id);
    close(fifo_id);

    free(buffer);
    free(fifo_name);

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}
