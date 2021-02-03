Benchmarks: array_swap, concurrent_queue, persistent_cache,red_black_tree, TATP, TPCC

Compile each benchmark:
  cd <benchmark_name>
  make

Run each benchmark:
  ./binary_name

Each benchmark can also be configured with the number of threads (NUM_THREADS), number of operations per thread etc (NUM_OPS). 
The benchamrks also have specific parameters such as number of sub operations per operation, number of sub elements per datum etc.

Please cite following works if you use these benchmarks:

1. Vaibhav Gogte, Stephan Diestelhorst, William Wang, Satish Narayanasamy, Peter M. Chen, and Thomas F. Wenisch. 2018. Persistency for synchronization-free regions. In Proceedings of the 39th ACM SIGPLAN Conference on Programming Language Design and Implementation (PLDI 2018). ACM, New York, NY, USA, 46-61. DOI: https://doi.org/10.1145/3192366.3192367

2. Aasheesh Kolli, Vaibhav Gogte, Ali Saidi, Stephan Diestelhorst, Peter M. Chen, Satish Narayanasamy, and Thomas F. Wenisch. 2017. Language-level persistency. In Proceedings of the 44th Annual International Symposium on Computer Architecture (ISCA '17). ACM, New York, NY, USA, 481-493. DOI: https://doi.org/10.1145/3079856.3080229



