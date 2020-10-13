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
	for (size_t nProcess = 1; nProcess <= numProcess; nProcess++){
		pid_t pid = fork();
		switch(pid) {

			case -1: {fprintf(stderr, "Process %zu can`t be created\n", nProcess);
					  abort();}

			case 0:  {fprintf(stderr, "CHILD [%zu]: My PID - %d   My parent`s PID - %d\n", nProcess, getpid(), getppid());					  
					  ReceiveMessage(msgkey, nProcess);
					  return 0;}

			default: {fprintf(stderr, "PARENT: PID - %d    My child`s PID - %d\n", getpid(), pid);
					  Send_Message(msgkey, nProcess);
					  break;}
		}
	}
 					//TODO Have to delete queue,
					// but problem is that parent removes queue before child receives message
//	int ret_rm = msgctl(msgkey, IPC_RMID, NULL);
//	if (ret_rm < 0)
//		PRINTERROR("Can`t remove queue\n")

	return 0;
}

void Send_Message(const key_t msgkey, const size_t id)
{
	struct MsgBuf msg_snd = {id};
	
	errno = 0;
	int ret_send = msgsnd(msgkey, &msg_snd, 0, 0);
	if (ret_send < 0)
		PRINTERROR("Can`t send message to queue\n")			
}

void ReceiveMessage(const key_t msgkey, const size_t id)
{
	struct MsgBuf msg_rcv = {0};	

	int ret_rcv = msgrcv(msgkey, &msg_rcv, 0, id, MSG_NOERROR);
	if (ret_rcv < 0)
		PRINTERROR("Error while receiving message from queue\n")

	printf(">>#%zu   Result = %ld\n", id, msg_rcv.mtype);
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