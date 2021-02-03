/*
 Author: Vaibhav Gogte <vgogte@umich.edu>
         Aasheesh Kolli <akolli@umich.edu>

*/


#include "cq.h"
#include <iostream>
/******************************************
 * tail-> item2 <- item1 <- item0 <- head *
 *        next<-item               *
 *     push > tail...head > pop      *
 ******************************************/



void concurrent_queue::push(int val) {
  item *new_item = (item*)malloc(sizeof(item));

  pthread_mutex_lock(&enq_lock);
  for (int i = 0; i < num_sub_items; i++) {
    (new_item->si + i)->val = val;
  }
  new_item->next = NULL;
  tail->next = new_item;
  tail = new_item;
  pthread_mutex_unlock(&enq_lock);
}

bool concurrent_queue::pop(int &out) {
  pthread_mutex_lock(&deq_lock);
  item *node = head;
  item *new_head = node->next;
  if (new_head == NULL) {
    pthread_mutex_unlock(&deq_lock);
    return false;
  }
  out = (new_head->si)->val;
  
  head = new_head;
  pthread_mutex_unlock(&deq_lock);
  
  free(node);
  return true;
}


void concurrent_queue::init(int n) {

  num_sub_items = n;
  item *new_item = (item *)malloc(sizeof(item));
  for (int i = 0; i < num_sub_items; i++) {
    (new_item->si + i)->val = -1;
  }
  new_item->next = NULL;
  head = new_item;
  tail = new_item;
}


concurrent_queue::concurrent_queue() {
  head = NULL; 
  tail = NULL;
  q_size = 0;
  
  //initialize mutex and cv
  pthread_mutex_init(&enq_lock,NULL);
  pthread_mutex_init(&deq_lock,NULL);
}

concurrent_queue::~concurrent_queue() {
  int temp;
  while(pop(temp));
}

