#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/prctl.h>

#pragma once

enum MODES{
    READ = 0,
    WRITE = 1,
};

const size_t MAXBUFFSIZE = 1000000;

#define DEBUG_MODE

#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured: ");\
         perror("");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}   

struct ChildInfo {
    unsigned id;
    pid_t pid;

    int fifoToPrnt[2];
    int fifoFromPrnt[2];
};

struct Connection {
    int fd_writer;
    int fd_reader;

    size_t buf_size;
    char* buffer;

    size_t iRead;
    size_t iWrite;    

    size_t busy;
    size_t empty;
};

#ifdef DEBUG_MODE
    #define DBG if(1)
#else
    #define DBG if(0)
#endif

size_t ScanNum(char* number_str);    
void ChildFunction(struct ChildInfo* childInfo, char* filePath);
void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds);
size_t CountSize(const unsigned nChild, const unsigned numChilds);
void WriteFromBuffer(struct Connection* connection, const int id);
void ReadToBuffer(struct Connection* connection, const int id);
void TrackPrntDied(pid_t ppid);
void PrepareBuffer(struct Connection* connections, struct ChildInfo* childInfos, 
                               const size_t nChild, const size_t numChilds);
void CloseUnusedPipes(struct ChildInfo* childInfos, const size_t curChild);