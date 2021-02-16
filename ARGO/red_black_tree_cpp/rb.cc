/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file defines RB tree functions.
*/

#include "rb.h"

// Cohort lock for the whole tree
extern argo::globallock::cohort_lock* lock_1;

void Red_Black_Tree::left_rotation(Node* x) {
	Node* y = x->right;
	recordChange(x);
	x->right = y->left;
	if (y->left) {
		recordChange(y->left);
		y->left->parent = x;
	}

	y->parent = x->parent;

	if (x == getRoot()) {
		changeRoot(y);
	} else if (x == x->parent->left){
		recordChange(x->parent);
		x->parent->left = y;
	} else {
		recordChange(x->parent);
		x->parent->right = y;
	}

	recordChange(y);
	y->left = x;

	recordChange(x);
	x->parent = y;
}

void Red_Black_Tree::right_rotation(Node* x) {
	Node* y = x->left;
	recordChange(x);
	x->left = y->right;
	if (y->right) {
		recordChange(y->right);
		y->right->parent = x;
	}

	y->parent = x->parent;

	if (x == getRoot()) {
		changeRoot(y);
	} else if (x == x->parent->right) {
		recordChange(x->parent);
		x->parent->right = y;
	} else {
		recordChange(x->parent);
		x->parent->left = y;
	}

	recordChange(y);
	y->right = x;
	recordChange(x);
	x->parent = y;
}

