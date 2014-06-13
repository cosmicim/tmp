#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include "list.h"
#include <sys/select.h>
#include <sys/sendfile.h>

#define NTHREADS 25
#define HOST "0.0.0.0"
#define PORT "1337"
#define NTHREADS 25
#define BUF_SIZE 1024

pthread_t threads[NTHREADS]; 	/* Thread pool */
int thread_connections[NTHREADS];
int num_connections;
int total_threads;
pthread_mutex_t lock;
struct list global_user_list;

struct thread_info {
	int thread_num;
	char *buf;
	int *num_clients;
	struct list *clients;
	fd_set *fds;
	struct timeval *tv;
};

const char *welcome = "WELCOME Velkommen til den beste serveren, %s!\n";

static inline int create_new_thread(void *data);
static void *handle_connection(void *data);
static inline ssize_t send_file(int cfd);

static inline int create_new_thread(void *data)
{
	return pthread_create(&threads[total_threads++], NULL, handle_connection, data);
}

static inline void read_clients(struct thread_info *info, int num)
{
	//struct node *n;
	struct client *c;
	ssize_t nread;
	info->tv->tv_sec = 2;
	info->tv->tv_usec = 0;
	int ret = select(num, info->fds, NULL, NULL, info->tv);
	int i = 0;

	if (ret == -1) {
		return;
	} else if (ret == 0) {
		return;
	}

	list_for_each(c, info->clients->head) {
		if (!FD_ISSET(c->cfd, info->fds))
			return;
		memset(info->buf, 0, BUF_SIZE);	// clear buffer
		nread = recv(c->cfd, info->buf, BUF_SIZE, 0);
		if (nread == 0) {
			printf("Client %d closed the connection (%d).\n", i, c->cfd);
			close(c->cfd);
			list_del_node(info->clients, c);
			--*info->num_clients;
			printf("Number of connections in thread %d: %d\n", info->thread_num, *info->num_clients);

			if (*info->num_clients == 0 && info->thread_num != 1) {
				printf("No more connections in thread %d. Bye-bye!\n", info->thread_num);
				pthread_detach(threads[info->thread_num-1]);
			}
		} else if (nread == -1) {
			perror("reading from client");
			return;
		} else {
			if (!strncmp(info->buf, "/send", 5)) {
				printf("Sent %zd bytes to client.\n", send_file(c->cfd));
			}
			printf("Received %zd bytes from client %d in thread %d: %s\n", nread, i, info->thread_num, info->buf);
			send(c->cfd, info->buf, strlen(info->buf)+1, 0);
		}
		i++;
	}
}

static inline ssize_t send_file(int cfd)
{
	FILE *fp = fopen("./server.c", "rb");
	fseek(fp, 0L, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	int fd = fileno(fp);

	ssize_t nread = sendfile(cfd, fd, 0, (size_t)file_size);
	fclose(fp);
	return nread;
}

static inline void send_welcome(struct client *c, char *buf)
{
	char *usr = malloc(strlen(buf));
	strcpy(usr, buf+8);
	size_t len = strlen(usr)-1;
	while (usr[len] == '\r' || usr[len] == '\n')
		usr[len--] = '\0';

	strcpy(c->username, usr);
	char *tmp = malloc(strlen(welcome)+len+1);
	sprintf(tmp, welcome, usr);
	send(c->cfd, tmp, strlen(tmp)+1, 0);

	printf("tmp: %s\n", tmp);
	free(tmp);
}

static inline void send_user_list(struct client *client)
{
	struct client *c;
	send(client->cfd, "USERS\n", 7, 0);	
	list_for_each(c, global_user_list.head) {
		send(client->cfd, c->username, strlen(c->username)+1, 0);
		send(client->cfd, "\n", 2, 0);
	}
}

static void *handle_connection(void *data)
{
	int num_clients = 0;	// number of clients connected
	char buf[BUF_SIZE];
	struct list clients;
	clients.head = clients.tail = NULL;
	int i;
	struct list *list = (struct list*) data;

	struct thread_info info;
	info.thread_num = total_threads;
	info.buf = buf;
	info.clients = &clients;
	info.num_clients = &num_clients;

	printf("New thread: %d\n", info.thread_num);

	fd_set readfds;
	struct timeval tv;
	info.fds = &readfds;
	info.tv = &tv;
	struct client *c;

	for (;;) {
		if (num_clients) {
			FD_ZERO(&readfds);

			list_for_each(c, clients.head) {
				FD_SET(c->cfd, &readfds);
				i = c->cfd;
			}

			read_clients(&info, i+1);
		}

		if (num_clients == 5 && total_threads == info.thread_num) // full thread
			create_new_thread(list);

		if (num_clients != 5 && list->head != NULL) { // new connection
			int tmp;

			pthread_mutex_lock(&lock);	
			tmp = list->head->cfd;
			list_del(list);
			pthread_mutex_unlock(&lock);

			recv(tmp, buf, BUF_SIZE, 0);
			if (!strncmp(buf, "CONNECT", 7)) {
				c = list_add(&clients, tmp);
				list_add_client(&global_user_list, c);
				send_welcome(c, buf);
				send_user_list(c);
				num_clients++;

				printf("Number of connections in thread %d: %d\n", info.thread_num, num_clients);
			}
		}
	}

	return (void*) 69;
}


int main(int argc, char *argv[])
{
	struct list list;
	list.head = list.tail = NULL;

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, cfd;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	static int yes = 1;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;	// any protocol

	if (getaddrinfo(HOST, PORT, &hints, &result))
		fprintf(stderr, "error in getaddrinfo()\n");

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break; 	/* success! */

		close(sfd);
	}

	if (listen(sfd, 5) == -1)
		fprintf(stderr, "listening\n");

	if (rp == NULL) {
		fprintf(stderr, "Could not connect socket to any address!\n");
		exit(1);
	}

	freeaddrinfo(result);

	pthread_create(&threads[total_threads++], NULL, handle_connection, (void*)&list);
	for (;;) {
		peer_addr_len = sizeof(struct sockaddr_storage);

		cfd = accept(sfd, (struct sockaddr*) &peer_addr, &peer_addr_len);
		printf("Client connected!\n");

		char host[NI_MAXHOST], service[NI_MAXSERV];
		s = getnameinfo((struct sockaddr *) &peer_addr,
				peer_addr_len, host, NI_MAXHOST,
				service, NI_MAXSERV, NI_NUMERICSERV);

		list_add(&list, cfd);

		if (s == 0)
			printf("Got connection from %s:%s\nSocket: %d\n", host, service, cfd);
		else
			fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

	}

	return 0;
}
