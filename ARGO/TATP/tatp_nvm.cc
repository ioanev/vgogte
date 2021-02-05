/*
 Author: Vaibhav Gogte <vgogte@umich.edu>
         Aasheesh Kolli <akolli@umich.edu>


This file is the TATP benchmark, performs various transactions as per the specifications.
*/

#include "tatp_db.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstdint>
#include <assert.h>
#include <sys/time.h>
#include <string>
#include <fstream>


#define NUM_SUBSCRIBERS 100000
#define NUM_OPS_PER_CS 2 
#define NUM_OPS 10000000
#define NUM_THREADS 12 

TATP_DB* my_tatp_db;
//#include "../DCT/rdtsc.h"

void init_db() {
  unsigned num_subscribers = NUM_SUBSCRIBERS;
  my_tatp_db = (TATP_DB *)malloc(sizeof(TATP_DB));
  my_tatp_db->initialize(num_subscribers,NUM_THREADS);
  fprintf(stderr, "Created tatp db at %p\n", (void *)my_tatp_db);
}


void* update_locations(void* args) {
  int thread_id = *((int*)args);
  for(int i=0; i<NUM_OPS/NUM_THREADS; i++) {
    my_tatp_db->update_location(thread_id,NUM_OPS_PER_CS);
  }
  return 0;
}

int main(int argc, char* argv[]) {

  printf("in main\n");

  struct timeval tv_start;
  struct timeval tv_end;
  std::ofstream fexec;
  fexec.open("exec.csv",std::ios_base::app);

  init_db();

  std::cout<<"done with initialization"<<std::endl;

  my_tatp_db->populate_tables(NUM_SUBSCRIBERS);
  std::cout<<"done with populating tables"<<std::endl;


  pthread_t threads[NUM_THREADS];
  int id[NUM_THREADS];

  gettimeofday(&tv_start, NULL);

  for(int i=0; i<NUM_THREADS; i++) {
    id[i] = i;
    pthread_create(&threads[i], NULL, update_locations, (void*)(id+i));
  }



  for(int i=0; i<NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  gettimeofday(&tv_end, NULL);
  fprintf(stderr, "time elapsed %ld us\n",
          tv_end.tv_usec - tv_start.tv_usec +
              (tv_end.tv_sec - tv_start.tv_sec) * 1000000);



  fexec << "TATP" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl;


  fexec.close();
  free(my_tatp_db);

  std::cout<<"done with threads"<<std::endl;


  return 0;
}
