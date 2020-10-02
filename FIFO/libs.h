#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>


#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured: ");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}

//#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define DBG if(1)
#else
    #define DBG if(0)
#endif
