#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#pragma once


#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured: ");\
         perror("");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}


#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define DBG if(1)
#else
    #define DBG if(0)
#endif

void Handler_USR1(int sig);
void Handler_USR2(int sig);
void Handler_HUP(int sig);
void Handler_PrntWait(int sig);

void ChildFunc(char** argv);
void ParentFunc(const pid_t child_pid);