# Makefile
LDFLAGS=-lncurses -pthread
CFLAGS=-std=gnu99 -g -Wall

all: main.o
	cc -o main main.o $(CFLAGS) $(LDFLAGS)
