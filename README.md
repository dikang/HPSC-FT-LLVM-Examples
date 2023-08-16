# An Example of HPSC FT (Fault Tolerance) LLVM Extension

This directory has an example of C file with HPSC FT pragma, FT library and software voter.
For a short tutorial of HPSC FT LLVM extension, see [HPSC FT reference](./HPSC-FT-reference.md).

## How to build

We assume to use an X86 host machine to build this example.
We also assume that you installed HPSC FT LLVM and use its clang to build this example.
You have to set up the CLANG environment variable in Makefile to the clang of HPSC FT LLVM.

```bash
CLANG=./bin/clang
```

All the source code is in `src` directory.

```bash
$cd src
```

The following environment variables in Makefile need to be set up properly on your system, if you want to generate executables for RISCV.

```bash
RISCVGCC=/HPSC/Microchip/tools/riscv64-unknown-linux-gnu-toolsuite-14.9.0-2022.12.0/bin/riscv64-unknown-linux-gnu-gcc
RISCVGCCHOME=/HPSC/Microchip/tools/riscv64-unknown-linux-gnu-toolsuite-14.9.0-2022.12.0/lib/gcc/riscv64-unknown-linux-gnu/11.3.1/
RISCVSYSROOT=/HPSC/Microchip/tools/riscv64-unknown-linux-gnu-toolsuite-14.9.0-2022.12.0/sysroot/
RISCVINCLUDE=-I$(RISCVGCCHOME)/include -I$(RISCVGCCHOME)/include-fixed -I$(RISCVGCCHOME)/../../../../riscv64-unknown-linux-gnu/include -I$(RISCVSYSROOT)/usr/include
```

Once those environment are set up properly, you can run the following command.

```bash
$ make
```

It will generate the following binaries:

```bash
app-launcher  		// launcher for both voter and client programs (for x86)
vote-client  		// client program running in NMR (N-modular reduncancy) fashion (for x86)
voter-daemon   		// software voter (for x86)
app-launcher-riscv 	// (for riscv) 
vote-client-riscv  	// (for riscv)
voter-daemon-riscv	// (for riscv)
```

## How to run

`app-launcher` launches both voter daemon and clients. Here is an example to run the client in TMR (Triple modular redundancy). The first argument of `app-launcher` determines how many copies of the clients will run.
The `vote-client` program has built-in error injection, which will cause report errors from `voter-daemon`.

```bash
$ ./app-launcher
uage: app-launcher <number of processes> <path of the executable> <path of the voter binary>

$ ./app-launcher 3 ./vote-client ./voter-daemon
  --- iteration (1)
  --- iteration (2)
  --- iteration (3)
Client 1: Recoverable error with (1) errors in (4) bytes
Client 1: Recoverable error with (2) errors in (8) bytes
  --- iteration (4)
  --- iteration (5)
  --- iteration (6)
Client 1: Recoverable error with (1) errors in (4) bytes
  --- iteration (7)
Client 1: Recoverable error with (2) errors in (8) bytes
  --- iteration (8)
  --- iteration (9)
Client 1: Recoverable error with (1) errors in (4) bytes
Client 1: Recoverable error with (2) errors in (8) bytes
  --- iteration (10)
```

`Recoverable errors with (1) error in (4) bytes` is reported by the first pragma `#pragma ft nmr rhs(b)`, because of the errors injected in the following line.

```cpp
    if (count % 3 == 0 && core_id == 1) c[count]++;    // error injection on 'b[3,6,9]' through c
```

`Recoverable error with (2) errors in (8) bytes` is reported by the second pragma `#pragma ft vote(b:sizeof(int)*2)`, which votes the first elements of array `b`.
Two errors are injected on `b[0]` and `b[1]` in the following lines.

```cpp
      if (count % 3 == 0 && core_id == 1) a++;  // error injection on a, which will corrupt b[0..1] below.

      b[0] = a;
      b[1] = a+1;
```

