#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "http.h"

#define BUF_SIZE 4096

struct arg_struct {
	int sock_fd;
	char *webroot;
};

void *worker(void *worker_args)
{
	struct arg_struct *wa = worker_args;
	char buf[BUF_SIZE];
	resp_t *resp;
	char *fname;

	pthread_detach(pthread_self());
	memset(buf, 0, BUF_SIZE);
	read(wa->sock_fd, buf, BUF_SIZE - 1);

	fname = parse_request(buf, wa->webroot);
	resp = create_response(fname);
	send_response(wa->sock_fd, resp);
	free(wa);
	printf("Connection Closed\n");
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int sock_fd, newsock_fd, port, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char *webroot;
	DIR *dir;
	pthread_t worker_thread;
	struct arg_struct *wa;

	if (argc != 3) {
		printf
		    ("Server usage is:\n./server [port number] [path to web root]\n");
		return 1;
	}

	port = atoi(argv[1]);
	if (port >= 0 && port < 65536) {
	} else {
		printf("Port %d is not a valid port. Exiting.\n", port);
		return 1;
	}

	webroot = strdup(argv[2]);
	printf("Webroot is %s\n", webroot);
	dir = opendir(webroot);
	if (dir) {
		closedir(dir);
	} else {
		printf
		    ("Path to web root does not exist or is not accesible. Exiting.\n");
		return 1;
	}

	printf("Starting webserver on port %d, in %s\n", port, webroot);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		printf("Error opening socket\n");
		return 1;
	}
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error binding to port\n");
		return 1;
	}
	listen(sock_fd, 10);
	clilen = sizeof(cli_addr);
	printf("Server start success. Listening.\n");

	while (1) {
		newsock_fd =
		    accept(sock_fd, (struct sockaddr *)&cli_addr, &clilen);
		printf("Connection Accepted.\n");
		wa = malloc(sizeof(struct arg_struct));
		wa->sock_fd = newsock_fd;
		wa->webroot = webroot;
		pthread_create(&worker_thread, NULL, worker, wa);
	}

	close(sock_fd);
	return 1;
}
