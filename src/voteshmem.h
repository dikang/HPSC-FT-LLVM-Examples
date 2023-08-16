#ifndef __FT_NMR_SHMEM_H__
#define __FT_NMR_SHMEM_H__

#include <stdint.h>

#define MAX_NMR 5
#define SHMEM_SIZE (256 * sizeof(int))
#define DATA_SHMEM_KEY 1234
#define DATA_SHMEM_SIZE ((MAX_NMR + 1) * SHMEM_SIZE)
#define CTRL_SHMEM_KEY 4321
#define CTRL_SHMEM_SIZE (SHMEM_SIZE)

#define VOTE_RESULT_UNKNOWN			0x0
#define VOTE_RESULT_NO_ERROR 			0x1
#define VOTE_RESULT_RECOVERABLE_ERROR		0x4
#define VOTE_RESULT_UNRECOVERABLE_ERROR		0x8

#ifdef DEBUG
#define printf_debug printf
#else
#define printf_debug(...)	{}
#endif

// Control shared memory layout

typedef struct {
  volatile uint32_t data_seq;
  volatile uint32_t vote_seq;
  volatile uint32_t num_ranks;
  volatile pid_t pids[MAX_NMR];
} control_block_voter;

typedef struct {
  volatile uint32_t seq_no;
  volatile uint32_t data_size;
  volatile uint32_t vote_result;
  volatile uint32_t error_count;
} control_block_client;

typedef struct {
  control_block_voter v;
  control_block_client c[MAX_NMR];
} control_mem;


extern char * data_arr;
extern char * ctrl_arr;

extern char * shmdatav;
extern char * shmdatac[MAX_NMR];

extern control_mem * cmptr;
extern control_block_voter * cbvptr;
extern control_block_client * cbcptr[MAX_NMR];

extern void shmem_init();
extern void shmem_exit();

#endif // __FT_NMR_SHMEM_H__
