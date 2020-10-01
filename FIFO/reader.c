#include "libs.h"

char* GenerateName(const int pid);

int main(){

    errno = 0;
    int create_common_fifo = mkfifo("common_fifo.p", 00600);
    if (create_common_fifo && errno != EEXIST)
        PRINTERROR("Can`t create common_fifo\n")

    int common_fifo_id = open("common_fifo.p", O_WRONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open common_fifo\n")

    pid_t pid = getpid();

    char* fifo_name = GenerateName(pid);
    printf("\n# > Generated name [%s]\n", fifo_name);

    printf("I am here #1\n");
    errno = 0;
    int create_fifo = mkfifo(fifo_name, 00600);
    if (create_common_fifo && errno != EEXIST)
        PRINTERROR("Can`t create fifo\n")

    printf("I am here #2\n");

    int fifo_id = open(fifo_name, O_RDONLY);
    if (common_fifo_id < 0)
        PRINTERROR("Can`t open fifo\n")

    printf("I am here #3\n");

    char* buffer = (char*) calloc(4096, sizeof(char));
    if (buffer == NULL)
        PRINTERROR("Can`t allocate memory for buffer in reader\n")


    int wr_common_st = write(common_fifo_id, fifo_name, strlen(fifo_name));
    if (wr_common_st <= 0)
        PRINTERROR("Can`t write unique fifo_name to common fifo\n")

    close(common_fifo_id);

    // int ret_fcntl = fcntl(fifo_id, F_SETFL, O_RDONLY);
    // if (ret_fcntl < 0)
    //     PRINTERROR("Error in fcntl\n");

    errno = 0;
    int read_st = 0;
    while ((read_st = read(fifo_id, buffer, 4096)) > 0){
        printf("\n# > Read from fifo [%d]\n", read_st);
        printf("%s", buffer);
    }

    if (errno == EPIPE)
        PRINTERROR("Fifo died\n")

    int ret_remove = remove(fifo_name);
    if (ret_remove)
        PRINTERROR("Can`t remove file\n");


    unlink(fifo_name);
    close(fifo_id);

    //unlink("common_fifo.p");
    close(common_fifo_id);

    free(fifo_name);
    free(buffer);

    return 0;
}

char* GenerateName(const int pid)
{
    char* fifo_name = (char*) calloc(20, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("Can`t allocate memory for fifo_name\n")

    strcat(fifo_name, "transfer_fifo");
    sprintf(fifo_name + 13, "%d", pid);
    strcat(fifo_name, ".p");


    return fifo_name;
}
