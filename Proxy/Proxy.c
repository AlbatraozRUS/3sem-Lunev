#include "lib.h"

int main(int argc, char** argv)
{
	if (argc != 3){
		PRINTERROR("Error: Incorrect input\n")
	}

	size_t numChilds = ScanNum(argv[1]);	

	struct ChildInfo* childInfos = calloc(numChilds, sizeof(struct ChildInfo));
	pid_t ppid = getpid();

	for (size_t nChild = 0; nChild < numChilds; nChild++){

		if (pipe(childInfos[nChild].fifoFromPrnt) == -1)
			PRINTERROR("Parent: Error in pipe from parent\n")

		if (pipe(childInfos[nChild].fifoToPrnt) == -1)
			PRINTERROR("Parent: Error in pipe to parent\n")

		childInfos[nChild].id = nChild;

		int ret_fork = fork();
		switch(ret_fork){

			case -1: {PRINTERROR("Error in fork()\n")}

			case 0:  {
					  TrackPrntDied(ppid);
					  CloseUnusedPipes(childInfos, nChild);							  	  
					  ChildFunction(&childInfos[nChild], argv[2]);
					  free(childInfos);
					  exit(EXIT_SUCCESS);
					 }

			default: {
					  if (close(childInfos[nChild].fifoFromPrnt[READ]) == -1)
					  	  PRINTERROR("Parent: Error in close(from_read)")
					  childInfos[nChild].fifoFromPrnt[READ] = -1;
					  if (close(childInfos[nChild].fifoToPrnt[WRITE]) == -1)	
					  	  PRINTERROR("Parent: Error in close(to_write)")
					  childInfos[nChild].fifoToPrnt[WRITE] = -1;
					  continue;
					  }
		}

	}

	ParentFunction(childInfos, numChilds);
	free(childInfos);

	DBG fprintf(stderr, "\n\nSUCCESS\n");

	return 0;
}

void ChildFunction(struct ChildInfo* childInfo, char* filePath)
{
    DBG fprintf(stderr, "CHILD [%d]: Start\n", childInfo->id);    

    if (close(childInfo->fifoToPrnt[READ]) == -1)	
		PRINTERROR("Child: Error in close(to_read)")
	childInfo->fifoToPrnt[READ] = -1;

	if (close(childInfo->fifoFromPrnt[WRITE]) == -1)	
		PRINTERROR("Child: Error in close(from_write)")
	childInfo->fifoFromPrnt[WRITE] = -1;

	int fd_reader = -1;
	int fd_writer = childInfo->fifoToPrnt[WRITE];

	childInfo->pid = getpid();

	if (childInfo->id == 0)
		fd_reader = open(filePath, O_RDONLY);	
	else
		fd_reader = childInfo->fifoFromPrnt[READ];

	if (fd_reader < 0)
		PRINTERROR("Child: Something is wrong with fd_reader\n")

	if (fcntl(fd_writer, F_SETFL, O_WRONLY) == -1)
		PRINTERROR("Child: Error in fcntl(write)\n")

    if (fcntl(fd_reader, F_SETFL, O_RDONLY) == -1)
		PRINTERROR("Child: Error in fcntl(read)\n")

	int ret_read = -1;
	char buffer[4096];
	while (true){		
	    ret_read = read(fd_reader, buffer, 4096);
	    
	    if (ret_read < 0)
	        PRINTERROR("Child: Error in read()\n")
	    
	    if (ret_read == 0)
            break;

		DBG fprintf(stderr, "CHILD [%d]: ChildFunction - read {%s}\n", childInfo->id, buffer);

		if (write(fd_writer, buffer, ret_read) == -1)
			PRINTERROR("Child: Error in write\n")		
	}

	if (close(fd_writer) == -1)	
		PRINTERROR("Child: Error in close(fd_writer)")
	childInfo->fifoToPrnt[WRITE] = -1;

	if (close(fd_reader) == -1)	
		PRINTERROR("Child: Error in close(fd_reader)")
	childInfo->fifoFromPrnt[READ] = -1;

	DBG fprintf(stderr, "CHILD [%d]: End\n", childInfo->id);
}

