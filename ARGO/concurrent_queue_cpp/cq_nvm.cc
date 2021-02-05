/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>
*/

#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include <string>
#include <fstream>
#include <unistd.h>

#include "cq.h"

#define NUM_SUB_ITEMS 64 
#define NUM_OPS 10000
#define NUM_THREADS 4 

int workrank;
int numtasks;

// Macro for only node0 to do stuff
#define WEXEC(inst) ({ if (workrank == 0) inst; })

concurrent_queue* CQ;

void initialize() {
	CQ = argo::conew_<concurrent_queue>();
	WEXEC(CQ->init(NUM_SUB_ITEMS));
	argo::barrier();

	WEXEC(fprintf(stderr, "Created cq at %p\n", (void *)CQ));
}


void* run_stub(void* ptr) {
	int ret;
	for (int i = 0; i < NUM_OPS/(NUM_THREADS*numtasks); ++i) {
		CQ->push(i);
	}
	return NULL;
}

int main(int argc, char** argv) {
	argo::init(1*1024*1024*1024UL);

	workrank = argo::node_id();
	numtasks = argo::number_of_nodes();

	WEXEC(std::cout << "In main\n" << std::endl);
	struct timeval tv_start;
	struct timeval tv_end;
	std::ofstream fexec;
	WEXEC(fexec.open("exec.csv",std::ios_base::app));

	initialize();

	pthread_t threads[NUM_THREADS];

	gettimeofday(&tv_start, NULL);
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&threads[i], NULL, &run_stub, NULL);
	}

	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}
	argo::barrier();
	gettimeofday(&tv_end, NULL);

	WEXEC(fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));

	WEXEC(fexec << "CQ" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);
	WEXEC(fexec.close());

	argo::codelete_(CQ);

	argo::finalize();

	return 0;
}
