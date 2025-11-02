#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <signal.h>

void sig_handler(int signo){
    if (signo == SIGINT){
        printf("\nCaught SIGINT signal. Exiting the program.\n");
    }
}

int main (int argc, char *argv[]){
    
    signal(SIGINT, sig_handler); //Declare our own SIGINT

    //Checks that user enters correct number of arguments
    if (argc != 2){
        printf("Please enter a correct number of arguments!\n");
        return 0;
    }
    
    int pid = fork();

    //Run child process which executes hash program with user arguments
    if (pid == 0){
        printf("Child1: pid %d, ppid %d\n", getpid(), getppid());
        int error_no = execl("./hash", "hash", argv[1], argv[2], NULL);
        printf("ERROR: %i\n", error_no);
    }
    
    wait(NULL); //Wait for child to finish
    printf("Parent: pid %d, ppid %d\n", getpid(), getppid());
    sleep(20);

    return 0;
}