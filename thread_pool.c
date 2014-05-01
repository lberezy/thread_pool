#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct task {
	void (*function)(void *);
    void *arg;
	struct task *next;
} task_t;


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


void* pool_worker(void* parent_pool);
threadpool_t* pool_create(int workers, int queue_size);

threadpool_t* pool_create(int workers, int queue_size) {

	threadpool_t *pool = malloc(sizeof(threadpool_t)); // allocate new pool

	pool->queue_size = queue_size;
	pool->queue_count = 0;
	pool->thread_count = 0;
	pool->head = 0;
	pool->tail = pool->head;

	// allocate thread array
	pool->threads = malloc(sizeof(pthread_t) * pool->thread_count);
	//allocate task queue
	pool->tasks = malloc(sizeof(task_t) * pool->queue_size);

	// initialise mutexes
	
	pthread_mutex_init(&(pool->lock), NULL); // use implementation default attr.
	pthread_cond_init(&(pool->notify), NULL);

	//NOTE: Do error checking on above.
	//
	
	for (int i = 0; i < (pool->thread_count); i++) {
		if (pthread_create(&(pool->threads[i]), NULL, &pool_worker, (void*)pool)) {
			perror("Error creating thread, exiting.\n");
			exit(EXIT_FAILURE);
			}
		pool->thread_count++;
	}

	return pool;
}

void pool_add_task(threadpool_t *pool, void (*function)(void *), void* arg) {

	task_t *task = malloc(sizeof(task));
	task->function = function;
	task->arg = arg;

	pthread_mutex_lock(&(pool->lock)); //
}

// pool worker function
void* pool_worker(void* parent_pool) {

	threadpool_t *parent_pool = parent_pool;
	task_t task;

	while(1) {
		//take lock. thread is blocked if not possible to take, that's fine.
		// need the lock in order to wait on condition. don't want condition
		// being changed.
		pthread_mutex_lock(&(parent_pool->lock));
		/* begin critical section */

		// wait for notification of new work when pool is empty
		while(pool->queue_count == 0) {
			// thread is blocked while waiting for a notification of work.
			// does this with an atomic release of lock so other workers can
			// also be waiting. lock is retained upon notification
			// no more busy waiting!
			pthread_cond_wait(&(parent_pool->notify), &(parent_pool->lock));
		}

		// otherwise grab the next task in the queue and run it
		task.function = parent_pool->queue[parent_pool->head].function;
        task.arg = parent_pool->queue[parent_pool->head].arg;

        // increment head of queue
        parent_pool->head = (parent_pool->head++) % parent_pool->queue_size;
        parentpool->count--; // removed a task from queue

        /* end critical section */
        pthread_mutex_unlock(&(parent_pool->lock));

        // execute task
        *(task.function)(task.arg);

	}
	return NULL;
}