#include "lib.h"

//Нихуя не понятно, но очень интересно

//Update: Стало чуть понятнее, но пока все еще интересно

//TODO Не работает select (виснет), проблема с кольцевым буфером
int main(int argc, char** argv)
{
	// if (argc != 3){
	// 	PRINTERROR("Error: Incorrect input\n")
	// }	

	//size_t numChilds = ScanNum(argv[1]);
	size_t numChilds = 8;

	struct ChildInfo* childInfos = calloc(numChilds, sizeof(struct ChildInfo));
	pid_t ppid = getpid();

	for (unsigned nChild = 0; nChild < numChilds; nChild++){

		if (pipe(childInfos[nChild].fifoFromPrnt) == -1)
			PRINTERROR("Parent: Error in pipe from parent\n")

		if (pipe(childInfos[nChild].fifoToPrnt) == -1)
			PRINTERROR("Parent: Error in pipe to parent\n")

		int ret_fork = fork();
		switch(ret_fork){

			case -1: {PRINTERROR("Error in fork()\n")}

			case 0:  {
					  TrackPrntDied(ppid);
					  childInfos[nChild].id = nChild;
					  ChildFunction(&childInfos[nChild], "text.txt"); 
					  exit(EXIT_SUCCESS);
					 }

			default: {continue;}
		}

	}

	ParentFunction(childInfos, numChilds);

	return 0;
}

