#ifndef HAVE_NETWORK_H
#define HAVE_NETWORK_H 1

#include <stdio.h>


/**
 * Create a server socket on the given port, all interfaces. Return
 * file descriptor. Will call exit(1) on error.
 */
int create_socket(int port);


/**
 * Accept a connection on a server socket. Open file streams for
 * reading and writing and store them in the pointers in and out.
 * Returns connection file descriptor.
 */
int accept_connection(int sockfd, FILE** in, FILE** out);

/**
 * Close the connection with the given file descriptor and in/out streams.
 */
void close_connection(int sockfd, FILE** in, FILE** out);

/**
 * Close the server socket with the given socket file descriptor.
 */
void close_socket(int sockfd);


#endif
