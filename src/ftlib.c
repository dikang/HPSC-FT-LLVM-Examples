
//read.c 
#include<stdio.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "voteshmem.h"

int nmr_id = -1;
int seq_no = 0;

static int vote_init() {
    int i; 
    // check if voter is ready 
    while (cbvptr->num_ranks == 0)
        usleep(100);
    // get its own rank among the participating processes
    nmr_id = -1;
    pid_t pid = getpid();
    while (nmr_id == -1) {
        usleep(100);
        for (i = 0; i < MAX_NMR; i++) {
            if (cbvptr->pids[i] == pid) nmr_id = i;
        }
    }
}

int ft_init() {
    shmem_init();
    vote_init();
    seq_no = cbvptr->vote_seq + 1;
    return nmr_id;
}

void ft_exit() {
//    shmem_exit();
}

static inline void __ft_copy(void * varaddr, int size) {
    char * data = varaddr;
    char * shmloc = (char *)shmdatac[nmr_id];
    printf_debug("%d: ft_copy: varaddr(%p), shmloc(%p)\n", nmr_id, data, shmloc);
    for (int i = 0; i < size; i++)
        shmloc[i] = data[i];
    cbcptr[nmr_id]->data_size = size;
    cbcptr[nmr_id]->seq_no = seq_no;
}

int32_t __ft_copy_and_vote_block(void * varaddr, const int32_t varsize, int * num_errors, int * result_status) {
   
   while (cbvptr->data_seq != seq_no) usleep(50);
   // data write
   __ft_copy(varaddr, varsize);

   usleep(50);

   // vote by voter

   while (cbvptr->vote_seq != seq_no) {
      usleep(50);
   }

   // vote result read
   int32_t result = cbcptr[nmr_id]->vote_result;
   * num_errors += cbcptr[nmr_id]->error_count;
   if (*result_status == VOTE_RESULT_UNRECOVERABLE_ERROR ||
       result == VOTE_RESULT_UNRECOVERABLE_ERROR) {
       *result_status = VOTE_RESULT_UNRECOVERABLE_ERROR;
   } else {
      switch (result) {
         case VOTE_RESULT_RECOVERABLE_ERROR:
              *result_status = result;
              char * orig_data = (char *)varaddr;
              for (int i = 0; i < cbcptr[nmr_id]->data_size; i++) {
                  shmdatac[nmr_id][i] = shmdatav[i];
                  orig_data[i] = shmdatav[i];	// correct user data
              }
              break;
         case VOTE_RESULT_UNRECOVERABLE_ERROR:
         case VOTE_RESULT_UNKNOWN:
         default:
              break;
      }
   } 
   seq_no++;
}

int32_t __ft_copy_and_vote(void * varaddr, const int32_t varsize, int * num_errors, int * result_status) {
   uint32_t num_votes = varsize / SHMEM_SIZE;
   int32_t last_votes = varsize % SHMEM_SIZE;
   int i;
   char * tptr = (char *)varaddr;
   * num_errors = 0;
   * result_status = VOTE_RESULT_NO_ERROR;

   for (i = 0; i < num_votes; i++, tptr += SHMEM_SIZE) 
      __ft_copy_and_vote_block((void*)tptr, (int32_t)SHMEM_SIZE, num_errors, result_status);

   if (last_votes > 0)
     __ft_copy_and_vote_block((void*)tptr, last_votes, num_errors, result_status);

}

int32_t __ft_vote_debug(void * varaddr, const int32_t varsize, void * strptr, const int32_t loc) {
   int i;
   int32_t num_errors;
   int32_t result;

   if (strptr != NULL) {
     printf("rank %d: vote: \"%s\" at line(%d) - addr (%p), size(%d)\n", nmr_id, (char*)strptr, loc, varaddr, varsize);
     usleep(nmr_id * 10);
   }
   __ft_copy_and_vote(varaddr, varsize, &num_errors, &result);
   switch (result) {
     case VOTE_RESULT_RECOVERABLE_ERROR:
       printf("Client %d: Recoverable error with (%d) errors in (%d) bytes\n", nmr_id, num_errors, varsize);
       break;
     case VOTE_RESULT_UNRECOVERABLE_ERROR:
           printf("Client %d: Unrecoverable error\n", nmr_id);
       break;
     case VOTE_RESULT_NO_ERROR:
       break;
     default:
       printf("Client %d: vote result is not ready\n", nmr_id);
       break;
   }
}

int32_t __ft_vote(void * varaddr, const int32_t varsize)
{
  __ft_vote_debug(varaddr, varsize, NULL, 0);
}

int32_t __ft_voter(void *varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_votenow(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_atomic_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_atomic_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_atomic_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_atomic_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_atomic_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t __ft_auto_atomic_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}

int32_t __ft_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_votenow_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_atomic_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_atomic_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_atomic_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_atomic_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_atomic_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t __ft_auto_atomic_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}

int32_t ft_vote(void *varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_voter(void *varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_votenow(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_atomic_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_atomic_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_atomic_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_atomic_vote(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_atomic_voter(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}
int32_t ft_auto_atomic_votel(void * varaddr, int32_t varsize) {
    return __ft_vote(varaddr, varsize);
}

int32_t ft_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_votenow_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_atomic_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_atomic_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_atomic_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_atomic_vote_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_atomic_voter_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
int32_t ft_auto_atomic_votel_debug(void * varaddr, int32_t varsize, void * strptr,  int32_t loc) {
    return __ft_vote_debug(varaddr, varsize, strptr, loc);
}
