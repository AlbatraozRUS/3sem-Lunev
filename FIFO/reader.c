#include "libs.h"

void Print_Buffer(const char* buffer, const int length);
char* GenerateName(const pid_t pid);

int main(int argc, char** argv){

    if (argc != 1)
        fprintf(stderr, "READER: WARNING: No agruments are needed\n");

    //Generating unique name for fifo
    pid_t pid = getpid();
    char* fifo_name = GenerateName(pid);
    DBG fprintf(stderr, " >> #Generated name [%s]\n", fifo_name);

    //Make common fifo for transfering unique name
    errno = 0;
    int ret_com_fifo = mkfifo("common_fifo.f", 0666);
    if (ret_com_fifo && errno != EEXIST)
        PRINTERROR("Can`t mkfifo <common_fifo>\n")
    DBG fprintf(stderr, "READER:[1] mkfifo(common_fifo)\n");

    //Open common fifo
    int common_fifo_id = open("common_fifo.f", O_WRONLY);
    if (common_fifo_id < 0)
        PRINTERROR("READER: Can`t open <common_fifo>\n")
    DBG fprintf(stderr, "READER:[2] open(common_fifo)\n");

    //Make unique fifo
    errno = 0;
    int ret_fifo = mkfifo(fifo_name, 0666);
    if (ret_fifo && errno != EEXIST)
        PRINTERROR("READER: Can`t mkfifo <fifo_name>\n")
    DBG fprintf(stderr, "READER:[3] mkfifo(fifo_name)\n");

    //Open unique fifo
    int fifo_id = open(fifo_name, O_RDONLY | O_NONBLOCK);
    if (fifo_id < 0)
        PRINTERROR("Can`t open <fifo_name>\n")
    DBG fprintf(stderr, "READER:[4] open(fifo_name)\n");    


    //Write to common fifo unique fifo_name
    int read_common_st = write(common_fifo_id, fifo_name, strlen(fifo_name));
    if (read_common_st < 0)
        PRINTERROR("READER: Error in writing <fifo_name> to common_fifo\n")
    DBG fprintf(stderr, "READER:[5] write(common_fifo)\n");    

    close(common_fifo_id);            

    char* buffer = (char*) calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("READER: Can`t allocate memory for buffer\n")

    int read_st = -1;
    int isEmpty = 1;
    errno = 0;
    for (int i = 0; i < 5; i++){          

        read_st = read(fifo_id, buffer, 4096);        

        if (read_st > 0){
            DBG fprintf(stderr, "READER: First read [%d]\n", i);
            Print_Buffer(buffer, read_st);
            isEmpty = 0;
            break;
        }

        usleep(1000);  

        if (errno == EPIPE)
            PRINTERROR("READER: Fifo died\n")                                   
    }   

    if (read_st < 0)
        PRINTERROR("READER: Error in reading from fifo\n")    

    //Remove fcntl of unique fifo
    int ret_fcntl = fcntl(fifo_id, F_SETFL, O_RDONLY);
    if (ret_fcntl)
        PRINTERROR("READER: Error in fcntl\n")
    DBG fprintf(stderr, "READER:[6] fcntl(fifo_name)\n");


    //Reading from unique fifo and printing text
    errno = 0;
    while ((read_st = read(fifo_id, buffer, 4096)) > 0){

        DBG fprintf(stderr, "READER: read(fifo_id)\n");

        if (errno == EPIPE)
            PRINTERROR("READER: Fifo died\n")

        isEmpty = 0;
        Print_Buffer(buffer, read_st);

}
    if (read_st < 0 || errno != 0)
        PRINTERROR("READER: Error in reading from pipe\n")

    if (isEmpty == 1)
        PRINTERROR("READER: Error in writing to stdout\n"
        "Reader has not pair or file is empty\n")

    close(fifo_id);

    //Delete unique fifo
    int rm_st = remove(fifo_name);
    if (rm_st)
        PRINTERROR("READER: Can`t remove <fifo_name>\n")
    DBG fprintf(stderr, "READER:[7] remove(fifo_name)\n");

    free(buffer);
    free(fifo_name);

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}


char* GenerateName(const pid_t pid)
{
    char* fifo_name = (char*) calloc(22, sizeof(char));
    if (fifo_name == NULL)
        PRINTERROR("WRITER: Can`t allocate memory for <fifo_name>\n")

    strcat(fifo_name, "transfer_fifo");
    sprintf(fifo_name + 13, "%d", pid);
    strcat(fifo_name, ".f");

    return fifo_name;
}

void Print_Buffer(const char* buffer, const int length)
{
    if (buffer == NULL)
        PRINTERROR("READER: Pointer to buffer is null in Print_Buffer\n")

    int write_st = write(STDOUT_FILENO, buffer, length);
    if (write_st == -1)
        PRINTERROR("READER: Error while writing to stdout\n")
}
