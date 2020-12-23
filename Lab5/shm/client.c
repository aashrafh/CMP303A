#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#define MAX_SIZE 260
#define MEMSZ 4096
#define N 1

int shmid, ssem, csem, empty, full;

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

char* get_input(){
    printf("\n\nWrite your message(maximum 256 character) \nand then press Enter to process it...\n");
    char ch;
    char* text = (char*) malloc(MAX_SIZE);
    int i = 0;
    while(i < 256) {
        ch = getchar();
        if(ch == 0x0A) break;
        text[i++] = ch;
    }
    text[i] = '\0';

    return text;
}

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("\n\nClient: Error in down()\n");
        exit(-1);
    }
}

void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("\n\nClient: Error in up()\n");
        exit(-1);
    }
}

void writer(char* msg, void *shmaddr){
    printf("\n\nClien, t: Trying to write...\n");
    down(empty);
    down(csem);
    printf("\n\nClient: currently writing...\n");

    // Write the message
    msg = strcpy((char *)shmaddr, msg);
    printf("\n\nClient: sent message: %s\n", msg);

    up(csem);
    up(full);
}

void reader(void *shmaddr){
    printf("\n\nClient: Trying to read...\n");
    down(full);
    down(ssem);      
    printf("\n\nClient: currently reading...\n");

    // char* msg; // Read the content of the memory
    // strcpy(msg, (char *)shmaddr);
    printf("\n\nClient: recieved message: %s\n", (char *)shmaddr);

    up(ssem);   
    up(empty);
}

void serve(void *shmaddr){
    char* msg = get_input();    // Get the message from the user
    writer(msg, shmaddr);    // Send to the server
    reader(shmaddr);       // Get the processed message from the server
}

int main(){
    key_t key_id = 65;
    union Semun semun, esemun, fsemun;
    char* text = (char*) malloc(MAX_SIZE);
    shmid = shmget(key_id, MAX_SIZE*sizeof(char), 0666 | IPC_CREAT);
    ssem = semget(65, 1, 0666 | IPC_CREAT);      // A semaphore for writing to the shared memory
    csem = semget(66, 1, 0666 | IPC_CREAT);      // A semaphore for reading from the shared memory
    full = semget(67, 1, 0666 | IPC_CREAT);      
    empty = semget(68, 1, 0666 | IPC_CREAT);      

    if(shmid == -1 || ssem == -1 || csem == -1){
        perror("\n\nServer: Error in create\n");
        exit(-1);
    }
    printf("\n\nServer: Shared Memory ID: %d\n", shmid);

    semun.val = 1; 
    esemun.val = N; 
    fsemun.val = 0; 
    if (semctl(ssem, 0, SETVAL, semun) == -1 || semctl(csem, 0, SETVAL, semun) == -1 || semctl(empty, 0, SETVAL, esemun) == -1 || semctl(full, 0, SETVAL, fsemun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nClient: Error in attach in writer\n");
        exit(-1);
    }

    while(1){
        printf("\n\nClient is running...\n");
        serve(shmaddr);
    }
    return 0;
}