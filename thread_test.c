#include <stdio.h>
#include <stdlib.h>
#include "thread_pool.h"

#define THREAD_COUNT 3
#define QUEUE_SIZE 100
#define TASK_COUNT 200

void printnum(void *num) {
	int *number = (int*)num;
	printf("Printing number: %d & addr %p\n", *number, number);
}

int main(int argc, char const *argv[])
{
	//create pool
	threadpool_t *pool = pool_create(THREAD_COUNT,QUEUE_SIZE);

	for (int i=0;i<TASK_COUNT;i++) {
		int* arg = (int*)malloc(sizeof(int));
		*arg = i+1;
		//printf("Loading number : %d & addr %p\n", *arg, arg);
		pool_add_task(pool,printnum,(void*)arg);
		//free(arg);
	}
	return 0;
}