#include "lib.h"

int Print_Buffer(const char* buffer);

int main()
{    
    //Генерация ключа                
    key_t key = ftok("/Users/albatraozrus/Desktop/Proga/3sem-Lunev/Sh_mem/.key", 1);
    if (key < 0)
        PRINTERROR("WRITER: Error in generating key\n")
    DBG fprintf(stderr, "READER: >>#Generated key - %d\n", key);

    //Создание семафора
    errno = 0;
    int semid = -1;                    
    if ((semid = semget(key, 3, 0666 | IPC_CREAT)) < 0)
      PRINTERROR("READER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[1] READER: Semaphore was created\n");
    //DBG fprintf(stderr, "WRITER: >>#Semid - %d\n", semid);
    
    //Блок sembuf для работы с семафорами
    struct sembuf block    = {mutex, 1, 0};
    struct sembuf wait     = {mutex, 0, 0};
    struct sembuf unblock  = {mutex, -1, 0};
    struct sembuf init     = {alive_r, 1, 0};
    struct sembuf start[2] = {        
        {alive_w, -1, 0},
        {alive_r, 1, 0}
    };    
    struct sembuf finish = {alive_w, 0, IPC_NOWAIT};
    

    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(key, 4096, 0666 | IPC_CREAT)) < 0)
        PRINTERROR("READER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[2] READER: ShM was created\n");
    //DBG fprintf(stderr, "WRITER: >>#Shmid - %d\n", shmid);

    //Получение адреса разделяемой памяти
    errno = 0;    
    char* msg = NULL;   
    if ((msg = (char*) shmat(shmid, NULL, 0)) == NULL)
        PRINTERROR("READER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[3] READER: Address of ShM was got\n");    

    //Создание буфера для обмена данными
    char* buffer = calloc(4096, 1);
    if (buffer == NULL)
        PRINTERROR("READER: Can`t allocate memory for buffer\n")

    //Проверка на начало работы    
    if (semop(semid, &init, 1) < 0)
        PRINTERROR("READER: Error in semop(), which inits\n");

    if (semop(semid, start, 2) < 0)
        PRINTERROR("READER: Error in semop(), which starts\n");

    //Цикл работы с получением сообщения  
    int ret_write = -1;  
    while(true) {    
        DBG fprintf(stderr, "[4] READER: Start to read to ShM\n");
        //Проверка на доступ к работе
        if (semop(semid, &wait, 1) < 0)
            PRINTERROR("READER: Error in semop(), which waits\n")
        
        //Блок работы с памятью
        //{
        if (semop(semid, &block, 1) < 0)
            PRINTERROR("READER: Error in semop(), which blocks\n")

        memcpy(buffer, msg, 4096);
        ret_write = Print_Buffer(buffer);    
        memset(msg, '\0', 4096);

        if (semop(semid, &unblock, 1) < 0)
            PRINTERROR("READER: Error in semop(), which unblocks\n")
        //}

        DBG fprintf(stderr, "[5] READER: Finish to read to ShM\n");

        // //Проверка на конец работы
        errno = 0; 
        semop(semid, &finish, 1);
            
        if (errno == EAGAIN)
            continue;                                  
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

int Print_Buffer(const char* buffer)
{
    if (buffer == NULL)
        PRINTERROR("READER: Pointer to buffer is null in Print_Buffer\n")

    int ret_write = printf("%s", buffer);
    fflush(0);
    //Распечатка данных в консоль
    // int ret_write = write(STDOUT_FILENO, buffer, 4096);
    // if (ret_write < 0)
    //     PRINTERROR("READER: Error while writing to stdout\n")
    // fprintf(stderr, "DEBUG ret_write - %d\n", ret_write);

    return ret_write;
}