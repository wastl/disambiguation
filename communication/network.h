// -*- mode: c++; -*-
#ifndef HAVE_NETWORK_H
#define HAVE_NETWORK_H 1

#include <iostream>

#include "connection.h"

namespace mico {
  namespace network {


    /**
     * Basic socket implementation, delegating to the libc functionalities for networking.
     */
    class BaseSocket {
    protected:

      int sockfd;

      int base_accept();

    public:
      /**
       * Create a server socket on the given port, all interfaces. 
       * Will call exit(1) on error.
       */
      BaseSocket(int port);

      /**
       * Close the server socket 
       */
      ~BaseSocket();

    };


    /**
     * Template class of a socket, returning connections of a certain type.
     */
    template<class T> class Socket : public BaseSocket {


    public:
      
      Socket(int port) : BaseSocket(port) {};


      /**
       * Accept a connection on a server socket. Open file streams for
       * reading and writing and store them in the pointers in and out.
       * Returns connection file descriptor.
       */
      Connection<T>* accept() {
	int fd = base_accept();
	if(fd >= 0) {
	  return new Connection<T>(fd);
	} else {
	  std::cerr << "could not accept connection!\n"; 
	  return NULL;
	}
      }; 

    };
  }
}


#endif
