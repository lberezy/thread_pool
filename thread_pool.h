#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <pthread.h>

typedef struct task {
	void (*function)(void *); //pointer to the function the task executes
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

threadpool_t* pool_create(int number_of_workers, int queue_size);

void pool_add_task(threadpool_t *pool, void (*function)(void *), void* arg);

#endif