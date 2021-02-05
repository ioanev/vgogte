/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file models the TPCC benchmark.
*/

#include <pthread.h>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <string>
#include <fstream>

#include "tpcc_db.h"

#define NUM_ORDERS 10000
#define NUM_THREADS 4 

#define NUM_WAREHOUSES 1
#define NUM_ITEMS 100
#define NUM_LOCKS NUM_WAREHOUSES*10 + NUM_WAREHOUSES*NUM_ITEMS

int workrank;
int numtasks;

// Macro for only node0 to do stuff
#define WEXEC(inst) ({ if (workrank == 0) inst; })

TPCC_DB* tpcc_db;


void initialize() {
	tpcc_db = argo::conew_<TPCC_DB>();
	WEXEC(tpcc_db->initialize(NUM_WAREHOUSES, NUM_THREADS*numtasks, NUM_LOCKS));
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

	WEXEC(std::cout<<"in main"<<std::endl);
	struct timeval tv_start;
	struct timeval tv_end;
	std::ofstream fexec;
	WEXEC(fexec.open("exec.csv",std::ios_base::app));

	initialize();


	WEXEC(std::cout<<"num_threads, num_orders = "<< NUM_THREADS*numtasks <<", "<<NUM_ORDERS <<std::endl);

	WEXEC(std::cout<<"done with initialization"<<std::endl);

	WEXEC(tpcc_db->populate_tables());

	WEXEC(std::cout<<"done with populating tables"<<std::endl);
	argo::barrier();

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
	argo::codelete_(tpcc_db);

	WEXEC(std::cout<<"done with threads"<<std::endl);

	argo::finalize();

	return 0;

}
