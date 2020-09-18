//TODO 1) Порождает n дочерних процессов, каждый процесс должен напечатать свой pid,
//                        pid родителя и порядковый номер, когда он был порожден.
//TODO 2) Написать запускалку, которая запускает произвольную программу с произвольным количеством аргументов
//TODO 3) Породить n потоков, все потоки должны инкрементировать одну и ту же переменную, например, 10 000 раз.


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

size_t ReadN(int argc, char** argv);

int main(int argc, char** argv)
{
    size_t numProcess = ReadN(argc, argv);

    for (size_t nProcess = 0; nProcess < numProcess; nProcess++){
        pid_t pid = fork();
        switch(pid) {
            case -1: {fprintf(stderr, "Procces %zu can`t be created\n", nProcess);
                      abort();}

            case 0: {printf("I am child."
                "My number is %zu, pid is \"%d'\" of parent \"%d \"\n",
                                        nProcess,getpid(), getppid()); break;}

            default: {printf("I am parent! My pid is %d\n", pid); break;}
        }
    }

    return 0;
}


size_t ReadN(int argc, char** argv)
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
