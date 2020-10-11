#include "lib.h"

int main(int argc, char** argv)
{
    size_t numProcess = ScanNum(argc, argv);

    key_t msgkey = msgget(70, IPC_CREAT | 0666);
    if (msgkey < 0)
       PRINTERROR("Can`t create queue\n")

    Send_Message(msgkey, numProcess);

    for (size_t nProcess = 0; nProcess < numProcess; nProcess++){
        pid_t pid = fork();
        switch(pid) {
            case -1: {fprintf(stderr, "Process %zu can`t be created\n", nProcess);
                      abort();}

            case 0:  {fprintf(stderr, "CHILD: My PID - %d   My parent`s PID - %d\n", getpid(), getppid());
                      ReceiveMessage(msgkey, nProcess);
                      //wait();
                      break;}

            default: {fprintf(stderr, "PARENT: PID - %d\n", getpid());break;}
            //          Send_Message(msgkey, numProcess); break;}
        }
    }

    int ret_rm = msgctl(msgkey, IPC_RMID, NULL);
    if (ret_rm < 0)
        PRINTERROR("Can`t remove queue\n")

    return 0;
}

void Send_Message(const key_t msgkey, const size_t numProcess)
{
    struct MsgBuf* msgs_snd = calloc(numProcess, sizeof(struct MsgBuf));
    if (msgs_snd == NULL)
        PRINTERROR("Can`t allocate memory for msgs_snd\n")

    for (size_t nProcess = 0; nProcess < numProcess; nProcess++){
        msgs_snd[nProcess].mtype = nProcess;

        int ret_send = msgsnd(msgkey, &msgs_snd[nProcess], 0, 0/*IPC_NOWAIT*/);
        if (ret_send < 0)
            PRINTERROR("Can`t send message to queue\n")

    }
}

void ReceiveMessage(const key_t msgkey, const size_t id)
{
    struct MsgBuf msg_rcv;

    int ret_rcv = msgrcv(msgkey, &msg_rcv, 0, id, 0);
    if (ret_rcv <= 0)
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
