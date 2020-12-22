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

int shmid;

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
        perror("Error in down()");
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
        perror("Error in up()");
        exit(-1);
    }
}

void writer(char* msg, int sem){
    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nServer: Error in attach in writer\n");
        exit(-1);
    }

    // Write the message
    strcpy((char *)shmaddr, msg);
    printf("\n\nServer: sent message: %s\n", msg);

    up(sem);        // Release the memory
    // shmdt(shmaddr); // Deatching the memory
}

char* reader(int sem){
    down(sem);      // Make sure that the client has written his message

    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nServer: Error in attach in reader\n");
        exit(-1);
    }

    char* msg; // Read the content of the memory
    strcpy(msg, (char *)shmaddr);
    printf("\n\nServer: recieved message: %s\n", msg);
}

void serve(int rsem, int wsem){
    char* msg = reader(rsem);    // Get the message from the client
    msg = convert(msg, strlen(msg));    // Process it
    writer(msg, wsem);           // Write it again to the memory
}

void handler(int signum) {
    printf("\n\nServer is terminating...\n");
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);          // Remove the shared memory segment
    printf("\n\nShared Memory has been removed, Goodbye!...\n");
    exit(0);
}

int main(){
    key_t key_id = 65;
    int wsem, rsem;
    union Semun semun;

    shmid = shmget(key_id, MEMSZ, 0666 | IPC_CREAT);
    wsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for writing to the shared memory
    rsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for reading from the shared memory

    if(shmid == -1 || wsem == -1 || rsem == -1){
        perror("\n\nServer: Error in create\n");
        exit(-1);
    }
    printf("\n\nServer: Shared Memory ID: %d\n", shmid);

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(wsem, 0, SETVAL, semun) == -1 || semctl(rsem, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    signal(SIGINT, handler);  // To catch the ctrl+c signal
    while(1){
        printf("\n\nServer is running...\n");
        serve(rsem, wsem);
    }
    return 0;
}