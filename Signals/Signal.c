#include "lib.h"

bool cur_bit;

int main(int argc, char** argv)
{

	if (argc != 2)
        argv[1] = "text.txt";

    sigset_t init_block = {0};    
    sigfillset(&init_block);

    sigprocmask(SIG_BLOCK, &init_block, NULL);

    cur_bit = 0;    

    pid_t parent_pid = getpid();
    pid_t child_pid = fork();
    switch(child_pid){

    	case -1: {PRINTERROR("Error in fork()\n")}
 
      	case  0: {ChildFunc(argv, parent_pid); break;}

    	default: {ParentFunc(child_pid); break;}
    }

	return 0;
}


void ChildFunc(char** argv, const pid_t ppid) 
{    
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
       PRINTERROR("Child: Error in prctl\n")

    if (ppid != getppid())
        PRINTERROR("Child: No parent\n")

    DBG fprintf(stderr, ">>#DEBUG: CHILD: My pid - %d, my ppid - %d\n", getpid(), getppid());
    int file_id = open(argv[1], O_RDONLY);
    if (file_id < 0)
        PRINTERROR("Can`t open file with text\n")            

    struct sigaction wait_prnt, parent_Died;

    memset(&wait_prnt, 0, sizeof(wait_prnt));    
    memset(&parent_Died, 0, sizeof(parent_Died));

    wait_prnt.sa_handler = Handler_Prnt_Wait;
    sigfillset(&wait_prnt.sa_mask);
    wait_prnt.sa_flags = SA_NODEFER;

    if (sigaction(SIGUSR1, &wait_prnt, NULL) == -1)
        PRINTERROR("Child: Error in sigaction(SIGUSR1)\n")

    parent_Died.sa_handler = Handler_Prnt_Died;
    sigfillset(&wait_prnt.sa_mask);
    parent_Died.sa_flags = SA_NODEFER;

    if (sigaction(SIGTERM, &parent_Died, NULL) == -1)
        PRINTERROR("Child: Error in sigaction(SIGTERM)\n")

    sigset_t waiting;
    sigfillset(&waiting);
    sigdelset(&waiting, SIGUSR1);
    sigdelset(&waiting, SIGTERM);

    DBG fprintf(stderr, ">>#DEBUG: CHILD: Start\n");
    int ret_read = -1;
    while(true){
    	char cur_letter = 0;
    	ret_read = read(file_id, &cur_letter, 1);

    	if (ret_read == 0)
    		break;

    	if (ret_read == -1)
    		PRINTERROR("Error in read()\n")

    	for (unsigned iBit = 0; iBit < nBits; iBit++){
    		char mask = 0x01 << iBit;

    		char bit = mask & cur_letter;    		
            
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
            sigsuspend(&waiting);
            DBG fprintf(stderr, ">>#DEBUG: CHILD: Next itteration\n");
    	}    
    }    

    close(file_id);
    DBG fprintf(stderr, "\n\nChild success!\n");
}


void ParentFunc(const pid_t child_pid)
{
    DBG fprintf(stderr, ">>#DEBUG: PARENT: My pid - %d, my child_pid - %d\n", getpid(), child_pid);    
	struct sigaction sig_USR1, sig_USR2, sig_CHLD_Died;

	memset(&sig_USR1, 0, sizeof(sig_USR1));
	memset(&sig_USR2, 0, sizeof(sig_USR2));
	memset(&sig_CHLD_Died,  0, sizeof(sig_CHLD_Died));

	sig_USR1.sa_handler       = Handler_USR1;
	sig_USR2.sa_handler       = Handler_USR2;
	sig_CHLD_Died.sa_handler  = Handler_CHLD_Died;

    sig_USR1.sa_flags = 0;
    sig_USR2.sa_flags = 0;
    sig_CHLD_Died.sa_flags = SA_NOCLDSTOP;

	sigfillset(&sig_USR1.sa_mask);    
	sigfillset(&sig_USR2.sa_mask);        
	sigfillset(&sig_CHLD_Died.sa_mask);

	if (sigaction(SIGUSR1, &sig_USR1, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGUSR1)\n")

	if (sigaction(SIGUSR2, &sig_USR2, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGUSR2)\n")

	if (sigaction(SIGCHLD,  &sig_CHLD_Died, NULL) == -1)
		PRINTERROR("Parent: Error in sigaction(SIGHUP)\n")
					
	sigset_t wait_sig;
	sigfillset(&wait_sig);
	sigdelset(&wait_sig, SIGUSR1);
	sigdelset(&wait_sig, SIGUSR2);		
	sigdelset(&wait_sig, SIGCHLD);

    DBG fprintf(stderr, ">>#DEBUG: PARENT: Start\n");
	while(true){

		char cur_letter = 0;

		for (unsigned iBit = 0; iBit < nBits; iBit++){
			char mask = 0x01 << iBit;           
            
			if (sigsuspend(&wait_sig) != -1)
				PRINTERROR("Parent: Error in sigsuspend()")
            DBG fprintf(stderr, ">>#DEBUG: PARENT: Receive bit\n");

			if (cur_bit)
                cur_letter = cur_letter | mask;
            else
                cur_letter = cur_letter & (~mask);

            kill(child_pid, SIGUSR1);
            DBG fprintf(stderr, ">>#DEBUG: PARENT: Send signal to child\n"); 
		}

		printf("%c", cur_letter); 
        fflush(stdout);       
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

void Handler_Prnt_Died(int sig)
{
    fprintf(stderr, "Child: Error, parent died...\n");
    exit(EXIT_FAILURE);
}

void Handler_CHLD_Died(int sig)
{
	DBG fprintf(stderr, ">>#DEBUG: PARENT: exit(EXIT_SUCCESS)\n");
	exit(EXIT_FAILURE);
}

void Handler_Prnt_Wait(int sig)
{
    return;
}
