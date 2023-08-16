#include<stdio.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include "voteshmem.h"

char * data_arr;
char * ctrl_arr;
int data_shmid;
int ctrl_shmid;
char * shmdatav;
char * shmdatac[MAX_NMR];

control_mem * cmptr;
control_block_voter * cbvptr;
control_block_client * cbcptr[MAX_NMR];

void shmem_init() {
    int i;
    // shared memory initialization
    data_shmid = shmget(DATA_SHMEM_KEY, DATA_SHMEM_SIZE, 0666|IPC_CREAT);
    ctrl_shmid = shmget(CTRL_SHMEM_KEY, CTRL_SHMEM_SIZE, 0666|IPC_CREAT);

    data_arr = shmat(data_shmid, NULL, 0);
    ctrl_arr = shmat(ctrl_shmid, NULL, 0);

    cmptr = (control_mem *)ctrl_arr;    
    cbvptr = & cmptr->v;

    shmdatav = data_arr;
    for (i = 0; i < MAX_NMR; i++) {
        shmdatac[i] = &data_arr[(i+1)*SHMEM_SIZE];
        cbcptr[i] = & cmptr->c[i];
    }
}

void shmem_exit()
{
    shmdt((void *) ctrl_arr);
    shmdt((void *) data_arr);
    shmctl(data_shmid, IPC_RMID, NULL);
    shmctl(ctrl_shmid, IPC_RMID, NULL);
}
