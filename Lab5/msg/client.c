#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_SIZE 260

struct msgbuff
{
    long mtype;
    char mtext[MAX_SIZE];
};

struct msgbuff get_input(){
    printf("\n\nWrite your message(maximum 256 character) \nand then press Enter to process it...\n");
    char text[MAX_SIZE], ch;
    int i = 0;
    while(i < 256) {
        ch = getchar();
        if(ch == 0x0A) break;
        text[i++] = ch;
    }
    text[i] = '\0';
    // Build the mesage buffer
    struct msgbuff msg;
    msg.mtype = getpid()%10000;
    strcpy(msg.mtext, text);

    return msg;
}

void recieve(long mtype,int msgqid){
    int rec_val;
    struct msgbuff msg;

    rec_val = msgrcv(msgqid, &msg, sizeof(msg.mtext), mtype, !IPC_NOWAIT);

    if(rec_val == -1) perror("\n\nClient: Error in receive\n");
    else printf("\n\nClient: Message received: %s\n", msg.mtext);
}

void send(struct msgbuff msg, int msgqid){
    int send_val;

    send_val = msgsnd(msgqid, &msg, sizeof(msg.mtext), !IPC_NOWAIT);

    if(send_val == -1) perror("\n\nClient: Error in send\n");
    else printf("\n\nClient: Message sent: %s\n", msg.mtext);
}

void serve(int upqid, int downqid){
    struct msgbuff msg = get_input();
    send(msg, upqid);               // Send the message to the server
    recieve(msg.mtype, downqid);    // Recieve the server messag
}

int main(){
    key_t key_id;
    int upqid, downqid;
    // Get the up queue
    key_id = ftok("up", 65);
    upqid = msgget(key_id, 0666 | IPC_CREAT);
    // Get the down queue
    key_id = ftok("down", 65);
    downqid = msgget(key_id, 0666 | IPC_CREAT);

    if(upqid == -1 || downqid == -1){
        perror("\n\nClient: Error in create\n");
        exit(-1);
    }
    printf("\n\nClient: \nUp Queue ID: %d\nDown Queue ID: %d\n", upqid, downqid);

    while(1){
        printf("\n\nClient is running...\n");
        serve(upqid, downqid);
    }

    return 0;
}