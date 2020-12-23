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

int main(){
    void *shmaddr = shmat(18, (void *)0, 0);
    shmctl(18, IPC_RMID, (struct shmid_ds *)0);          // Remove the shared memory segment
    return 0;
}