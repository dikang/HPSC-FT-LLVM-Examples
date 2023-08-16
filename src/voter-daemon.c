// write.c
#include<stdio.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "voteshmem.h"

int main(int argc, char const *argv[]) {
    printf_debug("Voter: is alive with argument %s\n", argv[1]);
    if (argc != 2) {
        printf("Usage: test <id>\n");
        exit(0);
    }

    int no_cores = atoi(argv[1]);
    int seq_no = 11;
    int no_majority = (no_cores / 2) + (no_cores % 2 == 0 ? 2 : 1);

    shmem_init();

    // initialize global vote sequence numbers
    cbvptr->vote_seq = seq_no-1;
    cbvptr->data_seq = seq_no;

    for (int i = 0; i < no_cores; i++) {
        cbcptr[i]->seq_no = seq_no-1;
    }
   
    printf_debug("Voter: vote_seq(%d), data_seq(%d)\n", cbvptr->vote_seq, cbvptr->data_seq);
    cbvptr->num_ranks = no_cores;	// Show that voter is ready.

    while(1) {
        for (int i = 0; i < no_cores; i++) {
            while(cbcptr[i]->seq_no != seq_no) {
                usleep(5);
                if (cbvptr->num_ranks == 0) {
                    return 0;
                }
        }
            cbcptr[i]->vote_result = VOTE_RESULT_UNKNOWN;
        }
        printf_debug("Voter: %d got all from (%d) clients\n", no_cores);
        // vote
        int data_size = cbcptr[0]->data_size;
        for (int i = 0; i < no_cores; i++) {
            for (int j = 0; j < data_size; j++) {
                printf_debug("(%d,%d) ", i, shmdatac[i][j]);
            }
            printf_debug("\n");
        }
	// vote and make a reference data
        bool unrecoverable_error = false;
        for (int i = 0; i < data_size; i++) {
            int vote_count;
            for (int j = 0; j < no_cores; j++) {
                vote_count = 0;
                for (int k = 0; k < no_cores; k++) {	// actual vote vote_matrix
                    if (shmdatac[j][i] == shmdatac[k][i]) vote_count++;
                }
                if (vote_count >= no_majority) {
                    shmdatav[i] = shmdatac[j][i];
                    break;
                } 
            }
            if (vote_count < no_majority) {
                unrecoverable_error = true;
                break;
            }
        }
        printf_debug("Ref data: ");
        for (int i = 0; i < data_size; i++) 
            printf_debug("%d ", shmdatav[i]);
        printf_debug("\n");
	// compare and get result
        if (unrecoverable_error) {
            printf("Voter: Unrecoverable error \n");
            for (int j = 0; j < no_cores; j++) {
               cbcptr[j]->vote_result = VOTE_RESULT_UNRECOVERABLE_ERROR;	
            } 
        } else { // copy shmdatav[] back
            printf_debug("Copy voted data back\n");
            for (int j = 0; j < no_cores; j++) {
                cbcptr[j]->error_count = 0;
                for (int i = 0; i < data_size; i++)
                    if (shmdatac[j][i] != shmdatav[i]) cbcptr[j]->error_count++;
                cbcptr[j]->vote_result = VOTE_RESULT_NO_ERROR;
                if (cbcptr[j]->error_count > 0) {
                    cbcptr[j]->vote_result = VOTE_RESULT_RECOVERABLE_ERROR;
                    printf_debug("Voter: Core %d: # of errors = %d\n", j, cbcptr[j]->error_count);
                }
            }
        }
                
        cbvptr->vote_seq = seq_no;
        seq_no++;
        cbvptr->data_seq = seq_no;
        usleep(5);
    }
//    shmem_exit();
}
