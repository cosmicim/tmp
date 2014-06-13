CC=gcc
CFLAGS=-W -Wall -Wextra -Wpedantic -std=c11 -g

all: server.c shitclient.c test.c
	$(CC) $(CFLAGS) server.c -o bin/server -lpthread
	$(CC) $(CFLAGS) shitclient.c -o bin/shitclient
	$(CC) $(CFLAGS) test.c -o bin/test

server: server.c
	$(CC) $(CFLAGS) server.c -o bin/server -lpthread

shitclient: shitclient.c
	$(CC) $(CFLAGS) shitclient.c -o bin/shitclient

test: test.c
	$(CC) $(CFLAGS) test.c -o bin/test

clean:
	rm -v bin/server
	rm -v bin/shitclient
	rm -v bin/test
