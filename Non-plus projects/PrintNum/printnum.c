#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int ReadN(int argc, char** argv)
{
    //Check if input correctly
    if (argc != 2){
        fprintf(stderr, "Incorrect input\n");
        abort();
    }

    char* endptr = calloc(10,1);

    unsigned long long N = strtoul(argv[1], &endptr, 10);
    if (!(*endptr == '\0') || errno == ERANGE){
        fprintf(stderr, "Something is wrong with your input\n");
        abort();
    }

    return N;
}
