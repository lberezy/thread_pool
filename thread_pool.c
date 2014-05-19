#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thread_pool.h"

threadpool_t* pool_create(int number_of_workers, int queue_size) {
	// allocate new pool
	threadpool_t *pool = (threadpool_t*)malloc(sizeof(threadpool_t)); 

	pool->queue_size = queue_size;
	pool->queue_count = 0;
	pool->thread_count = number_of_workers;
	pool->head = 0;
	pool->tail = pool->head;

	// allocate thread array
	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * pool->thread_count);
	//allocate task queue
	pool->task_queue = (task_t*)malloc(sizeof(task_t) * pool->queue_size);

	// initialise mutexes
	
	pthread_mutex_init(&(pool->lock), NULL); // use implementation default attr.
	pthread_cond_init(&(pool->notify), NULL);
	
	// TODO - error checking on above.
	
	for (int i = 0; i < (pool->thread_count); i++) {
		if ( pthread_create(&(pool->threads[i]), NULL, &pool_worker, (void*)pool) ) {
			perror("Error creating thread, exiting.\n");
			exit(EXIT_FAILURE);
			}
	}

	return pool;
}

void pool_add_task(threadpool_t *pool, void (*function)(void *), void* arg) {

	// set up task
	task_t task;
	task.function = function;
	task.arg = arg;

	pthread_mutex_lock(&(pool->lock)); // enter critical section

	if (pool->queue_count > pool->queue_size) { // TODO: Handle error bettter
		perror("Queue is full, try again!\n");
		return;
	}
	pool->task_queue[pool->tail] = task; // add task to end of queue

	pool->tail = (pool->tail++) % pool->queue_size; // advance end of queue
	pool->queue_count++; // job added to queue

	// notify waiting workers of new job
	pthread_cond_signal(&(pool->notify));

	pthread_mutex_unlock(&(pool->lock)); // end critical section

}

// pool worker function
void* pool_worker(void* input_parent_pool) {

	threadpool_t *parent_pool = (threadpool_t*)input_parent_pool;
	task_t task;

	while(1) {
		// take lock. thread is blocked if not possible to take, that's fine.
		// need the lock in order to wait on condition. don't want condition
		// being changed.
		pthread_mutex_lock(&(parent_pool->lock));
		/* begin critical section */

		// wait for notification of new work when pool is empty
		while(parent_pool->queue_count == 0) {
			// thread is blocked while waiting for a notification of work.
			// does this with an atomic release of lock so other workers can
			// also be waiting. lock is retained upon notification
			// no more busy waiting!
			pthread_cond_wait(&(parent_pool->notify), &(parent_pool->lock));
		}

		// otherwise grab the next task in the queue and run it
		task.function = parent_pool->task_queue[parent_pool->head].function;
        task.arg = parent_pool->task_queue[parent_pool->head].arg;

        // increment head of queue
        parent_pool->head = (parent_pool->head++) % parent_pool->queue_size;
        parent_pool->queue_count--; // removed a task from queue

        /* end critical section */
        pthread_mutex_unlock(&(parent_pool->lock));

        // execute task
        (*task.function)(task.arg);

	}
	return NULL;
}