CC=gcc
CFLAGS=-std=c11 -g -static
TARGET=9cc
SRC=9cc.c
OBJ:=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)

test:
	./test.sh

clean:
	rm -f $(OBJ) $(TARGET) tmp*

.PHONY:	test clean
