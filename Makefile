# Makefile
LDFLAGS=-lncurses -pthread
CFLAGS=-std=gnu99 -g -Wall

all: display_util.o main.o 
	cc -o main main.o display_util.o $(CFLAGS) $(LDFLAGS)

display_util:
	cc -o display_util display_util.o $(CFLAGS) $(LDFLAGS)
