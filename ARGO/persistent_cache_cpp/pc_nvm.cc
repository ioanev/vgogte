/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file uses the PersistentCache to enable multi-threaded updates to it.
*/

#include "argo.hpp"

#include <iostream>
#include <cstdint>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <fstream>

#define NUM_ELEMS_PER_DATUM 2 
#define NUM_ROWS 1000
#define NUM_UPDATES 1000
#define NUM_THREADS 4

int workrank;
int numtasks;

// Macro for only node0 to do stuff
#define WEXEC(inst) ({ if (workrank == 0) inst; })

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
	argo::globallock::global_tas_lock* lock_;
	// A flag attached to this lock
	bool* lock_flag_;
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
		p->hashmap[i].elements_ = argo::new_<Element>();
		p->hashmap[i].lock_flag_ = argo::new_<bool>(false);
		p->hashmap[i].lock_ = argo::new_<argo::globallock::global_tas_lock>(p->hashmap[i].lock_flag_);

		for(int j = 0; j < NUM_ELEMS_PER_DATUM; j++)
			p->hashmap[i].elements_->value_[j] = 0;
	}
}

void datum_free(pc* p) {
	for(int i = 0; i < NUM_ROWS; i++) {
		argo::delete_(p->hashmap[i].elements_);
		argo::delete_(p->hashmap[i].lock_flag_);
		argo::delete_(p->hashmap[i].lock_);
	}
}



void datum_set(int key, int value) {
	P->hashmap[key].lock_->lock();

	for(int j = 0; j < NUM_ELEMS_PER_DATUM; j++)
		P->hashmap[key].elements_->value_[j] = value+j;

	P->hashmap[key].lock_->unlock();
}


void initialize() {
	P = argo::conew_<pc>();
	P->num_rows_ = NUM_ROWS;
	P->num_elems_per_row_ = NUM_ELEMS_PER_DATUM;
	P->hashmap = argo::conew_array<Datum>(NUM_ROWS);
	WEXEC(datum_init(P));
	argo::barrier();

	WEXEC(fprintf(stderr, "Created hashmap at %p\n", (void *)P->hashmap));
}



void* CacheUpdates(void* arguments) {

	int key;
	for (int i = 0; i < NUM_UPDATES/(NUM_THREADS*numtasks); i++) {
		key = rand()%NUM_ROWS;
		datum_set(key, i);
	}
	return 0;

}


int main (int argc, char* argv[]) {
	argo::init(500*1024*1024UL);

	workrank = argo::node_id();
	numtasks = argo::number_of_nodes();

	WEXEC(std::cout << "In main\n" << std::endl);
	struct timeval tv_start;
	struct timeval tv_end;


	std::ofstream fexec;
	WEXEC(fexec.open("exec.csv",std::ios_base::app));

	// This contains the Atlas restart code to find any reusable data
	initialize();

	WEXEC(std::cout << "Done with cache creation" << std::endl);

	pthread_t threads[NUM_THREADS];

	gettimeofday(&tv_start, NULL);
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, CacheUpdates, NULL);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	argo::barrier();

	gettimeofday(&tv_end, NULL);

	WEXEC(fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));

	WEXEC(fexec << "PC" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);

	WEXEC(fexec.close());

	WEXEC(datum_free(P));
	argo::codelete_array(P->hashmap);
	argo::codelete_(P);


	WEXEC(std::cout << "Done with persistent cache" << std::endl);

	argo::finalize();

	return 0;
}
