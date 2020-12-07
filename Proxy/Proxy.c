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
	size_t numChilds = 3;

	struct ChildInfo* childInfos = calloc(numChilds, sizeof(struct ChildInfo));
	pid_t ppid = getpid();

	for (unsigned nChild = 0; nChild < numChilds; nChild++){

		if (pipe(childInfos[nChild].fifoFromPrnt) == -1)
			PRINTERROR("Parent: Error in pipe from parent\n")

		if (pipe(childInfos[nChild].fifoToPrnt) == -1)
			PRINTERROR("Parent: Error in pipe to parent\n")

		if (fcntl(childInfos[nChild].fifoFromPrnt[WRITE], F_SETFL, O_WRONLY | O_NONBLOCK) == -1)
			PRINTERROR("Parent: Error in fcntl(fd_reader)\n")

		if (fcntl(childInfos[nChild].fifoToPrnt[READ], F_SETFL, O_RDONLY | O_NONBLOCK) == -1)
			PRINTERROR("Parent: Error in fcntl(fd_reader)\n")

		childInfos[nChild].id = nChild;

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
	free(childInfos);

	return 0;
}

void ChildFunction(struct ChildInfo* childInfo, char* filePath)
{
	DBG fprintf(stderr, "CHILD [%d]: Start\n", childInfo->id);

	if (close(childInfo->fifoFromPrnt[WRITE]) == -1)
		PRINTERROR("Child: Can`t close pipe from parent [write]\n")
	childInfo->fifoFromPrnt[WRITE] = -1;

	if (close(childInfo->fifoToPrnt[READ]) == -1)
		PRINTERROR("Child: Can`t close pipe to parent [READ]\n")
	childInfo->fifoToPrnt[READ] 
	= -1;

	int fd_reader = -1;
	int fd_writer = childInfo->fifoToPrnt[WRITE];

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

	if (close(fd_reader) == -1)
		PRINTERROR("Child: Can`t close pipe to fd_reader\n")
	fd_reader = -1;

	if (close(fd_writer)  == -1)
		PRINTERROR("Child: Can`t close pipe to fd_writer\n")
	fd_writer = -1;

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

		PrepareBuffer(connections, childInfos, nChild, numChilds);

		//Closing unused pipes
		//{
		if (close(childInfos[nChild].fifoFromPrnt[READ]) == -1)
			PRINTERROR("Child: Can`t close pipe from parent [read]\n")
		childInfos[nChild].fifoFromPrnt[READ] = -1;

		if (close(childInfos[nChild].fifoToPrnt[WRITE]) == -1)
			PRINTERROR("Child: Can`t close pipe to parent [write]\n")
		childInfos[nChild].fifoToPrnt[WRITE] = -1;
		//}

		//Preparing for select()
		//{
		FD_ZERO(&fd_writers);
		FD_ZERO(&fd_readers);
		for (size_t iChild = nChild; iChild < numChilds; iChild++){

			FD_SET(connections[iChild].fd_reader, &fd_readers);
			FD_SET(connections[iChild].fd_writer, &fd_writers);

			if (connections[iChild].fd_reader > maxFD)
				maxFD = connections[iChild].fd_reader;
			if (connections[iChild].fd_writer > maxFD)
				maxFD = connections[iChild].fd_writer;
		}
		//}

		errno = 0;
		if (select(maxFD + 1, &fd_readers, &fd_writers, NULL, NULL) < 0 && errno != EINTR)
			PRINTERROR("Parent: Error in select\n")
		maxFD = -1;


		DBG fprintf(stderr, "PARENT: [%zu] ParentFunction - begin of cycle\n[\n", nChild);
		for (size_t iChild = nChild; iChild < numChilds; iChild++){

			DBG fprintf(stderr, "Parent: [%zu] read: FDISSET = %d, empty = %zu\n", iChild, FD_ISSET(connections[iChild].fd_reader, &fd_readers), connections[iChild].empty);
			if (FD_ISSET(connections[iChild].fd_reader, &fd_readers) /*&& connections[iChild].empty > 0*/){				
				int ret_read = read(connections[nChild].fd_reader, connections[nChild].buffer, connections[nChild].empty);
				if (ret_read < 0)
					PRINTERROR("Parent: Error in read\n")
				connections[nChild].empty -= ret_read;
				connections[nChild].busy += ret_read;
				//ReadToBuffer(&connections[iChild], iChild);			
			}

			DBG fprintf(stderr, "Parent: [%zu] write: FDISSET = %d, busy = %zu\n", iChild, FD_ISSET(connections[iChild].fd_writer, &fd_writers), connections[iChild].busy);
			if (FD_ISSET(connections[iChild].fd_writer, &fd_writers) /*&& connections[iChild].busy > 0*/){
				int ret_write = write(connections[nChild].fd_writer, connections[nChild].buffer, connections[nChild].busy);
				if (ret_write < 0)
					PRINTERROR("Parent: Error in read\n")

				connections[nChild].busy -= ret_write;
				connections[nChild].empty += ret_write;
				//WriteFromBuffer(&connections[iChild], iChild);		
			}

			if (close(connections[iChild].fd_writer) == -1)
				PRINTERROR("Child: Can`t close pipe to fd_writer\n")
			connections[iChild].fd_writer = -1;

			if (close(connections[iChild].fd_reader) == -1)
				PRINTERROR("Child: Can`t close pipe to fd_reader\n")
			connections[iChild].fd_reader = -1;

		}		
		DBG fprintf(stderr, "]\n");
	}



	// for (size_t nChild = 0; nChild < numChilds; nChild++){
	// 	free(connections[nChild].buffer);
	// 	if (waitpid(childInfos[nChild].pid, NULL, 0) < 0)
	// 		PRINTERROR("Parent: Error in waitpid()\n")	
	// 		fprintf(stderr, "Child [%zu] died\n", nChild);				
	// 	}

	// free(childInfos);
	// free(connections);		
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

	if (connection->iRead >= connection->iWrite)
		connection->busy += ret_read;

	if (connection->iRead + ret_read == connection->buf_size){
		connection->iRead = 0;
		connection->empty = connection->iWrite;
	}
	else {		
		connection->empty += ret_read;
		connection->iRead -= ret_read;	
	}


	DBG fprintf(stderr, "Child:[%d] ReadToBuffer {%s}\n", id, connection->buffer);
	
}

