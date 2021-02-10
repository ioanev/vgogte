/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This microbenchmark swaps two items in an array.
*/

#include "argo.hpp"
#include "../common/wtime.hpp"

#include <iostream>
#include <pthread.h>
#include <cstdint>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <fstream>

#define NUM_SUB_ITEMS 64
#define NUM_OPS 1000
#define NUM_ROWS 1000000
#define NUM_THREADS 4

int workrank;
int numtasks;

lock_barr_t argo_stats;

// Macro for only node0 to do stuff
#define WEXEC(inst) ({ if (workrank == 0) inst; })

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
	argo::globallock::global_tas_lock* lock_;
	// A flag attached to this lock
	bool* lock_flag_;
};

struct sps {
	Datum* array;
	int num_rows_;
	int num_sub_items_;
};


sps* S;

void datum_init(sps* s) {
	for(int i = 0; i < NUM_ROWS; i++) {
		s->array[i].elements_ = argo::new_<Element>();
		s->array[i].lock_flag_ = argo::new_<bool>(false);
		s->array[i].lock_ = argo::new_<argo::globallock::global_tas_lock>(s->array[i].lock_flag_);

		for(int j = 0; j < NUM_SUB_ITEMS; j++)
			s->array[i].elements_->value_[j] = i+j;
	}
}


void datum_free(sps* s) {
	for(int i = 0; i < NUM_ROWS; i++) {
		argo::delete_(s->array[i].elements_);
		argo::delete_(s->array[i].lock_flag_);
		argo::delete_(s->array[i].lock_);
	}
}

void initialize() {
	S = argo::conew_<sps>();
	S->num_rows_ = NUM_ROWS;
	S->num_sub_items_ = NUM_SUB_ITEMS;
	S->array = argo::conew_array<Datum>(NUM_ROWS);
	WEXEC(datum_init(S));
	argo_barrier();


	WEXEC(fprintf(stderr, "Created array at %p\n", (void *)S->array));

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

	argo_lock(S->array[index_a].lock_);
	argo_lock(S->array[index_b].lock_);

	//swap array values
	Element temp;
	temp = *(S->array[index_a].elements_);
	*(S->array[index_a].elements_) = *(S->array[index_b].elements_);
	*(S->array[index_b].elements_) = temp;

	argo_unlock(S->array[index_a].lock_);
	argo_unlock(S->array[index_b].lock_);

	return true;
}

void* run_stub(void* ptr) {
	for (int i = 0; i < NUM_OPS/(NUM_THREADS*numtasks); ++i) {
		int index_a = rand()%NUM_ROWS;
		int index_b = rand()%NUM_ROWS;
		swap(index_a, index_b);
	}
	return NULL;
}

int main(int argc, char** argv) {
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

	pthread_t threads[NUM_THREADS];

	gettimeofday(&tv_start, NULL);
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&threads[i], NULL, &run_stub, NULL);
	}   

	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}
	argo_barrier();

	gettimeofday(&tv_end, NULL);
	WEXEC(fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));

	WEXEC(fexec << "SPS" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);

	WEXEC(fexec.close());

	WEXEC(datum_free(S));
	argo::codelete_array(S->array);
	argo::codelete_(S);

	print_argo_stats();

	argo::finalize();

	return 0;
}
