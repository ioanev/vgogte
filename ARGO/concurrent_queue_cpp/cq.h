/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>
*/

#include "argo.hpp"
#include "../common/wtime.hpp"

#include <vector>
#include <pthread.h>
#include <cstdlib>
#include <pthread.h>
#include <cstdint>

#define NUM_SUB_ITEMS 64 

typedef struct item item;

struct sub_item {
	int val;
	sub_item& operator=(sub_item& other) {
		val = other.val;
		return *this;
	}
};

struct item {
	item* next;
	sub_item si[NUM_SUB_ITEMS];
	item() {
		next = NULL;
	};
	item& operator=(item& other) {
		for (int i= 0; i< NUM_SUB_ITEMS; i++) {
			*(si + i) = *(other.si + i);
		}
		return *this;
	}
};

class concurrent_queue {
	item *head;
	item *tail;
	bool *enq_lock_flag;
	bool *deq_lock_flag;
	argo::globallock::global_tas_lock *enq_lock;
	argo::globallock::global_tas_lock *deq_lock;
	int num_sub_items;

	public:
	concurrent_queue();
	~concurrent_queue();
	void push(int);
	bool pop(int&);
	void init(int n);
};