void WriteFromBuffer(struct Connection* connection, int id)
{
	errno = 0;
	int ret_write =  write(connection->fd_writer, &connection->buffer[connection->iWrite], connection->busy);
	if (ret_write < 0 && errno != EAGAIN)
		PRINTERROR("Child: ReadToBuffer: Error in read()\n")

	DBG fprintf(stderr, "Child:[%d] WriteFromBuffer {%s}\n", id, connection->buffer);

	if (connection->iWrite >= connection->iRead)
		connection->empty += ret_write;
	
	if (connection->iWrite + ret_write == connection->buf_size){
		connection->iWrite = 0;
		connection->busy = connection->iRead;
	}
	else {
		connection->iWrite += ret_write;
		connection->busy -= ret_write;
	}
}

void PrepareBuffer(struct Connection* connections, struct ChildInfo* childInfos, 
							   const size_t nChild, const size_t numChilds)
{	
	connections[nChild].buf_size = CountSize(nChild, numChilds);		
	connections[nChild].buffer = calloc(connections[nChild].buf_size, 1);
	if (connections[nChild].buffer == NULL)
		PRINTERROR("Parent: Can`t create buffer\n")

	connections[nChild].iRead = 0;
	connections[nChild].iWrite = 0;

	connections[nChild].busy = 0;
	connections[nChild].empty = connections[nChild].buf_size;

	connections[nChild].fd_reader = childInfos[nChild].fifoToPrnt[READ];
		
	if (nChild == numChilds -1)
		connections[nChild].fd_writer = STDOUT_FILENO;
	else 		
		connections[nChild].fd_writer = childInfos[nChild + 1].fifoFromPrnt[WRITE];	
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