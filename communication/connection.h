// -*- mode: c++; -*-
#ifndef HAVE_WORKER_H
#define HAVE_WORKER_H

#include <iostream>
#include <ext/stdio_filebuf.h>


#include "disambiguation_request.pb.h"

extern "C" {
#include "../graph/rgraph.h"
}


namespace mico {
  namespace network {

    class base_connection {
    protected:
      int conn;
  
      std::istream* in;
      std::ostream* out;
      
    public:

      // TODO: this should be removed from here, as it is specific to wsd-disambiguation.cc
      rgraph* graph;

      pthread_t thread; // thread descriptor in case threading is enabled
  
      /**
       * Create a connection for sending/receiving requests from standard input/output
       */
      base_connection(rgraph *graph);

      /**
       * Create a connection for sending/receiving requests from a file descriptor
       */
      base_connection(int conn, rgraph *graph);

      ~base_connection();
    };



    // R subclass of DisambiguationRequest
    template<class R> class connection : public base_connection {


    public:
      /**
       * Create a connection for sending/receiving requests from standard input/output
       */
      connection(rgraph *graph) : base_connection(graph) {};

      /**
       * Create a connection for sending/receiving requests from a file descriptor
       */
      connection(int conn, rgraph *graph) : base_connection(conn, graph) {};



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
      connection& operator<<(R &r);

      /**
       * Read in a disambiguation request from the connection. Will first read in an integer indicating
       * the size of the message (number of bytes) and then the serialized message itself.
       */
      connection& operator>>(R &r);
    };


  }
}




template <class R> mico::network::connection<R>& mico::network::connection<R>::operator<<(R &r) {
  int length = r.ByteSize();
  std::cout << "sending a response of "<<length<<" bytes ...\n";

  char buf[length];
  r.SerializeToArray(buf,length);

  out->write((char*)&length, sizeof(int));
  out->write(buf,length);
  out->flush();

  return *this;
}


template <class R> mico::network::connection<R>& mico::network::connection<R>::operator>>(R &r) {

  if( !in->eof() ) {
    // read length of next message
    int length;
    *in >> length;

    char buf[length];  
    in->read(buf,length);

    if(*in) {
      r.ParseFromArray(buf, length);
    }
  }
  return *this;
}



template <class R> R* mico::network::connection<R>::nextRequest() {
  if( in->eof() ) {
    return NULL;
  }

  // read length of next message
  int length;
  *in >> length;

  char buf[length];  
  in->read(buf,length);

  std::cout << "reading in next message of " << length << " bytes\n";

  if(*in) {
    R* r = new R(); 
    if(r->ParseFromArray(buf, length)) {
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
