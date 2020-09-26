#include "libs.h"

char* GeneraneName(int pid);

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("Invalid number of input args\n")

    errno = 0;
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("Can`t open file with text\n")

    pid_t pid = getpid();


    errno = 0;
    int create_common_fifo = mkfifo("common_fifo.p", 00600);
    if (create_common_fifo && errno != EEXIST)
        PRINTERROR("Can`t create common_fifo\n")

    int common_fifo_id = open("common_fifo.p", O_WRONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open common_fifo\n")

    char* fifo_name = GeneraneName(pid);
    int wr_common_st = write(common_fifo_id, fifo_name, 20);
    if (wr_common_st <= 0)
        PRINTERROR("Can`t write unique fifo_name to common fifo\n")


    errno = 0;
    int create_fifo = mkfifo(fifo_name, 00600);
    if (create_fifo && errno != EEXIST)
        PRINTERROR("Error in creating fifo of writer\n")

    errno = 0;
    int fifo_id = open(fifo_name, O_WRONLY);
    if (fifo_id < 0)
        PRINTERROR("Could not open fifo\n")

    char* buffer = (char*) calloc (4096, sizeof(char));
    if (buffer == NULL)
        PRINTERROR("Can`t allocate memory\n")

    for (errno = 0; read(file_id, buffer, 4096);) {
        int write_status = write(fifo_id, buffer, 4096);
        if (write_status < 0 && errno == EPIPE)
            PRINTERROR("Transfer fifo died\n")
        if (write_status < 0)
            PRINTERROR("Can`t write down text :(\n")
    }

    unlink("common_fifo.p");
    unlink(fifo_name);
    close(fifo_id);
    close(common_fifo_id);

    free(fifo_name);

    return 0;
}

char* GeneraneName(int pid)
{
    char* fifo_name = (char*) calloc(20, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("Can`t allocate memory for fifo_name\n")

    strcat(fifo_name, "transfer_fifo");
    sprintf(fifo_name + 13, "%d", pid);
    strcat(fifo_name, ".p");

    printf("# > Genetated name [%s]\n", fifo_name);

    return fifo_name;
}

// errno = 0;
// int writer_pid = 0;
// int ret_read = read(fifo_id, &writer_pid, sizeof(writer_pid));
// if (ret_read <= 0)
//     PRINTERROR("Can`t read pid from transfer fifo\n")
