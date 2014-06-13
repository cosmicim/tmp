#define _GNU_SOURCE // getopt_long
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#define CONNECT "CONNECT " // in use
#define LOGOUT "LOGOUT" // in use
#define MESSAGE "MESSAGE " // in use
#define NAME "NAME "
#define BUF_SIZE 1024

int get_msg(char *buf, size_t size);
int get_name(char *buf, size_t size);

static void print_usage()
{
	fprintf(stderr, "Usage: shitclient\n\t-s server-ip\n\t");
	fprintf(stderr, "-p port\n\t-u username\n\t-m message\n");
	exit(0);
}

void srv_connect(char *ipAddr, int portNum, char *usr, char *msg)
{
        /* Declarations */
        struct sockaddr_in serveraddr;
        int sd, ret;
        char conMessage[BUF_SIZE];
        char message[BUF_SIZE];
        char response[BUF_SIZE];

        /* Create socket */
        sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        /* Clear address structure */
        memset(&serveraddr, 0, sizeof(struct sockaddr_in));

        /* Add address family */
        serveraddr.sin_family = AF_INET;

        /* Add IP-address */
        inet_pton(AF_INET, ipAddr, &serveraddr.sin_addr);

        /* Add port number */
        serveraddr.sin_port = htons(portNum);

        /* Connect */
        ret = connect(sd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("connect");
	}

	if (usr == NULL) {
		printf("%d\n", get_name(conMessage, BUF_SIZE));
	} else {
		strcpy(conMessage, "CONNECT ");
		strcat(conMessage, usr);
	}

	//printf("%s\n", conMessage);
	write(sd, conMessage, strlen(conMessage)+1);
	read(sd, response, BUF_SIZE);
	printf("%s\n", response);

	//printf("%d\n", get_msg(message, BUF_SIZE));
	strcpy(message, msg);

        while (strcmp(message, "MESSAGE /q")) {
                /* Send data */
                //ret = write(sd, message, strlen(message)+1);
		ret = send(sd, message, strlen(message)+1, 0);
		if (ret == -1)
			perror("send");

		memset(response, 0, BUF_SIZE);

                /* Read data */
                //ret = read(sd, response, BUF_SIZE);
		ret = recv(sd, response, BUF_SIZE, 0);
		if (ret == -1)
			perror("recv");

                printf("%s\n", response);

		get_msg(message, BUF_SIZE);
        }

        write(sd, LOGOUT, 7);

        /* Close socket */
        close(sd);
}

/*
 * Get user input.
 */
char *get_input(char *buf, size_t size, const char *msg, const char *pre)
{
	int offset = strlen(pre);
	memset(buf, 0, size);
	strcat(buf, pre);

	printf("%s", msg);
	fgets(buf+offset, size-offset, stdin);
	return buf;
}

/*
 * Get username.
 * Returns number of characters read from user.
 */
int get_name(char *buf, size_t size)
{
	get_input(buf, size, "Your name, pls:\n", CONNECT);
	return strlen(buf);
}

/*
 * Get message from user.
 * Returns number of characters read from user.
 */
int get_msg(char *buf, size_t size)
{
	get_input(buf, size, "Your message, pls:\n", MESSAGE);
	return strlen(buf);
}

int main(int argc, char *argv[])
{
	int opt;
	char *username = NULL;
	char *msg = NULL;
	char *host = NULL;
	int port = 0;
	int index = 0;

        if (argc < 3) {
                exit(1);
        }

	struct option long_opts[] = {
		{ "username", 	required_argument, 0, 'u' },
		{ "message", 	required_argument, 0, 'm' },
		{ "server", 	required_argument, 0, 's' },
		{ "port", 	required_argument, 0, 'p' },
		{ 0,		0,		   0,  0  }
	};

	while ((opt = getopt_long(argc, argv, "u:m:s:p:", long_opts, &index)) != -1) {
		switch (opt) {
		case 'u':
			username = malloc(strlen(optarg)+1);
			strcpy(username, optarg);
			break;
		case 'm':
			msg = malloc(strlen(optarg)+1);
			strcpy(msg, optarg);
			break;
		case 's':
			host = malloc(strlen(optarg)+1);
			strcpy(host, optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			print_usage();
		}
	}

	/*if (username != NULL)
		printf("%s\n", username);
	if (msg != NULL)
		printf("%s\n", msg);*/

	if (host == NULL)
		print_usage();

        srv_connect(host, port, username, msg);

        return 0;
}
