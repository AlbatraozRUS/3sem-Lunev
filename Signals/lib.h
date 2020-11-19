#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/prctl.h>

#pragma once


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

const int nBits = 8;

void Handler_USR1(int sig);
void Handler_USR2(int sig);
void Handler_CHLD_Died(int sig);
void Handler_Prnt_Wait(int sig);
void Handler_Prnt_Died(int sig);

void ChildFunc(char** argv, const pid_t ppid);
void ParentFunc(const pid_t child_pid);