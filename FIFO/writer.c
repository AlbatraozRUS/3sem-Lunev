#include "libs.h"

char* GenerateName(const pid_t pid);

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    pid_t pid = getpid();
    char* fifo_name = GenerateName(pid);
    fprintf(stderr, " >> #Genetated name [%s]\n", fifo_name);

    //Open file with text
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    fprintf(stderr, "WRITER: open(file)\n");

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo common_fifo\n")
    fprintf(stderr, "WRITER: mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_WRONLY);
    if (common_fifo_id < 0)
        PRINTERROR("WRITER: Can`t open common_fifo\n")
    fprintf(stderr, "WRITER: open(common_fifo)\n");

    //Write to common fifo unique name
    int write_st = write(common_fifo_id, fifo_name, strlen(fifo_name));
    if (write_st != strlen(fifo_name))
        PRINTERROR("WRITER: Error in writing fifo_name to common_fifo\n")
    fprintf(stderr, "WRITER: write(common_fifo)\n");

    close(common_fifo_id);

    //Make fifo with unique name
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
            PRINTERROR("WRITER: Can`t mkfifo fifo_name\n")
    fprintf(stderr, "WRITER: mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_WRONLY | O_NONBLOCK);
    if (fifo_id < 0)
        PRINTERROR("WRITER: Can`t open fifo_name\n")
    fprintf(stderr, "WRITER: open(fifo_name)\n");

    //Removing flag O_NONBLOCK
    int ret_fctnl = fcntl(fifo_id, F_SETFL, O_WRONLY);
    if (ret_fctnl == -1)
        PRINTERROR("WRITER: Can`t change in fcntl\n")
    fprintf(stderr, "WRITER: fcntl(common_fifo)\n");

    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("WRITER: Can`t allocate memory for buffer\n")

    //Reading text from file and writing to unique fifo
    int read_st = -1;
    while ((read_st = read(file_id, buffer, 4096)) != 0){
        fprintf(stderr, "WRITER: read(fifo_name)\n");
        int write_st = write(fifo_id, buffer, read_st);
        if (write_st <= 0)
            PRINTERROR("WRITER: Can`t write to fifo_name\n")
        fprintf(stderr, "WRITER: write(fifo_name)\n");
    }

    if (errno == EPIPE)
        PRINTERROR("WRITER: Fifo died\n")

    close(file_id);

    close(fifo_id);

    fprintf(stderr, "\n\n\nSUCCESS\n");

    return 0;
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
