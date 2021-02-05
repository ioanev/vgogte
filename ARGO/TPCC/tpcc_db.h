/*
Author: Vaibhav Gogte <vgogte@umich.edu>
Aasheesh Kolli <akolli@umich.edu>

ArgoDSM/PThreads version:
Ioannis Anevlavis <ioannis.anevlavis@etascale.com>

This file declares the tpcc database and the accesor transactions.
*/

#include "argo.hpp"

#include "table_entries.h"
#include <atomic>
#include "simple_queue.h"
#include <pthread.h>
#include <cstdlib>

typedef simple_queue queue_t;


class TPCC_DB {

	private:
		// For the deallocation of the locks
		short num_locks;
		// Tables with size dependent on num warehouses
		short num_warehouses;
		short random_3000[3000];
		warehouse_entry* warehouse; 
		district_entry* district; 
		customer_entry* customer; 
		stock_entry* stock;

		// Tables with slight variation in sizes (due to inserts/deletes etc.)
		history_entry* history;
		order_entry* order; 
		new_order_entry* new_order; 
		order_line_entry* order_line; 

		// Fixed size table
		item_entry* item;

		unsigned long* rndm_seeds;

		queue_t* perTxLocks; // Array of queues of locks held by active Tx
		bool* lock_flags; // Array of flags for the locks
		argo::globallock::global_tas_lock** locks; // Array of locks held by the TxEngn. RDSs acquire locks through the TxEngn

	public:

		TPCC_DB();
		~TPCC_DB();

		void initialize(int _num_warehouses, int numThreads, int numLocks);
		void populate_tables();
		void fill_item_entry(int _i_id);
		void fill_warehouse_entry(int _w_id);
		void fill_stock_entry(int _s_w_id, int s_i_id);
		void fill_district_entry(int _d_w_id, int _d_id);
		void fill_customer_entry(int _c_w_id, int _c_d_id, int _c_id);
		void fill_history_entry(int _h_c_w_id, int _h_c_d_id, int _h_c_id);
		void fill_order_entry(int _o_w_id, int _o_d_id, int _o_id);
		void fill_order_line_entry(int _ol_w_id, int _ol_d_id, int _ol_o_id, int _o_ol_cnt, long long _o_entry_d);
		void fill_new_order_entry(int _no_w_id, int _no_d_id, int _no_o_id);

		void random_a_string(int min, int max, char* string_ptr);
		void random_n_string(int min, int max, char* string_ptr);
		void random_a_original_string(int min, int max, int probability, char* string_ptr);
		void random_zip(char* string_ptr);
		void fill_time(long long &time_slot);
		int rand_local(int min, int max);

		void new_order_tx(int threadId, int w_id, int d_id, int c_id);

		void copy_district_info(district_entry &dest, district_entry &source);
		void copy_customer_info(customer_entry &dest, customer_entry &source);
		void copy_new_order_info(new_order_entry &dest, new_order_entry &source);
		void copy_order_info(order_entry &dest, order_entry &source);
		void copy_stock_info(stock_entry &dest, stock_entry &source);
		void copy_order_line_info(order_line_entry &dest, order_line_entry &source);

		void update_order_entry(int _w_id, short _d_id, int _o_id, int _c_id, int _ol_cnt);
		void update_stock_entry(int threadId, int _w_id, int _i_id, int _d_id, float &amount);

		unsigned long get_random(int thread_id, int min, int max);
		unsigned long get_random(int thread_id);

		void printStackPointer(int* sp, int thread_id);

		void acquire_locks(int thread_id, queue_t &reqLocks);
		void release_locks(int thread_id);

};
