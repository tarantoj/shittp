#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>

// Response struct with header, content file descriptor and content length
struct response;
typedef struct response {
	char header[1024];
	int content_fd;
	int content_len;
} resp_t;

// Helper function, returns mime-type of given file name
char *mime_type(char *fname)
{
	char *ext;
	ext = strrchr(fname, '.') + 1;

	if (!ext) {
		return "application/octet-stream";
	} else if (strcmp(ext, "html") == 0) {
		return "text/html";
	} else if (strcmp(ext, "jpg") == 0) {
		return "image/jpeg";
	} else if (strcmp(ext, "css") == 0) {
		return "text/css";
	} else if (strcmp(ext, "js") == 0) {
		return "application/javascript";
	} else {
		return "application/octet-stream";
	}
}

// Create response struct from file
resp_t *create_response(char *fname)
{
	resp_t *resp;
	struct stat stat_buf;

	resp = malloc(sizeof(resp_t));

	memset(resp->header, 0, sizeof(resp->header));
	resp->content_fd = open(fname, O_RDONLY);
	resp->content_len = 0;

	if (resp->content_fd != -1) {
		fstat(resp->content_fd, &stat_buf);
		resp->content_len = stat_buf.st_size;
		snprintf(resp->header, 1024,
			 "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Size: %d\r\n\r\n",
			 mime_type(fname), resp->content_len);
	} else if (!fname) {
		strcpy(resp->header, "HTTP/1.0 400 Bad Request\r\n\r\n");
	} else {
		strcpy(resp->header, "HTTP/1.0 404 Not Found\r\n\r\n");
	}

	free(fname);

	return resp;
}

// Parse a HTTP request into its filepath
char *parse_request(char *request, char *webroot)
{
	char *request_array[3];
	char *p = strtok(request, " ");
	int i = 0;
	char *fname;

	while (p != NULL && i < 3) {
		request_array[i++] = p;
		p = strtok(NULL, " ");
	}

	if (strcmp(request_array[0], "GET") != 0) {
		return NULL;
	}

	fname = malloc(strlen(webroot) + strlen(request_array[1]) + 1);
	strcpy(fname, webroot);
	strcat(fname, request_array[1]);
	printf("Requested file is: %s\n", fname);

	return fname;
}

// TODO SIGPIPE Handling
// Send header and file to socket, close file and socket when done
int send_response(int sock_fd, resp_t * resp)
{
	int len;
	char *offset;

	len = strlen(resp->header);
	offset = resp->header;

	while (len > 0) {
		int i = send(sock_fd, offset, len, 0);
		len -= i;
		offset += i;
	}
	len = resp->content_len;
	while (len > 0) {
		len -= sendfile(sock_fd, resp->content_fd, NULL, len);
	}

	close(resp->content_fd);
	close(sock_fd);
	free(resp);

	return 1;
}
