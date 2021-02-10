#ifndef WTIME_HPP
#define WTIME_HPP

#include "argo.hpp"

#include <omp.h>
#include <pthread.h>
#include <sys/time.h>

extern int workrank;
extern int numtasks;

static
void wtime(double *t) {
	static int sec = -1;
	struct timeval tv;
	gettimeofday(&tv, 0);
	if (sec < 0) sec = tv.tv_sec;
	*t = (tv.tv_sec - sec) + 1.0e-6*tv.tv_usec;
}

typedef struct {
	int locks{0};
	int barrs{0};
	/////
	double locktime{0.0};
	double barrtime{0.0};
	/////
	double lock_t1{0.0};
	double lock_t2{0.0};
	/////
	double barr_t1{0.0};
	double barr_t2{0.0};
	/////
	pthread_mutex_t stat_mx = PTHREAD_MUTEX_INITIALIZER;
} lock_barr_t;

extern lock_barr_t argo_stats;

/**
 * @note: this will only work if the application is written for
 * only one thread per node to capture the global lock.
 */
static inline __attribute__((always_inline))
void argo_lock(argo::globallock::global_tas_lock *lock) {
	wtime(&argo_stats.lock_t1); // race
	lock->lock();
}

static inline __attribute__((always_inline))
void argo_unlock(argo::globallock::global_tas_lock *lock) {
	lock->unlock();
	wtime(&argo_stats.lock_t2); // race
	argo_stats.locktime += argo_stats.lock_t2 - argo_stats.lock_t1; // race
  	pthread_mutex_lock(&argo_stats.stat_mx);
	argo_stats.locks++; // ok
  	pthread_mutex_unlock(&argo_stats.stat_mx);
}

/**
 * @note: we need to overload this function and not supply a
 * default argument to make sure that it is inlined.
 */ 
static inline __attribute__((always_inline))
void argo_barrier() {
	wtime(&argo_stats.barr_t1);
	argo::barrier();
	wtime(&argo_stats.barr_t2);
	argo_stats.barrtime += argo_stats.barr_t2 - argo_stats.barr_t1;
	argo_stats.barrs++;
}

static
void print_argo_stats() {
	bool *lock_flag = argo::conew_<bool>(false);
	int *g_locks = argo::conew_array<int>(numtasks);
	argo::globallock::global_tas_lock *lock = new argo::globallock::global_tas_lock(lock_flag);

	g_locks[workrank] = argo_stats.locks;
	argo::barrier();

	if (workrank == 0)
	{
		for (int i = 0; i < numtasks; ++i)
			if (i != workrank)
				g_locks[workrank] += g_locks[i];

		printf("#####################STATISTICS#########################\n");
		printf("Argo locks : %d, barriers : %d\n",
			g_locks[workrank], argo_stats.barrs);
		printf("Argo locktime : %.3lf sec., barriertime : %.3lf sec.\n",
			argo_stats.locktime, argo_stats.barrtime);
		printf("########################################################\n\n");
	}

	delete lock;
	argo::codelete_(lock_flag);
	argo::codelete_array(g_locks);
}

#endif /* WTIME_HPP */
