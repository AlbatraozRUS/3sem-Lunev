#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


#define PRINTERROR(errorString)\
        {fprintf(stderr, "ATTENTION!\nError occured:");\
         fprintf(stderr, errorString);\
         exit(EXIT_FAILURE);}
