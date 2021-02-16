/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>
*/

#include "argo.hpp"
#include "cohort_lock.hpp"

#include <cstdlib>
#include <assert.h>
#include <pthread.h>

#include <set>
#include <vector>
#include <iostream>

#define TREE_LENGTH 100
#define NUM_UPDATES_PER_CS 64 
#define NUM_OPS 10000
#define NUM_THREADS 4

#define MAX_LEN 16384  // Max total size = 512KB

typedef enum { RED, BLACK } Color;

struct Node {
	int val;
	Color color;
	Node *left, *right, *parent;
	char padding [64 - sizeof(int) - sizeof(Color) - 3 * sizeof(Node*)+64];
	Node() {}
	Node(int _val) {
		val = _val;
		left = NULL;
		right = NULL;
		parent = NULL;
		color = BLACK;
	}
};

// Red Black Tree algorithm from:
//      http://www.cnblogs.com/skywang12345/p/3624291.html
//      This blog uses the algorithm from "Introduction to Algorithms".

class Red_Black_Tree {
	// roots of the two trees
	Node* root_1;

	// the current end ptrs of the two trees
	Node* tree_1_end;

	Node* start_1;
	// c++ 98 does not support unordered_set (O(1) insertion/deletion), 
	// so we use "std::vector". Although that may cause duplicated operations 
	// on the same node, the total complexity remains O(log(n)). However, using 
	// "std::set" will increase it to O((log(n))^2).
	// FIXME: there might be redundant changes of node recorded. 
	std::vector<Node*> changed_nodes;

	// mutex for the whole tree
	// note: this program has NO parallelism. Using mutex only creates a 
	// thread-safe tree
	// argo::globallock::cohort_lock* lock_1;

	int tree_length;


	// helper functions
	void changeRoot(Node* x);
	Node* getRoot();
	void clearChange();
	Node* createNode(int _val);
	// Only record changes in the current tree
	void recordChange(Node* node);
	// Apply the changes in the current tree to the backup tree and change 
	// the sel bit
	void copy_changes();

	// lock the whole current tree
	void lock();
	// unlock the whole current tree
	void unlock();

	// tree operations
	void left_rotation(Node* x);
	void right_rotation(Node* x);
	void insert_fix_up(Node* z);
	void delete_fix_up(Node* x, Node* y);
	Node* successor(Node* x);
	// If val is in the tree, return the ptr to the node, otherwise return NULL
	Node* rb_search(int val);
	// Create a new node at the end_ptr and increment the end_ptr;
	// note: insertion will not reuse the deleted memory.
	void rb_insert(int val);
	void rb_insert(Node* x);
	// delete node z
	// note: Deletion will not actually free the memory occupied by z
	// It will only set all pointers in z to be NULL and set the value
	// to be 0.
	void rb_delete(Node* z);

	public:
	Red_Black_Tree() {}
	Red_Black_Tree(Node* root);
	Red_Black_Tree(Node* root, int* array, unsigned length);
	~Red_Black_Tree() {}
	void initialize(Node* root, int* array, unsigned length);
	// return false if did an insertion
	// return true if did a deletion
	// note: only rb_delete_or_insert is persistent and thread-safe!
	bool rb_delete_or_insert(int num_updates);
};
