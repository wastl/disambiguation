#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#include "network.h"

/**
 * Create a server socket on the given port, all interfaces
 */
int create_socket(int port) {
  fprintf(stderr, "creating new socket on port %d ...\n",port);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    perror("error creating socket:");
    exit(1);
  }

  struct sockaddr_in addr;
  bzero((char*) &addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if(bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("error binding socket:");
    exit(1);
  }
 
  listen(sockfd,5);

  return sockfd;
}

/**
 * Accept a connection on a server socket
 */
int accept_connection(int sockfd, FILE** in, FILE** out) {
  fprintf(stderr, "awaiting connection ...\n");

  int newsockfd;
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

  if(newsockfd < 0) {
    perror("error accepting connection:");
    exit(1);
  }

  *in  = fdopen(newsockfd, "r");
  *out = fdopen(newsockfd, "w");
  
  return newsockfd;
}


void close_connection(int sockfd, FILE** in, FILE** out) {
  fprintf(stderr, "closing connection ...\n");

  fclose(*in);
  fclose(*out);
  close(sockfd);
}


/**
 * Close the server socket with the given socket file descriptor.
 */
void close_socket(int sockfd) {
  fprintf(stderr, "closing socket ...\n");
  close(sockfd);
}

