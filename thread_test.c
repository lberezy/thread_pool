#include <stdio.h>
#include <stdlib.h>
#include "thread_pool.h"

void printnum(void *num) {
	int *number = (int*)num;
	printf("Number: %d\n", *number);
}

int main(int argc, char const *argv[])
{
	//create pool of 10 workers with 100 queue space
	threadpool_t *pool = pool_create(10,100);
	for (int i=0;i<100;i++) {
		int* arg = (int*)malloc(sizeof(int));
		*arg = i+1;
		pool_add_task(pool,printnum,(void*)arg);
	}
	return 0;
}