void ChildFunction(struct ChildInfo* childInfo, char* filePath)
{
	DBG fprintf(stderr, "CHILD [%d]: Start\n", childInfo->id);

	if (close(childInfo->fifoFromPrnt[WRITE]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")

	if (close(childInfo->fifoToPrnt[READ]) == -1)
		PRINTERROR("Child: Can`t close pipe to parent [READ]\n")

	int fd_reader = -1;
	childInfo->pid = getpid();

	if (childInfo->id == 0)
		fd_reader = open(filePath, O_RDONLY);		
	else 
		fd_reader = childInfo->fifoFromPrnt[READ];	

	if (fd_reader < 0)
		PRINTERROR("Child: Something is wrong with fd_reader\n")

	int ret_read = -1;
	char buffer[4096];
	while (ret_read){	
		ret_read = read(fd_reader, buffer, 4096);
		if (ret_read < 0)
			PRINTERROR("Child: Error in read\n")

		DBG fprintf(stderr, "CHILD [%d]: ChildFunction - read {%s}\n", childInfo->id, buffer);

		if (write(childInfo->fifoToPrnt[WRITE], buffer, ret_read) == -1)
			PRINTERROR("Child: Error in write\n")
	}

	if (close(childInfo->fifoFromPrnt[READ]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [read]\n")

	if (close(childInfo->fifoToPrnt[WRITE])  == -1)
		PRINTERROR("Child: Can`t close pipe to parent [write]\n")

	DBG fprintf(stderr, "CHILD [%d]: End\n", childInfo->id);
}

void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds)
{
	int maxFD = -1;
	fd_set fd_writers, fd_readers;	

	struct Connection* connections = calloc(numChilds, sizeof(struct Connection));
	if (connections == NULL)
		PRINTERROR("Parent: Error in calloc(connections)\n")

	for (size_t nChild = 0; nChild < numChilds; nChild++){	

		//Preparing buffer
		//{				
		connections[nChild].buf_size = CountSize(nChild, numChilds);		
		connections[nChild].buffer = calloc(connections[nChild].buf_size, 1);

		connections[nChild].iRead = 0;
		connections[nChild].iWrite = 0;

		connections[nChild].size = 0;
		connections[nChild].empty = connections[nChild].buf_size;

		connections[nChild].fd_reader = childInfos[nChild].fifoToPrnt[READ];
		//}

		if (nChild == numChilds -1)
			connections[nChild].fd_writer = STDOUT_FILENO;
		else
			connections[nChild].fd_writer = childInfos[nChild + 1].fifoFromPrnt[WRITE];

		//Closing unused pipes
		{
		if (close(childInfos[nChild].fifoFromPrnt[READ]) == -1)
			PRINTERROR("Child: Can`t close pipe from parent [read]\n")

		if (close(childInfos[nChild].fifoToPrnt[WRITE]) == -1)
			PRINTERROR("Child: Can`t close pipe to parent [write]\n")
		}

		//Preparing for select()
		//{
		FD_ZERO(&fd_writers);
		FD_ZERO(&fd_readers);
		FD_SET(connections[nChild].fd_reader, &fd_readers);
		FD_SET(connections[nChild].fd_writer, &fd_writers);

		if (connections[nChild].fd_reader > maxFD)
			maxFD = connections[nChild].fd_reader;
		if (connections[nChild].fd_writer > maxFD)
			maxFD = connections[nChild].fd_writer;
		//}

		// if (select(maxFD + 1, &fd_readers, &fd_writers, NULL, NULL) < 1)
		// 	PRINTERROR("Parent: Error in select\n")
	}

		

		DBG fprintf(stderr, "PARENT: ParentFunction - begin of cycle\n{\n");
		for (size_t nChild = 0; nChild < numChilds; nChild++){

			DBG fprintf(stderr, "Parent: [%zu] FDISSET = %d, empty = %d\n", nChild, FD_ISSET(connections[nChild].fd_reader, &fd_readers), connections[nChild].empty > 0);
			 if (FD_ISSET(connections[nChild].fd_reader, &fd_readers) && connections[nChild].empty > 0)				
				ReadToBuffer(&connections[nChild], nChild);			

			DBG fprintf(stderr, "Parent: [%zu] FDISSET = %d, size = %d\n", nChild, FD_ISSET(connections[nChild].fd_writer, &fd_writers), connections[nChild].size > 0);
			if (FD_ISSET(connections[nChild].fd_writer, &fd_writers) && connections[nChild].size > 0)
				WriteFromBuffer(&connections[nChild], nChild);		
		}
		DBG fprintf(stderr, "\n}\n");

		// for (size_t nChild = 0; nChild < numChilds; nChild++){
		// 	if (waitpid(childInfos[nChild].pid, NULL, 0) < 0)
		// 		PRINTERROR("Parent: Error in waitpid()\n")	
		// 	fprintf(stderr, "Child [%zu] died\n", nChild);				
		// }

		free(connections);
		free(childInfos);
}

void ReadToBuffer(struct Connection* connection, int id)
{
	int ret_read =  read(connection->fd_reader, &connection->buffer[connection->iRead], connection->empty);
	if (ret_read < 0)
		PRINTERROR("Child: ReadToBuffer: Error in read()\n")

	if (ret_read == 0){
		close(connection->fd_reader);
		connection->fd_reader = -1;
	}

	if (connection->iRead + ret_read == connection->buf_size){
		connection->iRead = 0;
		connection->size += ret_read;
		connection->empty = connection->iWrite;
	}
	else {

		if (connection->iRead >= connection->iWrite)
			connection->size += ret_read;
		connection->empty -= ret_read;
		connection->iRead += ret_read;	
	}


	DBG fprintf(stderr, "Child:[%d] ReadToBuffer {%s}\n", id, connection->buffer);
	
}

void WriteFromBuffer(struct Connection* connection, int id)
{
	int ret_write =  write(connection->fd_writer, &connection->buffer[connection->iWrite], connection->size);
	if (ret_write < 0)
		PRINTERROR("Child: ReadToBuffer: Error in read()\n")

	DBG fprintf(stderr, "Child:[%d] WriteFromBuffer {%s}\n", id, connection->buffer);

	if (connection->iWrite + ret_write == connection->buf_size){	

		connection->iWrite = 0;
		connection->empty += ret_write;
		connection->size = connection->iRead;
        }
	else {

		if (connection->iWrite >= connection->iRead)
            connection->empty += ret_write;
		connection->size -= ret_write;
        connection->iWrite += ret_write;
	}
	
}

size_t CountSize(const unsigned nChild, const unsigned numChilds)
{	
	return pow(3, numChilds - nChild) * 1024;	
}

void TrackPrntDied(pid_t ppid)
{
    // if prctl((PR_SET_PDEATHSIG, SIGTERM) < 0)
    // 	PRINTERROR("Error in prctl()\n")

    if (ppid != getppid())
    	PRINTERROR("Error: ppid != getppid()\n")           
}

size_t ScanNum(char* number_str)
{
	char* endptr = calloc(10,1);

	unsigned long long N = strtoul(number_str, &endptr, 10);
	if (!(*endptr == '\0') || errno == ERANGE){
		fprintf(stderr, "Something is wrong with your input\n");
		abort();
	}

	return N;
}