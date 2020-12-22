#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_SIZE 260
#define MEMSZ 4096

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

int main(){
    key_t key_id = 65;
    int wsem, rsem, shimid;
    union Semun semun;

    shimid = shmget(key_id, MEMSZ, 0666 | IPC_CREAT);
    wsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for writing to the shared memory
    rsem = semget(key_id, 1, 0666 | IPC_CREAT);      // A semaphore for reading from the shared memory

    if(shimid == -1 || wsem == -1 || rsem == -1){
        perror("\n\nClient: Error in create\n");
        exit(-1);
    }
    printf("\n\nClient: Shared Memory ID: %d\n", shimid);

    
    return 0;
}