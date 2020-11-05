#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>


#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured: ");\
         perror("");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}

//#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define DBG if(1)
#else
    #define DBG if(0)
#endif


enum SEMS {
	MUTEX = 0,
	EMPTY = 1,
	FULL = 2, 
	ALIVE_W = 3,
	ALIVE_R = 4
};


const char FTOK_KEY_PATH[] = "/Users/albatraozrus/Desktop/Proga/3sem-Lunev/Sh_mem/.key";
const int  FTOK_KEY_NUMBER = 1;
const int  PAGE_SIZE 	   = 4096;
const int  NUM_SEM = 5;

#define PERMISSION 0666


//Блок sembuf для работы с семафорами
struct sembuf mutex_up = {MUTEX,    1, SEM_UNDO};
struct sembuf mutex_dw = {MUTEX,   -1, SEM_UNDO};
struct sembuf empty_up = {EMPTY,    1, 0};
struct sembuf empty_dw = {EMPTY,   -1, 0};
struct sembuf full_up  = {FULL,     1, 0};
struct sembuf full_dw  = {FULL,    -1, 0};
struct sembuf finish   = {ALIVE_W,  1, 0};        
struct sembuf check_R  = {ALIVE_R, -5, SEM_UNDO};
struct sembuf finish_check = {ALIVE_W, -1, IPC_NOWAIT};
struct sembuf start[3] = {
    {MUTEX,   1, 0},
    {EMPTY,   1, 0},
    {ALIVE_W, 5, SEM_UNDO}
}; 
struct sembuf start_check[2] = {
        {ALIVE_W, -5, SEM_UNDO},
        {ALIVE_R,  5, SEM_UNDO}
    };  
              
