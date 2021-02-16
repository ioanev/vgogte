/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file models the TPCC benchmark.
*/

#include "tpcc_db.h"

#include <pthread.h>
#include <sys/time.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

int workrank;
int numtasks;

TPCC_DB* tpcc_db;

void initialize() {
	tpcc_db = new TPCC_DB;
	tpcc_db->initialize(NUM_WAREHOUSES, NUM_THREADS*numtasks, NUM_LOCKS);
	argo::barrier();
	
	WEXEC(fprintf(stderr, "Created tpcc at %p\n", (void *)tpcc_db));
}

//void new_orders(TxEngine* tx_engine, int tx_engn_type, TPCC_DB* tpcc_db, int thread_id, int num_orders, int num_threads, int num_strands_per_thread, std::atomic<bool>*wait) {
void* new_orders(void* arguments) {
	int thread_id = *((int*)arguments);

	for(int i=0; i<NUM_ORDERS/(NUM_THREADS*numtasks); i++) {
		int w_id = 1;
		//There can only be 10 districts, this controls the number of locks in tpcc_db, which is why NUM_LOCKS = warehouse*10
		int d_id = tpcc_db->get_random(thread_id, 1, 10);
		int c_id = tpcc_db->get_random(thread_id, 1, 3000);

		tpcc_db->new_order_tx(thread_id, w_id, d_id, c_id);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	argo::init(500*1024*1024UL);

	workrank = argo::node_id();
	numtasks = argo::number_of_nodes();

	WEXEC(std::cout<<"In main"<<std::endl);
	struct timeval tv_start;
	struct timeval tv_end;

	std::ofstream fexec;
	WEXEC(fexec.open("exec.csv",std::ios_base::app));

	initialize();
	WEXEC(std::cout<<"num_threads, num_orders = "<< NUM_THREADS*numtasks <<", "<<NUM_ORDERS <<std::endl);
	WEXEC(std::cout<<"Done with initialization"<<std::endl);

	tpcc_db->populate_tables();
	WEXEC(std::cout<<"Done with populating tables"<<std::endl);

	pthread_t threads[NUM_THREADS];
	int global_tid[NUM_THREADS];

	gettimeofday(&tv_start, NULL);
	for(int i=0; i<NUM_THREADS; i++) {
		global_tid[i] = workrank*NUM_THREADS + i;
		pthread_create(&threads[i], NULL, new_orders, (void *)(global_tid+i));
	}
	for(int i=0; i<NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	argo::barrier();
	gettimeofday(&tv_end, NULL);

	WEXEC(fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));
	WEXEC(fexec << "TPCC" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);
	WEXEC(fexec.close());
	
	delete tpcc_db;

	WEXEC(std::cout<<"Done with threads"<<std::endl);

	argo::finalize();

	return 0;
}
