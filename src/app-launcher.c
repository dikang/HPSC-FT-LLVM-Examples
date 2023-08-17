#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "voteshmem.h"

void spawn_voter(int nmr, pid_t *pids, char * voter_binary) {
    char * voter_name = voter_binary;
    pid_t pid = fork(); // Create a new process
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        char nmr_string[100];
        sprintf(nmr_string, "%d  ", nmr);
        char *args[] = {voter_name, nmr_string, NULL};
        execvp(voter_name, args);
    } else {
        pids[nmr] = pid;
    }
}

void spawn_app(int nmr, pid_t * pids, char * binary_name) {
    int i;
    for (i = 0; i < nmr; i++) {
        pid_t pid = fork(); // Create a new process
        
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            // Execute the "a.out" binary
            char *args[] = {binary_name, NULL};
            execvp(binary_name, args);
            // execvp only returns if an error occurs
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            pids[i] = pid;
            cbvptr->pids[i] = pid;	// initialize control_block_voter's pids table
        }
    }
    usleep(50);
    cbvptr->num_ranks = nmr;
}

int test_file(char * fname) {
    FILE * file = fopen(fname, "r");
    if (!file) {
        printf("Error: %s file doesn't exist\n", fname);
        exit(0);
    }
    fclose(file);
}

int main(int argc, char ** argv) {
    int i;
    pid_t pids[MAX_NMR+1];

    if (argc != 4) {
        printf("uage: app-launcher <number of processes> <path of the executable> <path of the voter binary>\n");
        return -1;
    }

    int  N = atoi(argv[1]);
    char *binary_name = argv[2];

    for (i = 2; i <= 3; i++)
        test_file(argv[i]);

    shmem_init();
    cbvptr->num_ranks = 0;	// not ready for voting. voter needs to set it up.
    usleep(50);
    spawn_voter(N, pids, argv[3]);    
    usleep(50);
    spawn_app(N, pids, binary_name);
    
    int status;
    for (i = 0; i < N; i++) {
        waitpid(pids[i], &status, 0); // Wait for the child process to finish
    }
    cbvptr->num_ranks = 0; 
    waitpid(pids[N], &status, 0);
    shmem_exit();
    return 0;
}

