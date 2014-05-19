CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = -lpthread

thread_test:
	$(CC) $(CFLAGS) $(LDFLAGS) thread_test.c thread_pool.c -o thread_test

clean: 
	rm -f *.o thread_test