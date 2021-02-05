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


#define NUM_SUBSCRIBERS 10000
#define NUM_OPS_PER_CS 2
#define NUM_OPS 10000
#define NUM_THREADS 4

int workrank;
int numtasks;

// Macro for only node0 to do stuff
#define WEXEC(inst) ({ if (workrank == 0) inst; })

TATP_DB* my_tatp_db;
//#include "../DCT/rdtsc.h"

void init_db() {
	unsigned num_subscribers = NUM_SUBSCRIBERS;
	my_tatp_db = argo::conew_<TATP_DB>();
	WEXEC(my_tatp_db->initialize(num_subscribers,NUM_THREADS));
	WEXEC(fprintf(stderr, "Created tatp db at %p\n", (void *)my_tatp_db));
}


void* update_locations(void* args) {
	int thread_id = *((int*)args);
	for(int i=0; i<NUM_OPS/(NUM_THREADS*numtasks); i++) {
		my_tatp_db->update_location(thread_id,NUM_OPS_PER_CS);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	argo::init(500*1024*1024UL);

	workrank = argo::node_id();
	numtasks = argo::number_of_nodes();

	WEXEC(printf("in main\n"));

	struct timeval tv_start;
	struct timeval tv_end;
	std::ofstream fexec;
	WEXEC(fexec.open("exec.csv",std::ios_base::app));

	init_db();

	WEXEC(std::cout<<"done with initialization"<<std::endl);

	WEXEC(my_tatp_db->populate_tables(NUM_SUBSCRIBERS));
	WEXEC(std::cout<<"done with populating tables"<<std::endl);
	argo::barrier();


	pthread_t threads[NUM_THREADS];
	int global_tid[NUM_THREADS];

	gettimeofday(&tv_start, NULL);

	for(int i=0; i<NUM_THREADS; i++) {
		global_tid[i] = workrank*NUM_THREADS + i;
		pthread_create(&threads[i], NULL, update_locations, (void*)(global_tid+i));
	}



	for(int i=0; i<NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	argo::barrier();
	gettimeofday(&tv_end, NULL);
	WEXEC(fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));



	WEXEC(fexec << "TATP" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);


	WEXEC(fexec.close());
	argo::codelete_(my_tatp_db);

	WEXEC(std::cout<<"done with threads"<<std::endl);

	argo::finalize();

	return 0;
}
