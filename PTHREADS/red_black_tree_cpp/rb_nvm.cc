/*
  Author: Vaibhav Gogte <vgogte@umich.edu>
          Aasheesh Kolli <akolli@umich.edu>
*/ 

#include "rb.h"
#include <vector>
#include <iostream>
#include <cstdint>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <fstream>


#define TREE_LENGTH 100
#define NUM_UPDATES_PER_CS 64 
#define NUM_OPS 1000000
#define NUM_THREADS 12 


int* array;

Red_Black_Tree* RB;

void initialize() {
  RB = (Red_Black_Tree *)malloc(sizeof(Red_Black_Tree));
}



void* run_stub(void* ptr) {
  for (int i = 0; i < NUM_OPS/NUM_THREADS; ++i) {
    RB->rb_delete_or_insert(NUM_UPDATES_PER_CS);
  }
    
  return NULL;
}

// check if the two trees contain the same values
void verify(Node* root) {
  unsigned count = 0;
  Node* curr = root;
  while (count < MAX_LEN) {
    int val_1, val_2;
    val_1 = curr->val;
    val_2 = (curr + MAX_LEN)->val;
    assert(val_2 == val_1 && "Two trees do not contain the same values");
    curr++;
    count++;
  }
}

int main(int argc, char** argv) {
  std::cout << "In main\n" << std::endl;
  struct timeval tv_start;
  struct timeval tv_end;
  std::ofstream fexec;
  fexec.open("exec.csv",std::ios_base::app);
  
  // This contains the Atlas restart code to find any reusable data
  initialize();

  array = (int*)malloc(TREE_LENGTH * sizeof(int));

  for (int i = 0; i < TREE_LENGTH; ++i) {
      array[i] = i;
  }

  Node* root = (Node*)malloc(2 * sizeof(Node) * MAX_LEN);
  RB->initialize(root, array, TREE_LENGTH);
  std::cout << "Done with RBtree creation" << std::endl;

  pthread_t T[NUM_THREADS];

  gettimeofday(&tv_start, NULL);

  for (int i = 0; i < NUM_THREADS; ++i) {
      pthread_create(&T[i], NULL, &run_stub, NULL);
  }

  for (int i = 0; i < NUM_THREADS; ++i) {
      pthread_join(T[i], NULL);
  }

  gettimeofday(&tv_end, NULL);

  fprintf(stderr, "time elapsed %ld us\n",
          tv_end.tv_usec - tv_start.tv_usec +
              (tv_end.tv_sec - tv_start.tv_sec) * 1000000);

  fexec << "RB" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl;


  fexec.close();

  std::cout << "Done with RBtree" << std::endl;

  free(array);
  free(root);
  free(RB);
  std::cout << "Finished!" << std::endl;

  return 0;
}
