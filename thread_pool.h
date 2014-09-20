#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <pthread.h>

/**
 * Struct to hold data for an individual task for a thread pool
 */
typedef struct task {
	void (*function)(void *); //pointer to the function the task executes
	void *arg;
	struct task *next;
} task_t;

/**
 * Struct to hold data for an individual thread pool.
 */
typedef struct pool{
	pthread_mutex_t lock;
	pthread_cond_t notify;
	pthread_t *threads;
	task_t *task_queue; // task queue
	int queue_size;
	int queue_count;
	int thread_count;
	int head;
	int tail;
} threadpool_t;

/**
 * Pool worker removes job tasks from pool queue, executes them and then waits
 * for more work.
 * @param input_parent_pool Pool to which worker belongs. Jobs are taken from here.
 *
 * @return					Function does not return anything meaningful.
 */
void* pool_worker(void* parent_pool);

/**
 * Creates a pool of worker threads for later use
 * @param  number_of_workers [description]
 * @param  queue_size        Maximum lenght of job queue for workers to take
 *                           work from.
 * @return                   Returns a pointer to an initialised threadpool on
 *                           success or NULL on error.
 */
threadpool_t* pool_create(int number_of_workers, int queue_size);

/**
 * Creates a pool of worker threads for later use
 * @param	pool			Threadpool to add task to.
 * @param	function		Function/task for worker to execute.
 * @param	arg				Arguments to function/task.
 * 
 * @return					Returns 0 on success and -1 on error.
 */
int pool_add_task(threadpool_t *pool, void (*function)(void *), void* arg);

#endif
