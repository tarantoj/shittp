#ifndef HTTP_H_
#define HTTP_H_

// Response struct with header, content file descriptor and content length
struct response;
typedef struct response {
	char header[1024];
	int content_fd;
	int content_len;
} resp_t;

// Helper function, returns mime-type of given file name
char *mime_type(char *fname);

// Create response struct from file
resp_t *create_response(char *fname);

// Parse a HTTP request into its relative filepath
char *parse_request(char *request, char *webroot);

// Send header and file to socket, close file and socket when done
int send_response(int sockfd, resp_t * resp);

#endif				/* HTTP_H_ */
