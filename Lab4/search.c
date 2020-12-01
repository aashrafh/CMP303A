#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>


int value;
int termination_code;
pid_t child_a, child_b;
void handler(int signum);
void processChild(int st, int ed, int value, int list[], char child[]);
int main(int argc, char **argv){
    if(argc < 2) perror("Invalid arguments.");
    else {
        value = atoi(argv[1]);
        int n = argc-2;
        int list[n];
        int i = 0;
        while(i < n) {
            list[i] = atoi(argv[i+2]);
            i++;
        }
        termination_code = n+5;

        /*
            Who will be the process executing the code in the handler? The Parent.
            How did you know? The handler should be called whenever SIGUSR1 is called so when the child sends the signal
            the parent has to capture and process it wheneve it was sent. 
        */
        signal(SIGUSR1, handler);

        child_a = fork();
        if(child_a == -1) perror("error in fork");
        else if(child_a == 0){
            //  First Child 
            processChild(0, n/2, value, list, "First");
        }
        else {
            child_b = fork();
            if(child_b == -1) perror("error in fork");
            else if(child_b == 0){
                // Second Child
                processChild(n/2, n, value, list, "Second");
            }
            else {
                // Parent
                printf("\nI am the parent, my pid = %d\n\n", getpid());
                sleep(5);
                int stat_loc_a, stat_loc_b;
                pid_t pid1, pid2;
                pid1 = wait(&stat_loc_a);
                pid2 = wait(&stat_loc_b);
                printf("\nA child with PID=%d terminated\n\n", pid1);
                printf("\nA child with PID=%d terminated\n\n", pid2);
                if(((stat_loc_a >> 8) == termination_code) && ((stat_loc_b >> 8) == termination_code)) printf("\nValue Not Found\n\n");
                exit(0);
            }
        }
    }
    return 0;
}

/*
    Why is it not possible that the signal handler may not receive an exit code?
    When a process terminates using a signal it must send an exit code by default to indicate if
    the process terminated NORMALLY or NOT. You can customize exit codes to indicates special cases or to communicate with parent
*/
void handler(int signum) {
    // printf("\nHandler\n\n");
    int stat_loc;
    pid_t pid = wait(&stat_loc);
    // printf("Child PID that called the handler: %d", pid);
    if(!(stat_loc & 0x00FF)) {
        int idx = stat_loc >> 8;
        int child = 1;
        if(pid == child_b) child = 2;
        if(idx != termination_code){
            printf("\nChild %d: Value %d found at position %d\n\n", child, value, idx);
            for(int i=0; i<3; i++){
                killpg(getpgrp(), SIGKILL);
            }
            printf("\nKilled\n\n");
        }
    }
    signal(SIGUSR1, handler);
}

void processChild(int st, int ed, int value, int list[], char child[]) {
    int ppid = getppid();
    printf("\nI am the %s child, my pid = %d and my parent's pid = %d\n\n", child, getpid(), ppid);
    for(int i=st; i<ed; i++){
        if(list[i] == value){
            kill(ppid, SIGUSR1);
            exit(i);
        }
    }
    sleep(3);
    printf("\n%s Child Terminates\n\n", child);
    exit(termination_code);
}