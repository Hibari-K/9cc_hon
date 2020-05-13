CC=gcc
CFLAGS=-std=gnu11 -g -static
TARGET=9cc
SRCS=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

test: $(TARGET)
	./test.sh

clean:
	rm -f $(OBJS) $(TARGET) tmp*

.PHONY:	test clean
