/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>
*/ 

#include "rb.h"

#include <cstdint>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#include <vector>
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

// Cohort lock for the whole tree
argo::globallock::cohort_lock* lock_1;

int* array;
Red_Black_Tree* RB;

void initialize() {
	RB = argo::conew_<Red_Black_Tree>();
}

void* run_stub(void* ptr) {
	for (int i = 0; i < NUM_OPS/(NUM_THREADS*numtasks); ++i) {
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
	argo::init(256*1024*1024UL, 256*1024*1024UL);

	workrank = argo::node_id();
	numtasks = argo::number_of_nodes();

	lock_1 = new argo::globallock::cohort_lock();

	MAIN_PROC(workrank, std::cout << "In main\n" << std::endl);
	struct timeval tv_start;
	struct timeval tv_end;

	std::ofstream fexec;
	MAIN_PROC(workrank, fexec.open("exec.csv",std::ios_base::app));

	// This contains the Atlas restart code to find any reusable data
	initialize();

	MAIN_PROC(workrank, array = new int[TREE_LENGTH]);
	MAIN_PROC(workrank, for (int i = 0; i < TREE_LENGTH; ++i) {
			array[i] = i;
	});

	Node* root = argo::conew_array<Node>(2 * MAX_LEN);
	MAIN_PROC(workrank, RB->initialize(root, array, TREE_LENGTH));
	MAIN_PROC(workrank, if(DEBUG_RBT) { printf("|Init: RBTree|\n"); RB->rb_print(); });
	argo::barrier();
	MAIN_PROC(workrank, std::cout << "Done with RBtree creation" << std::endl);

	pthread_t T[NUM_THREADS];

	gettimeofday(&tv_start, NULL);
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_create(&T[i], NULL, &run_stub, NULL);
	}
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(T[i], NULL);
	}
	argo::barrier();
	gettimeofday(&tv_end, NULL);
	MAIN_PROC(workrank, if(DEBUG_RBT) { printf("|Exec: RBTree|\n"); RB->rb_print(); });

	MAIN_PROC(workrank, fprintf(stderr, "time elapsed %ld us\n",
				tv_end.tv_usec - tv_start.tv_usec +
				(tv_end.tv_sec - tv_start.tv_sec) * 1000000));
	MAIN_PROC(workrank, fexec << "RB" << ", " << std::to_string((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec) * 1000000) << std::endl);
	MAIN_PROC(workrank, fexec.close());

	MAIN_PROC(workrank, std::cout << "Done with RBtree" << std::endl);

	delete lock_1;
	MAIN_PROC(workrank, delete[] array);
	argo::codelete_array(root);
	argo::codelete_(RB);

	MAIN_PROC(workrank, std::cout << "Finished!" << std::endl);

	argo::finalize();

	return 0;
}
