#include "lib.h"

int main(int argc, char** argv)
{

//--------------------------------------------------------------------------------------------------------	
	size_t numProcess = ScanNum(argc, argv);

	key_t msgkey = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
	if (msgkey < 0)
	   PRINTERROR("Can`t create queue\n")
	DBG fprintf(stderr, "Queue was created successfully. Key - %d\n", msgkey);

//--------------------------------------------------------------------------------------------------------
	pid_t pid = -1;
	int id = -1;
	for (size_t nProcess = 1; nProcess <= numProcess; nProcess++){
		pid = fork();

		if (pid == -1){
			fprintf(stderr, "Process %zu can`t be created\n", nProcess);
			abort();
		}

		if (pid == 0){
			id = nProcess;
			break;
		}
	}

	if (pid == 0){

		DBG fprintf(stderr, "CHILD [%d]: My PID - %d   My parent`s PID - %d\n", id, getpid(), getppid());					  
		ChildReceive(msgkey, id + 1);
		return 0;	

	}

	else {
		for (int nProcess = 1; nProcess <= numProcess; nProcess++){
			DBG fprintf(stderr, "PARENT[%d]: PID - %d    My child`s PID - %d\n", nProcess, getpid(), pid);
			ParentSend(msgkey, nProcess + 1);
		}
	}
 					
					
	int ret_rm = msgctl(msgkey, IPC_RMID, NULL);
	if (ret_rm < 0)
		PRINTERROR("Can`t remove queue\n")

	return 0;
}

void ParentSend(const key_t msgkey, const size_t id)
{
	DBG fprintf(stderr, "Parent_Send(): first enter, id - %zu\n", id);
	struct MsgBuf msg = {id};

	errno = 0;
	int ret_send = msgsnd(msgkey, &msg, 0, 0);
	if (ret_send < 0)
		PRINTERROR("Can`t send message to queue\n")	
	DBG fprintf(stderr, "Parent_Send(): message send, id - %zu\n", id);

	errno = 0;
	int ret_rcv = msgrcv(msgkey, &msg, 0, id - 1, MSG_NOERROR);
	if (ret_rcv < 0)
		PRINTERROR("Can`t receive test message in Parent_Send()\n")
	DBG fprintf(stderr, "Parent_Send(): test message received, id - %zu\n", id);	
}

void ChildReceive(const key_t msgkey, const size_t id)
{
	DBG fprintf(stderr, "Child_Receive: first enter, id - %zu\n", id);
	struct MsgBuf msg = {0};	

	int ret_rcv = msgrcv(msgkey, &msg, 0, id, MSG_NOERROR);
	if (ret_rcv < 0)
		PRINTERROR("Error while receiving message from queue\n")
	DBG fprintf(stderr, "Child_Receive(): message received, id - %zu\n", id);

	printf(">>#%zu   Result = %ld\n", id - 1, msg.mtype - 1);
	
	fflush(0);

	msg.mtype--;

	errno = 0;
	int ret_send = msgsnd(msgkey, &msg, 0, 0);
	if (ret_send < 0)
		PRINTERROR("Can`t send test message in Child_Receive()\n")
	DBG fprintf(stderr, "Child_Receive(): test message send, id - %zu\n", id);	

}

size_t ScanNum(const int argc, char** argv)
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