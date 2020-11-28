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

#pragma once

enum MODES{
	READ = 0,
	WRITE = 1,
};

#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured: ");\
         perror("");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}

struct ChildInfo {
	unsigned id;

	int fifoToPrnt[2];
	int fifoFromPrnt[2];
};

#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define DBG if(1)
#else
    #define DBG if(0)
#endif

size_t ScanNum(char** argv);         
void ChildFunction(struct ChildInfo* childInfo, char* filePath);
void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds);

