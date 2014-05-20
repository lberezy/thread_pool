CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = -lpthread

TARGET = thread_test

DEPS = thread_pool.c
SRCS = $(DEPS) thread_test.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)
	@echo "thread_test has been compiled :-)"
	
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean: 
	rm *.o $(TARGET)