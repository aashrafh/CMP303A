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

#define N 1
#define MAX_SIZE 260
#define MEMSZ 4096

union Semun semun, esemun, fsemun;
int shmid, ssem, csem, empty, full;

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

char* convert(char *msg, int sz){
    for(int i=0; i<sz; i++){
        if(islower(msg[i])) msg[i] = toupper(msg[i]);
        else if(isupper(msg[i])) msg[i] = tolower(msg[i]);
    }
    return msg;
}

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("\n\nServer: Error in down()\n");
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
        perror("\n\nServer: Error in up()\n");
        exit(-1);
    }
}

void writer(char* msg, void *shmaddr){
    printf("\n\nServer: Trying to write...\n");
    down(empty);
    down(ssem);
    printf("\n\nServer: currently write...\n");

    // Write the message
    strcpy((char *)shmaddr, msg);
    printf("\n\nServer: sent message: %s\n", msg);

    up(ssem);
    up(full);
}

char* reader(void *shmaddr){
    printf("\n\nServer: Trying to read...\n");
    down(full);
    down(csem);      
    printf("\n\nServer: currently reading...\n");

    char* msg; // Read the content of the memory
    printf("\n\nI'm HEEEERRE\n");
    strcpy(msg, (char *)shmaddr);
    printf("\n\n55555555555555\n");
    printf("\n\nServer: recieved message: %s\n", msg);

    up(csem);
    up(empty);

    return msg;
}

void serve(int csem, int ssem, void *shmaddr){
    char* msg = reader(shmaddr);    // Get the message from the client
    msg = convert(msg, strlen(msg));    // Process it
    writer(msg, shmaddr);           // Write it again to the memory
}

void handler(int signum) {
    printf("\n\nServer is terminating...\n");
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);          // Remove the shared memory segment
    // Remove Semphores
    semctl(ssem, 0, IPC_RMID, semun);
    semctl(csem, 0, IPC_RMID, semun);
    semctl(full, 0, IPC_RMID, fsemun);
    semctl(empty, 0, IPC_RMID, esemun);
    
    printf("\n\nEverything has been removed, Goodbye!...\n");
    exit(0);
}

int main(){
    key_t key_id = 65;
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

    signal(SIGINT, handler);  // To catch the ctrl+c signal
    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nServer: Error in attach in reader\n");
        exit(-1);
    }
    while(1){
        printf("\n\nServer is running...\n");
        serve(csem, ssem, shmaddr);
    }
    return 0;
}