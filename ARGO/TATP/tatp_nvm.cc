/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file is the TATP benchmark, performs various transactions as per the specifications.
*/

#include "tatp_db.h"

#include <stdio.h>
#include <cstdint>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#include <string>
#include <fstream>
#include <iostream>

// Macro for only node0 to do stuff
#define MAIN_PROC(rank, inst) \
do { \
	if ((rank) == 0) { inst; } \
} while (0)

int workrank;
int numtasks;

TATP_DB* my_tatp_db;
//#include "../DCT/rdtsc.h"

void init_db() {
	unsigned num_subscribers = NUM_SUBSCRIBERS;
	my_tatp_db = new TATP_DB;
	my_tatp_db->initialize(num_subscribers,NUM_THREADS);
	argo::barrier();
	
	MAIN_PROC(workrank, fprintf(stderr, "Created tatp db at %p\n", (void *)my_tatp_db));
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

	MAIN_PROC(workrank, printf("In main\n"));
	struct timeval tv_start;
	struct timeval tv_end;
	
	std::ofstream fexec;
	MAIN_PROC(workrank, fexec.open("exec.csv",std::ios_base::app));

	init_db();
	MAIN_PROC(workrank, std::cout<<"Done with initialization"<<std::endl);

	my_tatp_db->populate_tables(NUM_SUBSCRIBERS);
	MAIN_PROC(workrank, std::cout<<"Done with populating tables"<<std::endl);

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

#if ENABLE_VERIFICATION == 1
	MAIN_PROC(workrank, my_tatp_db->verify());
#endif
	
	MAIN_PROC(workrank, fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));
	MAIN_PROC(workrank, fexec << "TATP" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);
	MAIN_PROC(workrank, fexec.close());
	
	delete my_tatp_db;

	MAIN_PROC(workrank, std::cout<<"Done with threads"<<std::endl);

	argo::finalize();

	return 0;
}
