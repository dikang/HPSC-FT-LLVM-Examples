# HPSC FT (Fault Tolerance) LLVM Extension

This tutorial shows an example of how to use HPSC FT pragma in a C program, and how to generate binary executables for x86 and RISCV architectures.
This tutorial also provides a software voter code to run and test FT voting routines.
If you want to build and run the examples with software voter daemon provided, please see [README](./README.md).

- [HPSC FT Directives](#hpscdirectives)
- [HPSC FT Clauses](#hpscclauses)
- [A Simple Demo](#quickdemo)
- [Note on compiler optimization and performance](#performance)

HPSC FT LLVM Extension provides a set of C pragma with which Clang inserts voting routines automatically.

## <a name="hpscdirectives"></a> HPSC FT Directives

Provides Clang with information of the variables to vote.

The following directives are supported.

|Directive|Description|
|---------|-----------|
|[nmr](#nmr)|Defines a region, where the variables specified in the clauses are voted.|
|[vote](#vote)|The variables specified in the clause are voted now.|

### <a name="nmr"></a> nmr

Defines a region, where the variables specified in the clauses are voted.

```cpp
#pragma ft nmr [clauses]
{
   code_block
}
```

#### Parameters

*clauses*<br/>
One or more clauses, see the **Remarks** section.

##### Remarks

The `ft` directive supports the following clauses

- [lhs](#lhs)
- [rhs](#rhs)


#### Example

The following sample shows that variables i,j are to be voted right after loading, variable k is to be voted right after storing.

```cpp
int i, j, k;

  #pragma ft nmr lhs(b) rhs(i,j)
  {
    k = i + j;
  }

```

### <a name="vote"></a> vote

Generates vote function call for the variables specified in its argument list

```cpp
#pragma ft vote [(var:size)]
```

#### Parameters

*var:size*<br/>
A comma-separated list of *var:size* that you want to vote here.

(Optional) *:size* field is optional.

#### Example

```cpp
int i, j, k;
int * data;
int sdata[1024];

data = (int*)malloc(1024*sizeof(int));

// vote the variables 'i', 'j', and 'k'.
// compiler can figure out the size of those variables.
#pragma ft vote(i,j,k)

// vote the array which is pointed by 'data'.
// compiler cannot figure out the size of the storage pointed by 'data' correctly.
// its size is provided by the user.
#pragma ft vote(data:1024*sizeof(int))

// vote the array 'sdata'.
// compiler can figure out the size of the storage pointed by 'sdata' correctly.
#pragma ft vote(sdata)
```

## <a name="hpscclauses"></a> HPSC FT Clauses

### <a name="lhs"></a> lhs

Specifies a list of variables whose left-hand-side(lhs) values are to be voted after storing them.

```cpp
lhs(var:size)
```

#### Parameters

*var:size*<br/>
A comma-separated list of *var:size* that you want to vote after storing them.

(Optional) *:size* field is optional. For a statically allocated variable, compiler can detect its size correctly. For a dynamically allocated variable, compiler cannot detect its size at compile time. It is user's responsibility to provide its size.

#### Example

```cpp
int i, j, k;
int * data;

// i value is voted after executing 'i = j * k' statement
#pragma ft nmr lhs(i)
{
i = j * k;
}

data = (int*)malloc(1024*sizeof(int));

// At each interation, data[i] will be voted after executing 'data[i] = i;' statement.
// Since data is a pointer to a dynamically allocated memory, we need to provide size information
#pragma ft nmr lhs(data:sizeof(int))
{
for (i = 0; i < 1024; i++) {
  data[i] = i;
}
}
```

### <a name="rhs"></a> rhs

Specifies a list of variables whose right-hand-size(rhs) values are to be voted before loading them.

```cpp
rhs(var:size)
```

#### Parameters

*var:size*<br/>
A comma-separated list of var:size that you want to vote after storing them.
(Optional) *:size* field is optional. For a statically allocated variable, compiler can detect its size correctly. For a dynamically allocated variable, compiler cannot detect its size at compile time. It is user's responsibility to provide its size.

#### Example

```cpp
int i, j, k;
int * data;

// the values of variables *j*, *k* arae voted before executing 'i = j * k;' statement
#pragma ft nmr rhs(j,k)
{
i = j * k;
}

data = (int*)malloc(1024*sizeof(int));

// At each interation, data[i] will be voted before executing 'k += data[i];' statement.
// Since data is a pointer to a dynamically allocated memory, we need to provide size information.
#pragma ft nmr rhs(data:sizeof(int))
{
for (i = 0; i < 1024; i++) {
  k += data[i];
}
}
```

## <a name="quickdemo"></a> Example (A Simple Demo) 

The following sample code shows how to used HPSC FT pragma and library in a C program. It is assumed that you installed HPSC FT LLVM and use it. 

```cpp
// test.c code
#include <ft.h>

typedef struct t {
  int f2;
} test;

int main()
{

  test t1;
  int i, j;
  int sdata[1024];
  int * data;

  ft_init();  // initialization fault-tolenrace library

  data = sdata;
  j = 0;

  #pragma ft nmr rhs(j) lhs(data,t1)
  {
     for (i = 0; i < 1024; i++) {
       data[i] = i * j++;
       t1.f2 = i;
     }

  }

#pragma ft vote(sdata)
#pragma ft vote(data:sizeof(int)*1024)

  ft_exit();  // exit of fault-tolenrace library
}
```

You can compile it using the following command.
`-fft` option enables proper linking of necessary libraries at link time, which is not needed for this demo.
Compiler generates llvm intermediate representation (IR) code with `-S -emit-llvm` options.
If the compiler cannot find `ft.h` file, its installation is not properly done.
This tutorial has a copy of `ft.h` file.

`-fft-debug-mode` option enables printout showing which variable and where it is voted in the source code at run-time.

```bash
$ clang -fft -S -emit-llvm test.c -o test.ll
```

When you use clang as a cross-compiler, and use a architecture specific back-end tools, you need to generate an assembly code using clang.
You can then use the architecture specific back-end tool to assemble and link the assembly code to make a final executable.
Here we show how to check if HPSC LLVM FT generates the proper code for HPSC FT pragmas.

The following code shows both a code snippet in the test.c code shown above and its corresponding llvm IR code generated by HPSC LLVM compiler
C++ style comments starting with `//` are added for explanation.

The first pragma is to show automatic generation of vote routine for right-hand-side value of variable `j`, and left-hand-side value of variables `data` and `t1`.
`@__ft_voter` is call for the vote routine for rhs value.
`@__ft_votel` is call for the vote routine for lhs value.
Their first argument is a pointer to variable, and their second argument is the size of the variable.
They are functionally equivalent, but their name distinguishes from which clauses they are derived.
```cpp
  // test.c 
  #pragma ft nmr rhs(j) lhs(data,t1)
  {
     for (i = 0; i < 1024; i++) {
       data[i] = i * j++;
       t1.f2 = i;
     }

  }
  --> translated by HPSC LLVM Compiler
  // test.ll
  for.body:                                         ; preds = %for.cond
    %1 = load i32, ptr %i, align 4
    %2 = call i32 @__ft_voter(ptr %j, i32 4)		// <-- from #pragma ft nmr rhs(j)
    %3 = load i32, ptr %j, align 4			// load of j
    ...
    %arrayidx = getelementptr inbounds i32, ptr %4, i64 %idxprom
    store i32 %mul, ptr %arrayidx, align 4		// store of data[i]
    %6 = call i32 @__ft_votel(ptr %arrayidx, i32 4)	// <-- from #pragma ft nmr lhs(data)
    ...
    %f2 = getelementptr inbounds %struct.t, ptr %t1, i32 0, i32 0
    store i32 %7, ptr %f2, align 4			// store of t1.f2
    %8 = call i32 @__ft_votel(ptr %f2, i32 4)		// <-- from #pragma ft nmr lhs(t1)
    ...
```

The second and third pragmas are to show the difference between static array and dynamically allocated array.
`sdata` is static array and its size is known to the compiler.
`data` is an integer pointer and the size of the storage it points can be dynamic at run-time.
To correctly vote the storage pointed by `data`, the user has to provide size.
In the example `:sizeof(int)*1024` denotes the size.
The second and third pragmas do the same job, since both `sdata` and `data` are the same.
`@__ft_votenow` is the library call for the vote routine for the clause `vote()`.
Its first argument is a pointer to variable, and its second argument is the size of the variable.
It is functionally equivalent to `@__ft_voter()` and `@__ft_votel()`, but its name shows from which clause it is derived.

```cpp
  // test.c 
  #pragma ft vote(sdata)
  #pragma ft vote(data:sizeof(int)*1024)

  --> translated by HPSC LLVM Compiler
  // test.ll
  for.end:                                          ; preds = %for.cond
    %10 = call i32 @__ft_votenow(ptr %sdata, i32 4096)	// <-- 	from #pragma ft vote(sdata)
    %11 = load ptr, ptr %data, align 8
    %12 = call i32 @__ft_votenow(ptr %11, i32 4096)	// <-- from #pragma ft vote(data:sizeof(int)*1024)
    %call2 = call i32 (...) @ft_exit()
```

## <a name="performance"></a> Note on compiler optimization and performance

The user needs to be aware of the interactions between HPSC FT pragmas and the compiler optimization to get the best performance.
HPSC FT LLVM inserts vote functions when load/store happens in the program. Those vote functions may affect the compiler in generating code. As an example, compiler optimizes temporary variables using registers, which saves execution time for load/store of those variables. However, if a user puts HPSC FT pragma to vote for a temporary variable, it forces the compiler to load/store the variable. It may end up performance loss. The following two example show the difference. In the following example, the loop variable `i` is to be voted. The compiler creates load/store instruction for the loop variable.

```cpp
  #pragma ft nmr rhs(i) lhs(j)			// vote when variable i is loaded, and variable j is stored
  {
  for (i = 0; i < 1000; i++) {
    j += i;
  }
  }
  --> translated by HPSC LLVM Compiler
  %i = alloca i32, align 4				// <-- 'i' is given a storage
  ...
  store i32 0, ptr %i, align 4, !tbaa !5
  %0 = call i32 @__ft_voter(ptr nonnull %i, i32 4) #2	// <-- from # pragma ft rhs(i)
  %1 = load i32, ptr %i, align 4, !tbaa !5
  ...

for.body:              ; preds = %entry, %for.body
  %2 = call i32 @__ft_voter(ptr nonnull %i, i32 4) #2	// <-- from # pragma ft rhs(i)
  %3 = load i32, ptr %i, align 4, !tbaa !5
  ...
  store i32 %add, ptr %j, align 4, !tbaa !5
  %5 = call i32 @__ft_votel(ptr nonnull %j, i32 4) #2	// <-- from # pragma ft lhs(j)
  %6 = call i32 @__ft_voter(ptr nonnull %i, i32 4) #2	// <-- from # pragma ft rhs(i)
  %7 = load i32, ptr %i, align 4, !tbaa !5
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %i, align 4, !tbaa !5
  %8 = call i32 @__ft_voter(ptr nonnull %i, i32 4) #2	// <-- from # pragma ft rhs(i)
  %9 = load i32, ptr %i, align 4, !tbaa !5
  %cmp = icmp slt i32 %9, 1000
  br i1 %cmp, label %for.body, label %for.end, !llvm.loop !9
```

In the following example, the loop variable `i` is not to be voted. The compiler does not create load/store instruction for the loop variable, which may result in faster execution.

```cpp
  #pragma ft nmr lhs(j)			// vote when variable j is stored
  for (i = 0; i < 1000; i++) {
    j += i;
  }
  --> translated by HPSC LLVM Compiler
  %i.03 = phi i32 [ 0, %entry ], [ %inc, %for.body ]	// <-- 'i' is given a register without load/store
  %0 = load i32, ptr %j, align 4, !tbaa !5
  %add = add nsw i32 %0, %i.03
  store i32 %add, ptr %j, align 4, !tbaa !5
  %1 = call i32 @__ft_votel(ptr nonnull %j, i32 4) #2	// <-- from # pragma ft lhs(j)
  %inc = add nuw nsw i32 %i.03, 1
  %exitcond.not = icmp eq i32 %inc, 1000
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !9
```

