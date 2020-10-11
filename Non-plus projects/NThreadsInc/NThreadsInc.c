//TODO 1) Порождает n дочерних процессов, каждый процесс должен напечатать свой pid,
//                        pid родителя и порядковый номер, когда он был порожден.
//TODO 2) Написать запускалку, которая запускает произвольную программу с произвольным количеством аргументов
//TODO 3) Породить n потоков, все потоки должны инкрементировать одну и ту же переменную, например, 10 000 раз.


#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

size_t GlobalVar = 0;

size_t ReadN(int argc, char** argv);
void* IncVar (void* numThread);

int main(int argc, char** argv)
{
    size_t numThread = ReadN(argc, argv);

    pthread_t* array_threads = (pthread_t*) calloc(numThread, sizeof(pthread_t));

    for (size_t nThread = 0; nThread < numThread; nThread++)
        pthread_create(&array_threads[nThread], NULL, IncVar, NULL);


    for (size_t nThread = 0; nThread < numThread; nThread++)
        pthread_join(array_threads[nThread], NULL);

    fprintf(stdout, "GLOBAL_VARIABLE - %zu\n", GlobalVar);
    fflush(0);

    free(array_threads);

    return 0;
}

void* IncVar (void* numThread)
{
    if (numThread)
        exit(EXIT_FAILURE);

    GlobalVar++;

    return NULL;
}

size_t ReadN(int argc, char** argv)
{
    //Check if input correctly
    if (argc != 2){
        fprintf(stderr, "Incorrect input\n");
        exit(EXIT_FAILURE);
    }

    char* endptr = calloc(10,1);

    unsigned long long N = strtoul(argv[1], &endptr, 10);
    if ((*endptr != '\0') || errno == ERANGE){
        fprintf(stderr, "Something is wrong with your input\n");
        exit(EXIT_FAILURE);
    }

    return N;
}
