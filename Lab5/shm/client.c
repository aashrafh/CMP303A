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

int shmid, wsem, rsem;;

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
    char text[MAX_SIZE], ch;
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

void writer(char* msg){
    down(wsem);

    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nClient: Error in attach in writer\n");
        exit(-1);
    }

    // Write the message
    strcpy((char *)shmaddr, msg);
    printf("\n\nClient: sent message: %s\n", msg);

    up(rsem);
}

void reader(){
    down(rsem);      

    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (*(int *)shmaddr == -1)
    {
        perror("\n\nClient: Error in attach in reader\n");
        exit(-1);
    }

    char* msg; // Read the content of the memory
    strcpy(msg, (char *)shmaddr);
    printf("\n\nClient: recieved message: %s\n", msg);

    up(wsem);
}

void serve(){
    char* msg = get_input();    // Get the message from the user
    writer(msg);    // Send to the server
    reader();       // Get the processed message from the server
}

int main(){
    key_t key_id = 65;
    union Semun wsemun, rsemun;

    shmid = shmget(key_id, MEMSZ, 0666 | IPC_CREAT);
    wsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for writing to the shared memory
    rsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for reading from the shared memory

    if(shmid == -1 || wsem == -1 || rsem == -1){
        perror("\n\nServer: Error in create\n");
        exit(-1);
    }
    printf("\n\nServer: Shared Memory ID: %d\n", shmid);

    rsemun.val = 0; 
    wsemun.val = 1; 
    if (semctl(wsem, 0, SETVAL, wsemun) == -1 || semctl(rsem, 0, SETVAL, rsemun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    while(1){
        printf("\n\nClient is running...\n");
        serve();
    }
    return 0;
}