#include "lib.h"

void Print_Buffer(const char* buffer);

int main()
{    
    //Генерация ключа                
    key_t key = ftok(FTOK_KEY_PATH, FTOK_KEY_NUMBER);
    if (key < 0)
        PRINTERROR("WRITER: Error in generating key\n")
    DBG fprintf(stderr, "READER: >>#Generated key - %d\n", key);

    //Создание семафора
    errno = 0;
    int semid = -1;                    
    if ((semid = semget(key, NUM_SEM, PERMISSION | IPC_CREAT)) < 0)
      PRINTERROR("READER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[1] READER: Semaphore was created\n");        

    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(key, PAGE_SIZE, PERMISSION | IPC_CREAT)) < 0)
        PRINTERROR("READER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[2] READER: ShM was created\n");    

    //Получение адреса разделяемой памяти
    errno = 0;    
    char* msg = NULL;   
    if ((msg = (char*) shmat(shmid, NULL, 0)) == NULL)
        PRINTERROR("READER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[3] READER: Address of ShM was got\n");    

    //Создание буфера для обмена данными
    char* buffer = calloc(PAGE_SIZE, 1);
    if (buffer == NULL)
        PRINTERROR("READER: Can`t allocate memory for buffer\n")

    //Сигнализирование о начале работы
    if (semop(semid, start_check, 2) < 0)
        PRINTERROR("READER: Error in semop, while start()\n")

    DBG fprintf(stderr, "[4] READER: Started to read from ShM\n");       


    //Цикл работы с получением сообщения
    while (true) {    
        if (semop(semid, &full_dw, 1) < 0)
            PRINTERROR("READER: Error in semop, while full_dw()\n")

        if (semop(semid, &mutex_dw, 1) < 0)
            PRINTERROR("READER: Error in semop, while mutex_dw()\n")

        memcpy(buffer, msg, PAGE_SIZE);
        Print_Buffer(buffer);
        memset(msg, '\0', PAGE_SIZE);

        if (semop(semid, &mutex_up, 1) < 0)
            PRINTERROR("READER: Error in semop, while mutex_up()\n")

        if (semop(semid, &empty_up, 1) < 0)
            PRINTERROR("READER: Error in semop, while empty_up()\n")

        errno = 0;
        semop(semid, &finish_check, 1);
        if (errno == EAGAIN)
            continue;
        if (errno == 0)
            break;                    
        if (errno != 0)
            PRINTERROR("READER: Error in semop, while finish_check()\n")
    }    

    DBG fprintf(stderr, "[5] READER: Finally finished reading from ShM\n");

    //Удаление семафора
    errno = 0;
    if (semctl (semid, 0, IPC_RMID, NULL) < 0)
        PRINTERROR("READER: Can`t remove ShM\n")
    DBG fprintf(stderr, "[6] READER: ShM was removed\n");  

    //Отсоединение сегмента разделяемой памяти
    errno = 0;
    if (shmdt(msg) < 0 )
      PRINTERROR("READER: Error in shmdt\n")
    DBG fprintf(stderr, "[7] READER: ShM was shmdted\n");

    //Удаление разделяемой памяти
    errno = 0;
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        PRINTERROR("READER: Can`t remove ShM\n")
    DBG fprintf(stderr, "[7] READER: ShM was removed\n");      

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}

void Print_Buffer(const char* buffer)
{
    if (buffer == NULL)
        PRINTERROR("READER: Pointer to buffer is null in Print_Buffer\n")

    //Распечатка данных в консоль
    printf("%s", buffer);
    fflush(0);      
}