#include "lib.h"

int main(int argc, char** argv)
{
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Oткрытие файла с текстом
    errno = 0;
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    DBG fprintf(stderr, "[1] WRITER: File was opened\n");

    //Создание семафора
    errno = 0;
    int semid = -1;                    
    if ((semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT)) < 0)
      PRINTERROR("WRITER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[2] WRITER: Semaphore was created\n");
    
    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(IPC_PRIVATE, SIZE_SHM, 0666 | IPC_CREAT)) < 0)
        PRINTERROR("WRITER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[3] WRITER: ShM was created\n");

    //Получение адреса разделяемой памяти
    errno = 0;   
    char* msg = NULL;        
    if ((msg = (char*) shmat(shmid, msg, 0)) == NULL)
        PRINTERROR("WRITER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[4] WRITER: Address of ShM was got\n");
                
    //Цикл работы с отправкой сообщения
    errno = 0;
    int ret_read = -1;    
    while(1){

        if (semctl(semid, 0, GETVAL, NULL))
            continue;

        if (semctl(semid, 0, SETVAL, 1) < 0)
            PRINTERROR("WRITER: Can`t set semaphore to 1\n")

        if ((ret_read = read(file_id, msg, 4096)) < 0)
            PRINTERROR("WRITER: Can`t read or write\n")

        if (semctl(semid, 0, SETVAL, NULL) < 0)
            PRINTERROR("WRITER: Can`t set semaphore to 0\n")

        if (ret_read == 0)
            break;
    }

    //Отсоединение сегмента разделяемой памяти
    if (shmdt(msg) < 0)
        PRINTERROR("WRITER: Error in shmdt\n")
    DBG fprintf(stderr, "[5] WRITER: ShM was shmdted\n");
    
    close(file_id);

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}