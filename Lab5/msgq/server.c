#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define MAX_SIZE 260

int upqid, downqid;

struct msgbuff
{
    long mtype;
    char mtext[MAX_SIZE];
};

char* convert(char *msg, int sz){
    for(int i=0; i<sz; i++){
        if(islower(msg[i])) msg[i] = toupper(msg[i]);
        else if(isupper(msg[i])) msg[i] = tolower(msg[i]);
    }
    return msg;
}

struct msgbuff recieve(int msgqid){
    int rec_val;
    struct msgbuff msg;

    rec_val = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, !IPC_NOWAIT);

    if(rec_val == -1) perror("\n\nServer: Error in receive\n");
    else printf("\n\nServer: Message received: %s\n", msg.mtext);

    return msg;
}

void send(struct msgbuff msg, int msgqid){
    int send_val;

    send_val = msgsnd(msgqid, &msg, sizeof(msg.mtext), !IPC_NOWAIT);

    if(send_val == -1) perror("\n\nServer: Error in send\n");
    else printf("\n\nServer: Message sent: %s\n", msg.mtext);
}

void serve(int upqid, int downqid){
    struct msgbuff msg = recieve(upqid);  // Recieve the client message
    strcpy(msg.mtext, convert(msg.mtext, strlen(msg.mtext))); // Process it
    send(msg, downqid); // Send the processed message to the client
}

void handler(int signum) {
    printf("\n\nServer is terminating...\n");
    msgctl(upqid, IPC_RMID, (struct msqid_ds *) 0);   // Remove the up queue
    msgctl(downqid, IPC_RMID, (struct msqid_ds *) 0); // Remove the down queue
    printf("\n\nUp and Down queues has been removed, Goodbye!...\n");
    // killpg(getpid(), SIGINT);  // Make the process kill itself
    exit(0);
}

int main(){
    key_t key_id;
    // Get the up queue
    key_id = ftok("up", 65);
    upqid = msgget(key_id, 0666 | IPC_CREAT);
    // Get the down queue
    key_id = ftok("down", 65);
    downqid = msgget(key_id, 0666 | IPC_CREAT);

    if(upqid == -1 || downqid == -1){
        perror("\n\nServer: Error in create\n");
        exit(-1);
    }
    printf("\n\nServer: \nUp Queue ID: %d\nDown Queue ID: %d\n", upqid, downqid);

    signal(SIGINT, handler);  // To catch the ctrl+c signal
    while(1){
        printf("\n\nServer is running...\n");
        serve(upqid, downqid);
    }

    return 0;
}