void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds)
{
	DBG fprintf(stderr, "Parent: Start\n");
	int maxFD = -1;
	fd_set fd_writers, fd_readers;

	struct Connection* connections = calloc(numChilds, sizeof(struct Connection));
	if (connections == NULL)
		PRINTERROR("Parent: Error in calloc(connections)\n")

	for (size_t nChild = 0; nChild < numChilds; nChild++){

		PrepareBuffer(connections, childInfos, nChild, numChilds);

		if (fcntl(connections[nChild].fd_reader, F_SETFL, O_RDONLY | O_NONBLOCK) == -1)
		    PRINTERROR("Parent: Error in fcntl(fd_reader)")

		if (fcntl(connections[nChild].fd_writer, F_SETFL, O_WRONLY | O_NONBLOCK) == -1)
		    PRINTERROR("Parent: Error in fcntl(fd_writer)")
	}	

	size_t cur_alive = 0;
	while (cur_alive < numChilds){

		//Preparing for select()
		//{
		FD_ZERO(&fd_writers);
		FD_ZERO(&fd_readers);

		for (size_t nChild = cur_alive; nChild < numChilds; nChild++){
			if (connections[nChild].fd_reader != -1 && connections[nChild].empty > 0)
				FD_SET(connections[nChild].fd_reader, &fd_readers);
			if (connections[nChild].fd_writer != -1 && connections[nChild].busy > 0)
				FD_SET(connections[nChild].fd_writer, &fd_writers);

			if (connections[nChild].fd_reader > maxFD)
				maxFD = connections[nChild].fd_reader;
			if (connections[nChild].fd_writer > maxFD)
				maxFD = connections[nChild].fd_writer;
		}
		//}		

		errno = 0;
		if (select(maxFD + 1, &fd_readers, &fd_writers, NULL, NULL) < 0)
			PRINTERROR("Parent: Error in select()\n")	    
	    maxFD = -1;			    

	    for (size_t nChild = cur_alive; nChild < numChilds; nChild++){
			DBG fprintf(stderr, "Parent: [%zu] read: FDISSET = %d, empty = %zu\n", nChild, FD_ISSET(connections[nChild].fd_reader, &fd_readers), connections[nChild].empty);
			if (FD_ISSET(connections[nChild].fd_reader, &fd_readers) && connections[nChild].empty > 0)
				ReadToBuffer(&connections[nChild], nChild);
			

			DBG fprintf(stderr, "Parent: [%zu] write: FDISSET = %d, busy = %zu\n", nChild, FD_ISSET(connections[nChild].fd_writer, &fd_writers), connections[nChild].busy);
			if (FD_ISSET(connections[nChild].fd_writer, &fd_writers) && connections[nChild].busy > 0)
				WriteFromBuffer(&connections[nChild], nChild);
			

	        if (connections[nChild].fd_reader == -1 && connections[nChild].fd_writer != -1 && connections[nChild].busy == 0) {
				close(connections[nChild].fd_writer);								
				connections[nChild].fd_writer = -1;

				if (cur_alive != nChild)
	                PRINTERROR("Parent: One of childs is dead\n")

	            cur_alive++;
				free(connections[nChild].buffer);
			}
		}
	}	

	for (size_t nChild = 0; nChild < numChilds; nChild++){
		if (waitpid(childInfos[nChild].pid, NULL, 0) == -1)
			PRINTERROR("Parent: Error in waitpid()\n")
		DBG fprintf(stderr, "Parent: child [%zu] died success\n", nChild);
	}
	
	free(connections);
}

void ReadToBuffer(struct Connection* connection, const int id)
{
    size_t ret_read = read(connection->fd_reader, &connection->buffer[connection->iRead], connection->empty);
    if (ret_read < 0) 
        PRINTERROR("Parent: ReadToBuffer: Error in read\n")
    
    if (ret_read == 0) {
        close(connection->fd_reader);
        connection->fd_reader = -1;
        return;
    }

    if (connection->iRead >= connection->iWrite)
        connection->busy += ret_read;

    if (connection->iRead + ret_read == connection->buf_size) {
        connection->iRead = 0;
        connection->empty = connection->iWrite;
    }
    else {
        connection->iRead += ret_read;
        connection->empty -= ret_read;
    }


	DBG fprintf(stderr, "Parent: [%d] ReadToBuffer {%s}\n", id, connection->buffer);
}

void WriteFromBuffer(struct Connection* connection, const int id)
{	
    size_t ret_write = write(connection->fd_writer, &connection->buffer[connection->iWrite], connection->busy);
    if (ret_write < 0 && errno != EAGAIN) {
        perror("Error write");
        exit(EXIT_FAILURE);
    }

    if (connection->iWrite >= connection->iRead)
        connection->empty += ret_write;

    if (connection->iWrite + ret_write == connection->buf_size) {
        connection->iWrite = 0;
        connection->busy = connection->iRead;
    }
    else {
        connection->iWrite += ret_write;
        connection->busy -= ret_write;
    }

	DBG fprintf(stderr, "Parent: [%d] WriteFromBuffer {%s}\n", id, connection->buffer);
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

void CloseUnusedPipes(struct ChildInfo* childInfos, const size_t curChild)
{
	for (size_t nChild = 0; nChild < curChild; nChild++){
		if (close(childInfos[nChild].fifoFromPrnt[WRITE]) == -1)	
			PRINTERROR("Child: Error in close(from_write)")
		childInfos[nChild].fifoFromPrnt[WRITE] = -1;	

		if (close(childInfos[nChild].fifoToPrnt[READ]) == -1)	
			PRINTERROR("Child: Error in close(to_read)")
		childInfos[nChild].fifoToPrnt[READ] = -1;
	}
}

size_t CountSize(const unsigned nChild, const unsigned numChilds)
{
	size_t buf_size = pow(3, numChilds - nChild) * 1024;

	if (buf_size > MAXBUFFSIZE)
		return MAXBUFFSIZE;
	else 
		return buf_size;
}

void TrackPrntDied(pid_t ppid)
{
    if prctl((PR_SET_PDEATHSIG, SIGTERM) < 0)
    	PRINTERROR("Error in prctl()\n")

    if (ppid != getppid())
    	PRINTERROR("Error: ppid != getppid()\n")
}

size_t ScanNum(char* number_str)
{
	char* endptr = calloc(10,1);

	unsigned long long N = strtoul(number_str, &endptr, 10);
	if (*endptr != '\0' || errno == ERANGE){
		fprintf(stderr, "Something is wrong with your input\n");
		abort();
	}

	return N;
}
