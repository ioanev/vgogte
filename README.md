# The VGOGTE Parallel Benchmarks in C++

There is no official repository available.

## Implementations

**PTHREADS** - This directory contains the parallel Pthread version of the benchmarks.

**ARGO** - This directory contains the parallel ArgoDSM/Pthread version of the benchmarks.

## Benchmarks

```sh
Concurrent queue (CQ) - Insert/Delete nodes in a queue
Array Swap (SPS)      - Random swap of array elements
Persistent Cache (PC) - Update entries in persistent hash table
RB-tree               - Insert/Delete nodes in RB-Tree
TATP                  - Update location trans. from TATP
TPCC                  - New Order trans. from TPCC
```

## Software Requirements

**ARGO** - Assumes you have already installed [argodsm](https://github.com/etascale/argodsm).

## Building

Enter the directory from the version desired and execute:
```sh
$ cd <benchmark_name>
$ make
```

## Executing

Run each benchmark:
```sh
$ (mpirun $OMPIFLAGS) ./binary_name
```

Each benchmark can also be configured with the number of threads (`NUM_THREADS`), number of operations per thread (`NUM_OPS`), etc.. The benchmarks also have specific parameters, such as number of sub operations per operation, number of sub elements per datum, etc.

## References

Please cite the following works if you use these benchmarks:

1. Vaibhav Gogte, Stephan Diestelhorst, William Wang, Satish Narayanasamy, Peter M. Chen, and Thomas F. Wenisch. 2018. Persistency for synchronization-free regions. In Proceedings of the 39th ACM SIGPLAN Conference on Programming Language Design and Implementation (PLDI 2018). ACM, New York, NY, USA, 46-61. DOI: https://doi.org/10.1145/3192366.3192367

2. Aasheesh Kolli, Vaibhav Gogte, Ali Saidi, Stephan Diestelhorst, Peter M. Chen, Satish Narayanasamy, and Thomas F. Wenisch. 2017. Language-level persistency. In Proceedings of the 44th Annual International Symposium on Computer Architecture (ISCA '17). ACM, New York, NY, USA, 481-493. DOI: https://doi.org/10.1145/3079856.3080229
