#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>

#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#include "ft.h"

#define DATA_SIZE 1024

int b[DATA_SIZE];
int main(int argc, char ** argv)
{    
    int a, i, core_id, count;
    int * c = (int *)b;	// for error injection in this program
    uintptr_t pmem;

    if (argc != 1) {
        printf("Usage: client \n");
        exit(0);
    }
    a = 0;

    // core_id is provided for debugging purpose, which may not be supported in the future release.
    core_id = ft_init();	
 
    for (i = 0; i < DATA_SIZE; i++) b[i] = i;		// initialization of b[]

    for (count = 1; count <= 10; count++ ) {
      if (core_id == 0) 
          printf("  --- iteration (%d)\n", count);

      if (count % 3  == 0 && core_id == 1) c[count]++;	// error injection on 'b[3,6,9]' through c for Client_1

      #pragma ft nmr lhs(a) rhs(b)
      {
      // vote will be added after store(a) instruction    
      // vote will be added before load(b[i]) instruction
      // since b[count] is voted and corrected by rhs(b), lhs(a) is not needed
            a = b[count];		
      }

      if (count % 3 == 0 && core_id == 1) a++;	// error injection on a, which will corrupt b[0..1] below for Client_1

      b[0] = a;
      b[1] = a+1;
      // vote will be done for b[0..1]
      // it will report error on the array 'b' if any
      #pragma ft vote(b:sizeof(int)*2) 
    }
    ft_exit();
    if (core_id != 1) 
       printf("Client %d: exits. It must have 0 errors.\n", core_id);
    else
       printf("Client %d: exits. It must have %d occurrences of recoverable errors.\n", core_id, (int)10/3*2);
    return 0;
}
