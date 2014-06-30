#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#include "network.h"
#include "connection.h"

namespace mico {
  namespace network {

    /**
     * Create a server socket on the given port, all interfaces
     */
    BaseSocket::BaseSocket(int port) {
      std::cerr << "creating new socket on port " << port << "\n";

      sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    }

    /**
     * Accept a connection on a server socket
     */
    int BaseSocket::base_accept() {
      std::cerr << "awaiting connection ...\n";

      int newsockfd;
      struct sockaddr_in cli_addr;
      socklen_t clilen = sizeof(cli_addr);

      newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

      if(newsockfd < 0) {
	perror("error accepting connection:");
	exit(1);
      }

  
      return newsockfd;
    }


    /**
     * Close the server socket with the given socket file descriptor.
     */
    BaseSocket::~BaseSocket() {
      std::cerr << "closing socket ...\n";
      close(sockfd);
    }


  }
}
