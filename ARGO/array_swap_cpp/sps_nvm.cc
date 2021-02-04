/*
 Author: Vaibhav Gogte <vgogte@umich.edu>
         Aasheesh Kolli <akolli@umich.edu>

  This microbenchmark swaps two items in an array.
*/


#include <iostream>
#include <pthread.h>
#include <cstdint>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <fstream>

#define NUM_SUB_ITEMS 2 
#define NUM_OPS 10000000
#define NUM_ROWS 100000
#define NUM_THREADS 1 


struct Element {
  int32_t value_[NUM_SUB_ITEMS];
  Element& operator=(Element& other) {
    for (int i= 0; i< NUM_SUB_ITEMS; i++) {
      *(value_ + i) = *(other.value_ + i);
    }
    return *this;
  }
};


struct Datum {
  // pointer to the hashmap
  Element* elements_;
  // A lock which protects this Datum
  pthread_mutex_t* lock_;

};

struct sps {
  Datum* array;
  int num_rows_;
  int num_sub_items_;
};


sps* S;

void datum_init(sps* s) {
  for(int i = 0; i < NUM_ROWS; i++) {
  s->array[i].elements_ = (Element*)malloc(sizeof(Element));
  s->array[i].lock_ = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(s->array[i].lock_, NULL);
  
  for(int j = 0; j < NUM_SUB_ITEMS; j++)
    s->array[i].elements_->value_[j] = i+j;
  }
}


void datum_free(sps* s) {
  for(int i = 0; i < NUM_ROWS; i++) {
    free(s->array[i].elements_);
    free(s->array[i].lock_);

  }
}

void initialize() {
  S = (sps *)malloc(sizeof(sps));
  S->num_rows_ = NUM_ROWS;
  S->num_sub_items_ = NUM_SUB_ITEMS;
  S->array = (Datum *)malloc(sizeof(Datum)*NUM_ROWS);
  datum_init(S);
  
  
  fprintf(stderr, "Created array at %p\n", (void *)S);

}

bool swap(unsigned int index_a, unsigned int index_b) {
  //check if index is out of array
  assert (index_a < NUM_ROWS && index_b < NUM_ROWS); 

  //exit if swapping the same index
  if (index_a == index_b)
    return true;

  //enforce index_a < index_b
  if (index_a > index_b) {
    int index_tmp = index_a;
    index_a = index_b;
    index_b = index_tmp;
  }

  pthread_mutex_lock(S->array[index_a].lock_);
  pthread_mutex_lock(S->array[index_b].lock_);

  //swap array values
  Element temp;
  temp = *(S->array[index_a].elements_);
  *(S->array[index_a].elements_) = *(S->array[index_b].elements_);
  *(S->array[index_b].elements_) = temp;

  pthread_mutex_unlock(S->array[index_a].lock_);
  pthread_mutex_unlock(S->array[index_b].lock_);

  return true;
}

void* run_stub(void* ptr) {
  for (int i = 0; i < NUM_OPS/NUM_THREADS; ++i) {
    int index_a = rand()%NUM_ROWS;
    int index_b = rand()%NUM_ROWS;
    swap(index_a, index_b);
  }
  return NULL;
}

int main(int argc, char** argv) {
  std::cout << "In main\n" << std::endl;
  struct timeval tv_start;
  struct timeval tv_end;

  std::ofstream fexec;
  fexec.open("exec.csv",std::ios_base::app);


  // This contains the Atlas restart code to find any reusable data
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

  fexec << "SPS" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl;


  fexec.close();

  datum_free(S);
  free(S->array);
  free(S);
  return 0;
}
