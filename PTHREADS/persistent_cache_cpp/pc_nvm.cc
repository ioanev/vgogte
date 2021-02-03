/*
 Author: Vaibhav Gogte <vgogte@umich.edu>
         Aasheesh Kolli <akolli@umich.edu>

 This file uses the PersistentCache to enable multi-threaded updates to it.
*/

#include <iostream>
#include <cstdint>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <fstream>

#define NUM_ELEMS_PER_DATUM 2 
#define NUM_ROWS 100000
#define NUM_UPDATES 10000000
#define NUM_THREADS 12 

// Persistent cache organization. Each Key has an associated Value in the data array.
// Each datum in the data array consists of up to 8 elements. 
// The number of elements actually used by the application is a runtime argument.
struct Element {
  int32_t value_[NUM_ELEMS_PER_DATUM];
};

struct Datum {
  // pointer to the hashmap
  Element* elements_;
  // A lock which protects this Datum
  pthread_mutex_t* lock_;
};


struct pc {
  Datum* hashmap;
  int num_rows_;
  int num_elems_per_row_;
};

uint32_t pc_rgn_id;
pc* P;


void datum_init(pc* p) {
  for(int i = 0; i < NUM_ROWS; i++) {
    p->hashmap[i].elements_ = (Element*)malloc(sizeof(Element));
    p->hashmap[i].lock_ = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(p->hashmap[i].lock_, NULL);
    
    for(int j = 0; j < NUM_ELEMS_PER_DATUM; j++)
      p->hashmap[i].elements_->value_[j] = 0;
  }
}

void datum_free(pc* p) {
  for(int i = 0; i < NUM_ROWS; i++) {
    free(p->hashmap[i].elements_);
    free(p->hashmap[i].lock_);
  }
}



void datum_set(int key, int value) {
  pthread_mutex_lock(P->hashmap[key].lock_);
  
  for(int j = 0; j < NUM_ELEMS_PER_DATUM; j++)
      P->hashmap[key].elements_->value_[j] = value+j;
  
  pthread_mutex_unlock(P->hashmap[key].lock_);
}


void initialize() {
  P = (pc *)malloc(sizeof(pc));
  P->num_rows_ = NUM_ROWS;
  P->num_elems_per_row_ = NUM_ELEMS_PER_DATUM;
  P->hashmap = (Datum *)malloc(sizeof(Datum)*NUM_ROWS);
  datum_init(P);
  
  fprintf(stderr, "Created hashmap at %p\n", (void *)P);
}



void* CacheUpdates(void* arguments) {

  int key;
  for (int i = 0; i < NUM_UPDATES/NUM_THREADS; i++) {
    key = rand()%NUM_ROWS;
    datum_set(key, i);
  }
  return 0;

}


int main (int argc, char* argv[]) {
  std::cout << "In main\n" << std::endl;
  struct timeval tv_start;
  struct timeval tv_end;


  std::ofstream fexec;
  fexec.open("exec.csv",std::ios_base::app);

  // This contains the Atlas restart code to find any reusable data
  initialize();

  std::cout << "Done with cache creation" << std::endl;

  pthread_t threads[NUM_THREADS];

  gettimeofday(&tv_start, NULL);
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, CacheUpdates, NULL);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  gettimeofday(&tv_end, NULL);

  fprintf(stderr, "time elapsed %ld us\n",
          tv_end.tv_usec - tv_start.tv_usec +
              (tv_end.tv_sec - tv_start.tv_sec) * 1000000);

  fexec << "PC" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl;

  fexec.close();

  datum_free(P);
  free(P->hashmap);
  free(P);


  std::cout << "Done with persistent cache" << std::endl;
  return 0;
}
