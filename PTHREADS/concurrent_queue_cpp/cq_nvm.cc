/*
 Author: Vaibhav Gogte <vgogte@umich.edu>
         Aasheesh Kolli <akolli@umich.edu>
*/


#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <string>
#include <fstream>
#include <unistd.h>

#include "cq.h"

#define NUM_SUB_ITEMS 64 
#define NUM_OPS 10000000
#define NUM_THREADS 12 

concurrent_queue* CQ;

void initialize() {
  CQ = (concurrent_queue*)malloc(sizeof(concurrent_queue));
  CQ->init(NUM_SUB_ITEMS);
  
  fprintf(stderr, "Created cq at %p\n", (void *)CQ);
}


void* run_stub(void* ptr) {
  int ret;
  for (int i = 0; i < NUM_OPS/NUM_THREADS; ++i) {
    CQ->push(i);
  }
  return NULL;
}

int main(int argc, char** argv) {
  std::cout << "In main\n" << std::endl;
  struct timeval tv_start;
  struct timeval tv_end;
  std::ofstream fexec;
  fexec.open("exec.csv",std::ios_base::app);

  initialize();

  pthread_t threads[NUM_THREADS];
  
  gettimeofday(&tv_start, NULL);
  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_create(&threads[i], NULL, &run_stub, NULL);
  }
  
  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&tv_end, NULL);

  fprintf(stderr, "time elapsed %ld us\n",
          tv_end.tv_usec - tv_start.tv_usec +
          (tv_end.tv_sec - tv_start.tv_sec) * 1000000);


  fexec << "CQ" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl;
  fexec.close();

  free(CQ);
  
  return 0;
}
