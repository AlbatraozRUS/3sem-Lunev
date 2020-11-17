#include "lib.h"

bool cur_bit;

int main(int argc, char** argv)
{

	if (argc != 2)
        argv[1] = "text.txt";

    pid_t pid = fork();

    cur_bit = 0;    

    switch(pid){

    	case -1: {PRINTERROR("Error in fork()\n")}
 
      	case  0: {ChildFunc(argv); break;}

    	default: {ParentFunc(pid); break;}
    }

	return 0;
}


void ChildFunc(char** argv) 
{    
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("Can`t open file with text\n")    

    pid_t ppid = getppid();    
    int ret_read = -1;

    struct sigaction wait_prnt;
    memset(&wait_prnt, 0, sizeof(wait_prnt));
    wait_prnt.sa_handler = Handler_PrntWait;
    sigemptyset(&wait_prnt.sa_mask);
    sigaction(SIGALRM, &wait_prnt, NULL);

    sigset_t waiting;
    sigfillset(&waiting);
    sigdelset(&waiting, SIGALRM);

    while(true){
    	char cur_letter = 0;
    	ret_read = read(file_id, &cur_letter, 1);

    	if (ret_read == 0)
    		break;

    	if (ret_read == -1)
    		PRINTERROR("Error in read()\n")

    	for (unsigned iBit = 0; iBit < 8; iBit++){
    		char mask = 0x01 << iBit;

    		char bit = mask & cur_letter;    		

            sigsuspend(&waiting);
            DBG fprintf(stderr, "CHILD: Send bit\n");   
    		if (bit == 0){
    			DBG fprintf(stderr, ">>#DEBUG: CHILD: Bit =  0\n");
    			if (kill(ppid, SIGUSR1) == -1)
    				PRINTERROR("Child: Error in kill(SIGUSR1)\n")
    		}
    		else {
    			DBG fprintf(stderr, ">>#DEBUG: CHILD: Bit = 1\n");
    			if (kill(ppid, SIGUSR2) == -1)
    				PRINTERROR("Child: Error in kill(SIGUSR2)\n")
    		}    		                    
    	}    
    }    
    if (kill(ppid, SIGHUP) == -1)
    	PRINTERROR("Child: Error in kill(SIGHUP)\n")

    close(file_id);
    DBG fprintf(stderr, "\n\nChild success!\n");
}


void ParentFunc(const pid_t child_pid)
{
	struct sigaction sig_USR1, sig_USR2, sig_HUP;

	memset(&sig_USR1, 0, sizeof(sig_USR1));
	memset(&sig_USR2, 0, sizeof(sig_USR2));
	memset(&sig_HUP,  0, sizeof(sig_HUP));

	sig_USR1.sa_handler = Handler_USR1;
	sig_USR2.sa_handler = Handler_USR2;
	sig_HUP.sa_handler  = Handler_HUP;

	sigemptyset(&sig_USR1.sa_mask);
	sigemptyset(&sig_USR2.sa_mask);
	sigemptyset(&sig_HUP.sa_mask);

	if (sigaction(SIGUSR1, &sig_USR1, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGUSR1)\n")

	if (sigaction(SIGUSR2, &sig_USR2, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGUSR2)\n")

	if (sigaction(SIGHUP,  &sig_HUP, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGHUP)\n")
					
	sigset_t wait_sig;
	sigfillset(&wait_sig);
	sigdelset(&wait_sig, SIGUSR1);
	sigdelset(&wait_sig, SIGUSR2);		
	sigdelset(&wait_sig, SIGHUP);

	while(true){

		char cur_letter = 0;

		for (int i = 0; i < 8; i++){
			char mask = 0x01 << i;

            kill(child_pid, SIGALRM);
            DBG fprintf(stderr, "PARENT: Send signal to child\n");            
            
			if (sigsuspend(&wait_sig) != -1)
				PRINTERROR("Parent: Error in sigsuspend()")
            DBG fprintf(stderr, "PARENT: Receive bit\n");

			if (cur_bit)
                cur_letter = cur_letter | mask;
            else
                cur_letter = cur_letter & (~mask);
		}

		printf("%c", cur_letter);
        fflush(0);
	}		
}


void Handler_USR1(int sig)
{
	cur_bit = 0;
	DBG fprintf(stderr, ">>#DEBUG: PARENT: SIGNAL - SIGUSR1 = 0\n");	
}


void Handler_USR2(int sig)
{
	cur_bit = 1;
	DBG fprintf(stderr, ">>#DEBUG: PARENT: SIGNAL - SIGUSR2 = 1\n");		
}


void Handler_HUP(int sig)
{
	DBG fprintf(stderr, ">>#DEBUG: PARENT: exit(EXIT_SUCCESS)\n");
	exit(EXIT_SUCCESS);
}

void Handler_PrntWait(int sig)
{
    return;
}