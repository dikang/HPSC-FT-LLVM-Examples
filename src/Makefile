ARMCC=/home/dkang/nvme/INCEPTION/Tools/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
RISCV=/nvme/dkang/HPSC/Microchip/tools/riscv64-unknown-linux-gnu-toolsuite-14.9.0-2022.12.0/
RISCVGCC=$(RISCV)/bin/riscv64-unknown-linux-gnu-gcc
RISCVGCCHOME=$(RISCV)/lib/gcc/riscv64-unknown-linux-gnu/11.3.1/
RISCVSYSROOT=$(RISCV)/sysroot/
CLANG=/tmp/bin/clang

BINARIES=voter-daemon vote-client app-launcher 
BINARIES_RISCV=voter-daemon-riscv vote-client-riscv app-launcher-riscv

debug = 0
ifeq ($(debug),1)
	CFLAGS=-DDEBUG=1 -g -fPIC
else
	CFLAGS=-fPIC
endif

all: voter-daemon vote-client app-launcher voter-daemon-riscv vote-client-riscv app-launcher-riscv
RISCVINCLUDE=-I$(RISCVGCCHOME)/include -I$(RISCVGCCHOME)/include-fixed -I$(RISCVGCCHOME)/../../../../riscv64-unknown-linux-gnu/include -I$(RISCVSYSROOT)/usr/include 

RISCVCFLAGS=-mabi=lp64d -fno-addrsig 

app-launcher.o: app-launcher.c voteshmem.h
	gcc $(CFLAGS)  app-launcher.c -c 

ftlib.o: ftlib.c voteshmem.h
	gcc $(CFLAGS)  -c ftlib.c 

shmem.o: shmem.c voteshmem.h
	gcc $(CFLAGS)  shmem.c -c 

vote-client.o: vote-client.c voteshmem.h
	$(CLANG) $(CFLAGS) -fft vote-client.c -c 
	$(CLANG) $(CFLAGS) -fft vote-client.c -S -emit-llvm -o vote-client.ll

voter-daemon.o: voter-daemon.c voteshmem.h
	gcc $(CFLAGS)  voter-daemon.c -c 

app-launcher: app-launcher.o shmem.o 
	gcc $(CFLAGS)  app-launcher.o shmem.o -o app-launcher 

vote-client: vote-client.o shmem.o ftlib.o
	gcc $(CFLAGS)   shmem.o vote-client.o ftlib.o -o vote-client

voter-daemon: voter-daemon.o shmem.o
	gcc $(CFLAGS)  voter-daemon.o shmem.o -o voter-daemon

app-launcher-riscv.o: app-launcher.c voteshmem.h
	$(RISCVGCC) app-launcher.c -c -o app-launcher-riscv.o

ftlib-riscv.o: ftlib.c voteshmem.h
	$(RISCVGCC) $(CFLAGS) -c ftlib.c -o ftlib-riscv.o 

shmem-riscv.o: shmem.c voteshmem.h
	$(RISCVGCC) shmem.c -c -o shmem-riscv.o

vote-client-riscv.s: vote-client.c voteshmem.h
	$(CLANG) $(CFLAGS) $(RISCVCFLAGS) $(RISCVINCLUDE) --target=riscv64 -fft vote-client.c -S -o vote-client-riscv.s
	$(CLANG) $(CFLAGS) $(RISCVCFLAGS) $(RISCVINCLUDE) --target=riscv64 -fft vote-client.c -S -emit-llvm -o vote-client-riscv.ll

app-launcher-riscv: app-launcher-riscv.o shmem-riscv.o 
	$(RISCVGCC) $(CFLAGS)  app-launcher-riscv.o shmem-riscv.o -o app-launcher-riscv 

voter-daemon-riscv.o: voter-daemon.c voteshmem.h
	$(RISCVGCC) $(CFLAGS) voter-daemon.c -c -o voter-daemon-riscv.o

vote-client-riscv: vote-client-riscv.s ftlib-riscv.o
	$(RISCVGCC) shmem-riscv.o vote-client-riscv.s ftlib-riscv.o -o vote-client-riscv

voter-daemon-riscv: voter-daemon-riscv.o shmem-riscv.o
	$(RISCVGCC) voter-daemon-riscv.o shmem-riscv.o -o voter-daemon-riscv

run: $(BINARIES)
	./app-launcher 3 ./vote-client ./voter-daemon

run-riscv: $(BINARIES_RISCV)
	./app-launcher-riscv 3 ./vote-client-riscv ./voter-daemon-riscv

clean: 
	rm -f *.o *.s *.ll $(BINARIES) $(BINARIES_RISCV)
