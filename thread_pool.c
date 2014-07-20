#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thread_pool.h"

threadpool_t* pool_create(int number_of_workers, int queue_size) {

	int ret_val;
	// allocate new pool
	threadpool_t *pool = (threadpool_t*)malloc(sizeof(threadpool_t)); 

	// Pool settings
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
	if ((ret_val = pthread_mutex_init(&(pool->lock), NULL)) != 0) {
		fprintf(stderr, "Could not initialise mutex: %s\n", strerror(ret_val));
		goto error;
	}
	if ((ret_val = pthread_cond_init(&(pool->notify), NULL)) != 0) {
		fprintf(stderr, "Could not initialise conditional mutex: %s\n", strerror(ret_val));
		goto error;
	}
	// instantiate worker threads
	for (int i = 0; i < (pool->thread_count); i++) {
		if ((ret_val = pthread_create(&(pool->threads[i]), NULL, &pool_worker, (void*)pool)) != 0) {
			fprintf(stderr, "Could not allocate worker %d: %s\n", i, strerror(ret_val));
			goto error;
			perror("Error creating thread, exiting.\n");
		}
	}

	return pool;

	error:
		fprintf(stderr, "Failed to create threadpool\n");
		fflush(stderr);
		free(pool->threads);
		free(pool->task_queue);
		free(pool);
		return NULL;
}

int pool_add_task(threadpool_t *pool, void (*function)(void *), void* arg) {

	// set up task
	task_t task;
	task.function = function;
	task.arg = arg; //

	pthread_mutex_lock(&(pool->lock)); // enter critical section

#ifdef DEBUG
	//printf("--->Adding task to pool: function & %p, arg & %p\n", function, arg);
#endif

	if (pool->queue_count > pool->queue_size) {
		fprintf(stderr, "Could not add task. Queue full.\n");
		fflush(stderr);
		pthread_mutex_unlock(&(pool->lock)); // release lock
		return -1;
	}

	pool->task_queue[pool->tail] = task; // add task to end of queue
	pool->tail = (pool->tail + 1) % pool->queue_size; // advance end of queue
	pool->queue_count++; // job added to queue

#ifdef DEBUG
	printf("--->Queue at %d / %d, queue tail @ %d\n",pool->queue_count,pool->queue_size,pool->tail);
#endif

	pthread_cond_signal(&(pool->notify)); // notify waiting workers of new job
	pthread_mutex_unlock(&(pool->lock)); // end critical section

	return 0;
}

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

		// grab the next task in the queue and run it
		task.function = parent_pool->task_queue[parent_pool->head].function;
        task.arg = parent_pool->task_queue[parent_pool->head].arg;

        // increment head of queue
        parent_pool->head = (parent_pool->head+1) % parent_pool->queue_size;
        parent_pool->queue_count--; // removed a task from queue

        /* end critical section */
        pthread_mutex_unlock(&(parent_pool->lock));

        // execute task
        (*task.function)(task.arg);

        //TODO - need to free argument pointer...maybe

	}
	return NULL;
}
