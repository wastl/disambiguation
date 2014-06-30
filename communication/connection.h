// -*- mode: c++; -*-
#ifndef HAVE_WORKER_H
#define HAVE_WORKER_H

#include <iostream>
#include <ext/stdio_filebuf.h>
#include <arpa/inet.h>

#include "disambiguation_request.pb.h"

#include "../graph/rgraph.h"



namespace mico {
  namespace network {

    class BaseConnection {
    protected:
      int conn;
  
      std::istream* in;
      std::ostream* out;
      
    public:

      pthread_t thread; // thread descriptor in case threading is enabled
  
      /**
       * Create a connection for sending/receiving requests from standard input/output
       */
      BaseConnection();

      /**
       * Create a connection for sending/receiving requests from a file descriptor
       */
      BaseConnection(int conn);

      ~BaseConnection();
    };



    // R subclass of DisambiguationRequest
    template<class R> class Connection : public BaseConnection {


    public:
      /**
       * Create a connection for sending/receiving requests from standard input/output
       */
      Connection() : BaseConnection() {};

      /**
       * Create a connection for sending/receiving requests from a file descriptor
       */
      Connection(int conn) : BaseConnection(conn) {};



      /**
       * Return a pointer to the next request received from the connection, or NULL
       * if parsing failed (e.g. connection was closed). Will first read in an integer indicating
       * the size of the message (number of bytes) and then the serialized message itself.
       */
      R* nextRequest();

      /**
       * Write out a disambiguation request over the connection. Will first write an integer indicating
       * the size of the message (number of bytes) and then the serialized message itself.
       */
      Connection& operator<<(R &r);

      /**
       * Read in a disambiguation request from the connection. Will first read in an integer indicating
       * the size of the message (number of bytes) and then the serialized message itself.
       */
      Connection& operator>>(R &r);
    };


  }
}




template <class R> mico::network::Connection<R>& mico::network::Connection<R>::operator<<(R &r) {
  uint32_t length = r.ByteSize();
  uint32_t hlength = htonl(length);
  std::cout << "sending a response of "<<length<<" bytes ...\n";

  char buf[length];
  r.SerializeToArray(buf,length);

  out->write((char*)&hlength, sizeof(int));
  out->write(buf,length);
  out->flush();

  return *this;
}


template <class R> mico::network::Connection<R>& mico::network::Connection<R>::operator>>(R &r) {

  if( !in->eof() ) {
    // read length of next message
    uint32_t length;
    in->read((char*)&length, sizeof(int));

    char buf[length];  
    in->read(buf,ntohl(length));

    if(*in) {
      r.ParseFromArray(buf, ntohl(length));
    }
  }
  return *this;
}



template <class R> R* mico::network::Connection<R>::nextRequest() {
  if( in->eof() ) {
    return NULL;
  }

  // read length of next message
  uint32_t length;
  in->read((char*)&length, sizeof(int));

  char buf[ntohl(length)];  
  in->read(buf,ntohl(length));

  std::cout << "reading in next message of " << ntohl(length) << " bytes\n";

  if(*in) {
    R* r = new R(); 
    if(r->ParseFromArray(buf, ntohl(length))) {
      return r;
    } else {
      delete r;
      return NULL;
    }
  } else {
    return NULL;
  }
}




#endif
