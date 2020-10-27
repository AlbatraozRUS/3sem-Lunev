#include "lib.h"

int main(int argc, char** argv)
{
    //Проверка на корректность ввода аргумента
    if (argc != 2)
        PRINTERROR("WRITER: Invalid number of arguments\n")

    //Генерация ключа                
    key_t key = ftok("/Users/albatraozrus/Desktop/Proga/3sem-Lunev/Sh_mem/.key", 1);
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
    if ((semid = semget(key, 3, 0666 | IPC_CREAT)) < 0)
      PRINTERROR("WRITER: Can`t create semaphore\n")
    DBG fprintf(stderr, "[2] WRITER: Semaphore was created\n");
    //DBG fprintf(stderr, "WRITER: >>#Semid - %d\n", semid);
    
    //Блок sembuf для работы с семафорами
    struct sembuf block    = {mutex, 1, 0};
    struct sembuf wait     = {mutex, 0, 0};
    struct sembuf unblock  = {mutex, -1, 0};
    struct sembuf init     = {alive_w, 1 ,0};
    struct sembuf start[2] = {        
        {alive_r, -1, 0},
        {alive_w, 1, 0}
    };
    struct sembuf finish = {alive_w, -1, 0};

    //Cоздание сегмента разделяемой памяти
    errno = 0;
    int shmid = -1;           
    if ((shmid = shmget(key, 4096, 0666 | IPC_CREAT)) < 0)
        PRINTERROR("WRITER: Can`t create shared memory\n")
    DBG fprintf(stderr, "[3] WRITER: ShM was created\n");
    //DBG fprintf(stderr, "WRITER: >>#Shmid - %d\n", shmid);

    //Получение адреса разделяемой памяти
    errno = 0;   
    char* msg = NULL;        
    if ((msg = (char*) shmat(shmid, NULL, 0)) == NULL)
        PRINTERROR("WRITER: Can`t get address of SgM\n")  
    DBG fprintf(stderr, "[4] WRITER: Address of ShM was got\n");    

    //Выставление семафора сигнализирующего о начале работы    
    if (semop(semid, &init, 1) < 0)
        PRINTERROR("READER: Error in semop(), which inits\n");

    if (semop(semid, start, 2) < 0)
        PRINTERROR("READER: Error in semop(), which starts\n");

    //Цикл работы с отправкой сообщения
    errno = 0;      
    int ret_read = 1;
    while (ret_read > 0){
        DBG fprintf(stderr, "[5] WRITER: Start to write to ShM\n");
        //Проверка на доступ к работе
        if (semop(semid, &wait, 1) < 0)
            PRINTERROR("WRITER: Error in semop(), which waits\n")        

        //Блок работы с памятью
        //{
        if (semop(semid, &block, 1) < 0)
            PRINTERROR("WRITER: Error in semop(), which blocks\n")

        if ((ret_read = read(file_id, msg, 4096)) < 0)
            PRINTERROR("WRITER: Error in reading from file and writting to ShMmem\n")
        memset(msg + ret_read, '\0', 4096 - ret_read);

        if (semop(semid, &unblock, 1) < 0)
            PRINTERROR("WRITER: Error in semop(), which unblocks\n")
        //}

        DBG fprintf(stderr, "[6] WRITER: Finish to write to ShM\n");

    }

    DBG fprintf(stderr, "[7] WRITER: Finally finished writting to ShM\n");

    //Выставление семафора, сигнализирующего о конце работы
    if (semop(semid, &finish, 1) < 0)
        PRINTERROR("READER: Error in semop(), which finishes\n");

    //Отсоединение сегмента разделяемой памяти
    if (shmdt(msg) < 0)
        PRINTERROR("WRITER: Error in shmdt\n")
    DBG fprintf(stderr, "[8] WRITER: ShM was shmdted\n");
    
    close(file_id);

    DBG fprintf(stderr, "SUCCESS\n\n\n");

    return 0;
}