#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

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

#endif