void Red_Black_Tree::rb_insert(int val) {
	Node* z = createNode(val);
	Node* y = NULL;
	Node* x = getRoot();

	while (x) {
		y = x;
		if (z->val < x->val) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	z->parent = y;

	if (!y) {
		changeRoot(z);
	} else if (z->val < y->val) {
		recordChange(y);
		y->left = z;
	} else {
		recordChange(y);
		y->right = z;
	}

	recordChange(z);
	z->color = RED;

	insert_fix_up(z);
}

void Red_Black_Tree::rb_insert(Node* z) {
	Node* y = NULL;
	Node* x = getRoot();

	while (x) {
		y = x;
		if (z->val < x->val) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	z->parent = y;

	if (!y) {
		changeRoot(z);
	} else if (z->val < y->val) {
		recordChange(y);
		y->left = z;
	} else {
		recordChange(y);
		y->right = z;
	}

	recordChange(z);
	z->color = RED;

	insert_fix_up(z);
}

void Red_Black_Tree::insert_fix_up(Node* z) {
	Node* y = NULL;
	Node* gp, *p;
	while (z->parent && z->parent->color == RED) {
		p = z->parent;
		gp = p->parent;
		if(!gp) break;
		if (p == gp->left) {
			y = gp->right;
			if (y && y->color == RED) {
				recordChange(p);
				p->color = BLACK;
				recordChange(y);
				y->color = BLACK;
				recordChange(gp);
				gp->color = RED;
				recordChange(z);
				z = gp;
				continue;
			} 
			if (z == p->right) {
				//Left-right case
				Node* tmp;
				left_rotation(p);
				recordChange(z);
				recordChange(p);
				tmp = p;
				p = z;
				z = tmp;
			}
			//left-left case
			recordChange(p);
			recordChange(gp);
			p->color = BLACK;
			gp->color = RED;
			right_rotation(gp);
		} else {
			y = gp->left;
			if (y && y->color == RED) {
				recordChange(p);
				p->color = BLACK;
				recordChange(y);
				y->color = BLACK;
				recordChange(gp);
				gp->color = RED;
				recordChange(z);
				z = gp;
				continue;
			} 
			if (z == p->left) {
				//Right-left case
				Node* tmp;
				right_rotation(p);
				recordChange(z);
				recordChange(p);
				tmp = p;
				p = z;
				z = tmp;
			}
			//Right-right case
			recordChange(p);
			recordChange(gp);
			p->color = BLACK;
			gp->color = RED;
			left_rotation(gp);
		}
	}
	recordChange(getRoot());
	getRoot()->color = BLACK;
}

Node* Red_Black_Tree::rb_search(int val) {
	Node* current = getRoot();
	while (current) {
		if (val == current->val) {
			return current;
		} else if (val < current->val) {
			current = current->left;
		} else {
			current = current->right;
		}
	}
	return current;
}

Node* Red_Black_Tree::successor(Node* x) {
	Node* current = NULL;
	if (x->right) {
		current = x->right;
		while (current->left) {
			current = current->left;
		}
	}
	return current;
}

void Red_Black_Tree::rb_delete(Node* z) {
	Node *x, *y;
	Color color;
	Node* succ;

	if (z->left && z->right) {
		succ = successor(z);

		if (z != getRoot()) {
			recordChange(z->parent);
			if (z->parent->left == z){
				z->parent->left = succ;
			} else {
				z->parent->right = succ;
			}
		} else {
			changeRoot(succ);
		}

		x = succ->right;
		y = succ->parent;
		color = succ->color;

		if (z == y) {
			y = succ;
		} else {
			if (x) {
				recordChange(x);
				x->parent = y;
			} 
			recordChange(y);
			y->left = x;
			recordChange(succ->right);
			succ->right = z->right;
			recordChange(z->right);
			z->right->parent = succ;
		}

		recordChange(succ);
		succ->parent = z->parent;
		succ->color = z->color;
		succ->left = z->left;
		recordChange(z->left);
		z->left->parent = succ;

		if (color == BLACK) {
			delete_fix_up(x, y);
		}
		//delete z
		recordChange(z);
		z->val = 0;
		z->left = NULL;
		z->right = NULL;
		z->parent = NULL;
		//        nvm_free(z);
	} else {
		if (z->left) {
			x = z->left;
		} else {
			x = z->right;
		}
		y = z->parent;
		color = z->color;

		if (x) {
			recordChange(x);
			x->parent = y;
		}

		if (y) {
			recordChange(y);
			if (y->left == z) {
				y->left = x;
			} else {
				y->right = x;
			}
		} else {
			changeRoot(x);
		}

		if (color == BLACK) {
			delete_fix_up(x, y);
		}
		//delete z
		recordChange(z);
		z->val = 0;
		z->left = NULL;
		z->right = NULL;
		z->parent = NULL;
		//        nvm_free(z);
	}
}

void Red_Black_Tree::delete_fix_up(Node* x, Node* y) {
	Node* w;
	while ((!x || x->color == BLACK) && x != getRoot()) {
		if (x == y->left) {
			w = y->right;
			if (w->color == RED) {
				recordChange(w);
				w->color = BLACK;
				recordChange(y);
				y->color = RED;
				left_rotation(y);
				w = y->right;
			}
			else if ((!w->left || w->left->color == BLACK) &&
					(!w->right || w->right->color == BLACK)) {
				recordChange(w);
				w->color = RED;

				if(y->color == RED) {
					recordChange(y);
					y->color = BLACK;
					break;
				}

				x = y;
				y = x->parent;
			} else {
				if (!w->right || w->right->color == BLACK) {
					recordChange(w->left);
					w->left->color = BLACK;
					recordChange(w);
					w->color = RED;
					right_rotation(w);
					w = y->right;
				}
				recordChange(w);
				w->color = y->color;
				recordChange(y);
				y->color = BLACK;
				recordChange(w->right);
				w->right->color = BLACK;
				left_rotation(y);
				x = getRoot();
				break;
			}
		} else {
			w = y->left;
			if (w->color == RED) {
				recordChange(w);
				w->color = BLACK;
				recordChange(y);
				y->color = RED;
				right_rotation(y);
				w = y->left;
			}
			else if ((!w->left || w->left->color == BLACK) &&
					(!w->right || w->right->color == BLACK)) {
				recordChange(w);
				w->color = RED;

				if(y->color == RED) {
					recordChange(y);
					y->color = BLACK;
					break;
				}

				//Y is black, W is black and its children are black
				x = y;
				y = x->parent;
			} else {
				if (!w->left || w->left->color == BLACK) {
					recordChange(w->right);
					w->right->color = BLACK;
					recordChange(w);
					w->color = RED;
					left_rotation(w);
					w = y->left;
				}
				recordChange(w);
				w->color = y->color;
				recordChange(y);
				y->color = BLACK;
				recordChange(w->left);
				w->left->color = BLACK;
				right_rotation(y);
				x = getRoot();
				break;
			}
		}
	}
	if (x) {
		recordChange(x);
		x->color = BLACK;
	}
}

bool Red_Black_Tree::rb_delete_or_insert(int num_updates) {
	lock();

	for(int i = 0; i < num_updates; i++) {
		int val =  rand() % (tree_length);
		Node* toFind = rb_search(val);
		if (toFind) {
			rb_delete(toFind);
		}
		else {
			rb_insert(val);
		}
	}
	unlock();
	return true;
}

Red_Black_Tree::Red_Black_Tree(Node* root) {
	// initialize start_ptr
	start_1 = root;
	// initialize end ptr
	tree_1_end = root_1;
}

void Red_Black_Tree::initialize(Node* root, int* array, unsigned length) {
	// initialize start_ptr
	start_1 = root;

	// initialize end ptr
	tree_1_end = root;

	// build tree_1
	for (unsigned i = 0; i < length; ++i) {
		Node* new_node = tree_1_end;
		*new_node = Node(array[i]);
		tree_1_end++;
		rb_insert(new_node);
	}

	tree_length = length;
	//init mutex
	// lock_1 = argo::new_<argo::globallock::cohort_lock>();
	return;
}

Red_Black_Tree::Red_Black_Tree(Node* root, int* array, unsigned length) {
	// initialize start_ptr
	start_1 = root;

	// initialize end ptr
	tree_1_end = root;

	// build tree_1
	for (unsigned i = 0; i < length; ++i) {
		Node* new_node = tree_1_end;
		new_node->val = array[i];
		tree_1_end++;
		rb_insert(new_node);
	}

	tree_length = length;
	//init mutex
	// lock_1 = argo::new_<argo::globallock::cohort_lock>();
}

Node* Red_Black_Tree::createNode(int _val) {
	Node* new_node_1 = start_1 + _val; 
	assert(!new_node_1->left && !new_node_1->right && !new_node_1->parent); 
	*new_node_1 = Node(_val);
	return new_node_1;
}

Node* Red_Black_Tree::getRoot() {
	return root_1;
}

void Red_Black_Tree::changeRoot(Node* x) {
	recordChange(root_1);
	root_1 = x;
}

void Red_Black_Tree::recordChange(Node* node) {
	return;
}

void Red_Black_Tree::clearChange() {
	changed_nodes.clear();
}

void Red_Black_Tree::copy_changes() {
	return;   
}

void Red_Black_Tree::lock() {
	lock_1->lock();
}

void Red_Black_Tree::unlock() {
	lock_1->unlock();
}
