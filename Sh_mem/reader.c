#include "lib.h"

int main()
{                    
    //Создание семафора
    errno = 0;
    int semid = -1;                    
    if ((semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT)) < 0)
      PRINTERROR("READER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[1] READER: Semaphore was created\n");
    
    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(IPC_PRIVATE, sizeof (char*), 0666 | IPC_CREAT)) < 0)
        PRINTERROR("READER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[2] READER: ShM was created\n");

    //Получение адреса разделяемой памяти
    errno = 0;    
    char* msg = NULL;   
    if ((msg = (char*) shmat(shmid, msg, 0)) == NULL)
        PRINTERROR("READER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[3] READER: Address of ShM was got\n");

    // //Инициализация семафора
    // errno = 0;    
    // if (semctl(semid, 0, SETVAL, 0) < 0)
    //     PRINTERROR("READER: Can`t set semaphore for 0\n")
    //DBG fprintf(stderr, "[4] READER: Semaphore was initialized\n");

    //Цикл работы с получением сообщения
    errno = 0;
    int ret_read = -1;    
    while(1){

        if (semctl(semid, 0, GETVAL, NULL))
            continue;

        if (semctl(semid, 0, SETVAL, 1) < 0)
            PRINTERROR("READER: Can`t set semaphore to 1\n")

        printf("%s", msg);

        if (semctl(semid, 0, SETVAL, NULL) < 0)
            PRINTERROR("READER: Can`t set semaphore to 0\n")

        if (ret_read == 0)
            break;
    }

    //Удаление семафора
    errno = 0;
    if (semctl (semid, 0, IPC_RMID, NULL) < 0)
        PRINTERROR("READER: Can`t remove ShM\n")
    DBG fprintf(stderr, "[5] READER: ShM was removed\n");  

    //Отсоединение сегмента разделяемой памяти
    errno = 0;
    if (shmdt(msg) < 0 )
      PRINTERROR("READER: Error in shmdt\n")
    DBG fprintf(stderr, "[6] READER: ShM was shmdted\n");

    //Удаление разделяемой памяти
    errno = 0;
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        PRINTERROR("READER: Can`t remove ShM\n")
    DBG fprintf(stderr, "[7] READER: ShM was removed\n");      

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}