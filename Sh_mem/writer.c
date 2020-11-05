#include "lib.h"

int main(int argc, char** argv)
{
    //Проверка на корректность ввода аргумента
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Генерация ключа                
    key_t key = ftok(FTOK_KEY_PATH, FTOK_KEY_NUMBER);
    if (key < 0)
        PRINTERROR("WRITER: Error in generating key\n")
    DBG fprintf(stderr, "WRITER: >>#Generated key - %d\n", key);

    //Oткрытие файла с текстом
    errno = 0;
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("WRITER: Can`t open file with text\n")
    DBG fprintf(stderr, "[1] WRITER: File was opened\n");

    //Создание семафора
    errno = 0;
    int semid = -1;                    
    if ((semid = semget(key, NUM_SEM, PERMISSION | IPC_CREAT)) < 0)
      PRINTERROR("WRITER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[2] WRITER: Semaphore was created\n");
    
    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(key, PAGE_SIZE, PERMISSION | IPC_CREAT)) < 0)
        PRINTERROR("WRITER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[3] WRITER: ShM was created\n");   

    //Получение адреса разделяемой памяти
    errno = 0;   
    char* msg = NULL;        
    if ((msg = (char*) shmat(shmid, NULL, 0)) == NULL)
        PRINTERROR("WRITER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[4] WRITER: Address of ShM was got\n");    

    //Сигнализирование о начале работы
    if (semop(semid, start, 3) < 0)
        PRINTERROR("WRITER: Error in semop, while start()\n")  
    if (semop(semid, &check_R, 1) < 0)
        PRINTERROR("WRITER: Error in semop, while check_R()\n")

    DBG fprintf(stderr, "[5] WRITER: Started to write down to ShM\n");  


    //Цикл работы с отправкой сообщения
    errno = 0;      
    int ret_read = 1;
    while (ret_read > 0){

        if (semop(semid, &empty_dw, 1) < 0)
            PRINTERROR("READER: Error in semop, while empty_dw()\n")

        if (semop(semid, &mutex_dw, 1) < 0)
            PRINTERROR("READER: Error in semop, while mutex_dw()\n")

        if ((ret_read = read(file_id, msg, PAGE_SIZE)) < 0)
            PRINTERROR("READER: Can`t read from file\n")
        memset(msg + ret_read, '\0', PAGE_SIZE - ret_read);

        if (semop(semid, &mutex_up, 1) < 0)
          PRINTERROR("READER: Error in semop, while mutex_up()\n")

        if (semop(semid, &full_up, 1) < 0)
            PRINTERROR("READER: Error in semop, while full_up()") 
    }

    //Сигнализирование о конце работы
    if  (semop(semid, &finish, 1) < 0)
        PRINTERROR("READER: Error in semop, while start()\n") 

    DBG fprintf(stderr, "[6] WRITER: Finally finished writting to ShM\n");


    //Отсоединение сегмента разделяемой памяти
    if (shmdt(msg) < 0)
        PRINTERROR("WRITER: Error in shmdt\n")
    DBG fprintf(stderr, "[7] WRITER: ShM was shmdted\n");
    
    close(file_id);

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}