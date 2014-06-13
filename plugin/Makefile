CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -W -std=c11

all:

libaids: Makefile aids.c
	$(CC) $(CFLAGS) -c -fpic aids.c
	$(CC) -shared -o libaids.so aids.o

test: Makefile test.c aids.c plugin.h
	$(CC) $(CFLAGS) test.c -o test -ldl
