#include "lib.h"

//Нихуя не понятно, но очень интересно

int main(int argc, char** argv)
{
	
	if (argc != 3)
		PRINTERROR("Error: Incorrect input\n")			

	size_t numChilds = ScanNum(argv);		

	struct ChildInfo* childInfos = calloc(numChilds, sizeof(struct ChildInfo));
	
	for (unsigned nChild = 0; nChild < numChilds; nChild++){

		if (pipe(childInfos[nChild].fifoFromPrnt) == -1)
			PRINTERROR("Parent: Error in pipe from parent\n")

		if (pipe(childInfos[nChild].fifoToPrnt) == -1)
			PRINTERROR("Parent: Error in pipe to parent\n")

		int ret_fork = fork();
		switch(ret_fork){

			case -1: {PRINTERROR("Error in fork()\n")}

			case 0:  {childInfos[nChild].id = nChild;
					  ChildFunction(&childInfos[nChild], argv[2]); 
					  exit(EXIT_SUCCESS);}

			default: {continue;}
		}

	}

	ParentFunction(childInfos, numChilds);

	return 0;
}

void ChildFunction(struct ChildInfo* childInfo, char* filePath)
{
	if (close(childInfo->fifoFromPrnt[WRITE]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")

	if (close(childInfo->fifoToPrnt[READ]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")

	int fd_reader = -1;

	if (childInfo->id == 0)
		fd_reader = open(filePath, O_RDONLY);		
	else 
		fd_reader = childInfo->fifoFromPrnt[READ];	

	int ret_read = -1;
	char buffer[4096];
	while (ret_read){
		ret_read = read(fd_reader, buffer, 4096);
		if (ret_read < 0)
			PRINTERROR("Child: Error in read\n")
		if (write(childInfo->fifoToPrnt[WRITE], buffer, ret_read) == -1)
			PRINTERROR("Child: Error in write\n")
	}

	if (close(childInfo->fifoFromPrnt[READ]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")

	if (close(childInfo->fifoToPrnt[WRITE])  == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")
}

void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds)
{
	for (unsigned  nChild = 0; nChild < numChilds; nChild++){

		if (close(childInfos[nChild].fifoFromPrnt[READ]) == -1)
			PRINTERROR("Child: Can`t close pipe from parent [write]\n")

		if (close(childInfos[nChild].fifoToPrnt[WRITE]) == -1)
			PRINTERROR("Child: Can`t close pipe from parent [write]\n")
	}

	int maxFD = -1;

	for (unsigned nChild = 0; nChild < numChilds; nChild++){
		fd_set fd_writers, fd_readers;
		FD_ZERO(&fd_readers);
		FD_ZERO(&fd_writers);

		FD_SET(childInfos[nChild].fifoFromPrnt[WRITE], &fd_writers);
		FD_SET(childInfos[nChild].fifoToPrnt[READ], &fd_readers);

		if (childInfos[nChild].fifoFromPrnt[WRITE] > maxFD)
			maxFD = childInfos[nChild].fifoFromPrnt[WRITE];
		if (childInfos[nChild].fifoToPrnt[READ] > maxFD)
			maxFD = childInfos[nCh			ьфчАВ = сршдвШтащыхтСршдвъюашащЕщЗктехКУФВъж
ild].fifoToPrnt[READ];
	}
}

size_t CountSize(const unsigned nChild, const unsigned numChilds)
{
	return pow(3, numChilds - nChild) * 1024;
}

size_t ScanNum(char** argv)
{
	char* endptr = calloc(10,1);

	unsigned long long N = strtoul(argv[1], &endptr, 10);
	if (!(*endptr == '\0') || errno == ERANGE){
		fprintf(stderr, "Something is wrong with your input\n");
		abort();
	}

	return N;
}