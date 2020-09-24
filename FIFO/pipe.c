#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char* ReadFile(char* pathToFile);

int main(int argc, char** argv)
{
    if (argc != 2){
        fprintf(stderr, "Incorrect number of args\n");
        abort();
    }

    char* buffer = ReadFile(argv[1]);

    pipefd(1);



    return 0;
}

char* ReadFile(char* pathToFile)
{
    FILE* text = fopen(pathToFile, "r");
    if (!text){
        fprintf(stderr, "Could not open file \"%s\"\n", pathToFile);
        abort();
    }

    fseek(text , 0 , SEEK_END);
    size_t nSymbols = ftell(text);
    fseek(text, 0 , SEEK_SET );

    char* buffer = (char*) calloc(nSymbols, 1);

    fread(buffer, 1, nSymbols, text);

    fclose(text);

    return buffer;
}
