#include "libs.h"

char* GenerateName(const pid_t pid, const int isNotEmpty);

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Open file with text
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    DBG fprintf(stderr, "WRITER: open(file)\n");

    //Check if file is empty
    char firstLetter = 0;
    int isNotEmpty = read(file_id, &firstLetter, 1);
    if (isNotEmpty < 0)
        PRINTERROR("WRITER: Can`t check for first letter\n")
    DBG fprintf(stderr, "WRITER: read(file_id, 1)\n");

    //Generating unique name for fifo
    pid_t pid = getpid();
    char* fifo_name = GenerateName(pid, isNotEmpty);
    DBG fprintf(stderr, " >> #Generated name [%s]\n", fifo_name);

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo <common_fifo>\n")
    DBG fprintf(stderr, "WRITER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_WRONLY);
    if (common_fifo_id < 0)
        PRINTERROR("WRITER: Can`t open <common_fifo>\n")
    DBG fprintf(stderr, "WRITER: open(common_fifo)\n");

    //Writing unique name from common fifo
    int write_common_st = write(common_fifo_id, fifo_name, strlen(fifo_name));
    if (write_common_st != strlen(fifo_name))
        PRINTERROR("WRITER: Error in writing <fifo_name>\n")
    DBG fprintf(stderr, "WRITER: write(common_fifo)\n");

    close(common_fifo_id);


    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("WRITER, Can`t allocate memory for buffer\n")

    //Make fifo with unique name
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo <fifo_name>\n")
    DBG fprintf(stderr, "WRITER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = -1;
    for (int i = 0; i < 1000 && fifo_id <= 0; i++)
        fifo_id = open(fifo_name, O_WRONLY | O_NONBLOCK);//мвм  ч ч цымчрпвчрцавтрцачацачравчрцарцарвуцарварвааваупацаууцрьонмк3лпнчкпчшс2ш3а
    if (fifo_id < 0)
        PRINTERROR("WRITER: Can`t open <fifo_name>\n")
    DBG fprintf(stderr, "WRITER: open(fifo_name)\n");

    //Remove fcntl of unique fifo
    int ret_fcntl = fcntl(fifo_id, F_SETFL, O_WRONLY);
    if (ret_fcntl)
        PRINTERROR("WRITER: Error in fcntl\n")
    DBG fprintf(stderr, "WRITER: fcntl(fifo_name)\n");

    int write_st = write(fifo_id, &firstLetter, 1);
    if (write_st <= 0)
        PRINTERROR("WRITER: Can`t write firstLetter to <fifo_name>\n")

    //Reading text from file and writing to unique fifo
    errno = 0;
    int read_st = -1;
    while ((read_st = read(file_id, buffer, 4096)) != 0){
        write_st = write(fifo_id, buffer, read_st);
        if (write_st <= 0)
            PRINTERROR("WRITER: Can`t write to <fifo_name>\n")
        DBG fprintf(stderr, "WRITER: write(fifo_name)\n");
    }

    if (errno == EPIPE)
        PRINTERROR("WRITER: Fifo died\n")

    close(file_id);
    close(fifo_id);

    free(buffer);
    free(fifo_name);

    DBG fprintf(stderr, "\n\n\nSUCCESS\n");

    return 0;
}

char* GenerateName(const pid_t pid, const int isNotEmpty)
{
    char* fifo_name = (char*) calloc(22, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("WRITER: Can`t allocate memory for <fifo_name>\n")

    if (isNotEmpty)
        strcat(fifo_name, "transfer_fifo");
    else
        strcat(fifo_name, "emptybuf_fifo");

    sprintf(fifo_name + 13, "%d", pid);
    strcat(fifo_name, ".f");

    return fifo_name;
}
