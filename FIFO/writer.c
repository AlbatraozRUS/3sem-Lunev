#include "libs.h"

char* GeneraneName(int pid);

int main(int argc, char** argv)
{
    PRINTERROR(argc != 2, "Invalid number of input args\n")

    errno = 0;
    int file_id = open(argv[2], O_RDONLY);
    PRINTERROR(file_id < 0, "Can`t open file with text\n")

    pid_t pid = getpid();


    errno = 0;
    int create_common_fifo = mkfifo("common_fifo.p", 00600);
    PRINTERROR(create_common_fifo && errno != EEXIST,
               "Can`t create common_fifo\n")

    int common_fifo_id = open("common_fifo.p", O_WRONLY);
    PRINTERROR(common_fifo_id < 0, "Can`t open common_fifo\n")

    char* fifo_name = GeneraneName(pid);
    write(common_fifo_id, fifo_name, sizeof(pid));


    errno = 0;
    int create_fifo = mkfifo(fifo_name, 00600);
    PRINTERROR(create_fifo && errno != EEXIST,
               "Error in creating fifo of writer\n")

    errno = 0;
    int fifo_id = open(fifo_name, O_WRONLY);
    PRINTERROR(fifo_id < 0, "Could not open fifo\n")

    char* buffer = (char*) calloc (4096, sizeof(char));
    PRINTERROR(buffer == NULL, "Can`t allocate memory\n")

    for (errno = 0; read(file_id, buffer, 4096);) {
        int write_status = write(fifo_id, buffer, 4096);
        PRINTERROR(write_status < 0 && errno == EPIPE,
                   "Transfer fifo died\n")
        PRINTERROR(write_status < 0 , "Can`t write down text :(\n")
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
    char* fifo_name = (char*) calloc(25, sizeof(char));
    PRINTERROR(fifo_name == NULL, "Can`t allocate memory for fifo_name\n")

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
