#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include "stringlist.h"

struct sockaddr_un sizecheck;
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(sizecheck.sun_path)
#endif

LISTNODEPTR messages = NULL;

int connection_handler(int connection_fd, size_t maxsize) {
	int nbytes;
	char *buffer = calloc(maxsize, sizeof(char));
	char *response = "thanks for your message";

	nbytes = read(connection_fd, buffer, maxsize);
	buffer[nbytes] = 0;
	stringlist_insert(&messages, buffer);

	write(connection_fd, response, strlen(response));

	close(connection_fd);
	return 0;
}

struct options {
	char *filename;
	int n_messages;
	int message_length;
};

struct options* parse_options(int argc, char **argv) {
	int c=0;
	struct options *opts = malloc(sizeof(struct options));
	opts->n_messages = 3;
	opts->message_length = 256;
	while(c!=-1) {
		struct option long_options[] = {
			{"path", required_argument, 0,  'p' },
			{"messages", required_argument, 0,  'm' },
			{"message-length", required_argument, 0,  'l' },
			{0,0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "p:m:l:", long_options, &option_index);
		switch(c) {
			case 0:
				break;
			case 'p':
				opts->filename = calloc(strlen(optarg), sizeof(char));
				strcpy(opts->filename, optarg);
				break;
			case 'm':
				sscanf(optarg, "%d", &opts->n_messages);
				break;
			case 'l':
				sscanf(optarg, "%d", &opts->message_length);
				break;
			case '?':
				printf("Error '?' on option: %c\n", c);
				exit(2);
				break;
			case -1:
				break;
			default:
				printf("Error 'default' on option: %c", c);
				exit(2);
				break;
		}
	}
	if(opts->filename == NULL) {
		fprintf(stderr, "Filename not supplied\n");
		exit(2);
	}
	return opts;
}

int accumulate(struct options* options) {
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;
	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	} 

	unlink(options->filename);

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, options->filename);

	if(bind(socket_fd, 
				(struct sockaddr *) &address, 
				sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind() failed\n");
		return 1;
	}

	if(listen(socket_fd, 5) != 0)
	{
		printf("listen() failed\n");
		return 1;
	}

	while(options->n_messages-- > 0 &&
		(connection_fd = accept(socket_fd,(struct sockaddr *) &address, &address_length)
		) > -1 )
	{
		connection_handler(connection_fd, options->message_length);
	}
	unlink(options->filename);
}

void combine(struct options* options) {
	int tochild[2];
	int pid;
	if(pipe(tochild)<0) {
		perror("Error making pipe");
		exit(1);
	}
	switch( pid = fork() ) {
		case -1:
			perror("Can't fork");
			exit(1);
		case 0: /* Child */
			close(0); /* Close current stdin. */
			dup( tochild[0]); /* Make stdin come from read
					end of pipe. */
			close(tochild[1]);
			
			dup2(1,2); /* redirect stderr to stdout */
			close(1);
			execl("/usr/bin/ssss-combine", "ssss-combine", "-Qxt",
					"3"/* TODO: parametrize on n_messages */);
			perror("Problem with combine");
			exit(126);
		default: /* Parent */
			for(;messages != NULL; messages=messages->nextPtr) {
				write(tochild[1], messages->data, strlen(messages->data));
				write(tochild[1], "\n", 1);

				write(2, messages->data, strlen(messages->data));
				write(2, "\n", 1);
			}
			close(tochild[1]);
			exit(0);
	}
}

int main(int argc, char **argv) {
	/*TODO: options
	 * socket path
	 * how many keys to wait for
	 * message size
	 */
	struct options *options;

	if(mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
		fprintf(stderr, "Memory locking failed: %d (", errno);
		if(errno == ENOMEM)
			fprintf(stderr, "ENOMEM");
		else if(errno == EAGAIN)
			fprintf(stderr, "EAGAIN");
		else if(errno == EINVAL)
			fprintf(stderr, "EINVAL");
		else if(errno == EPERM)
			fprintf(stderr, "EPERM");
		fprintf(stderr, ")\n");
		exit(1);
	}
	options	= parse_options(argc, argv);

	/*READ FROM A DOMAIN SOCKET AND ACCUMULATE MESSAGES*/
	if(accumulate(options))
		fprintf(stderr, "Errors reading from socket\n");

	combine(options);
}

