#ifndef HAVE_WORKER_H
#define HAVE_WORKER_H

#include <iostream>

#include "disambiguation.h"

extern "C" {
#include "rgraph.h"
}



class WorkerConnection {

private:
  int connection;
  
  std::istream* in;
  std::ostream* out;

public:

  rgraph* graph;

#ifdef USE_THREADS
  pthread_t thread; // thread descriptor in case threading is enabled
#endif
  
  WorkerConnection(rgraph *graph);

  WorkerConnection(int connection, rgraph *graph);

  ~WorkerConnection();

  /**
   * Return a pointer to the next request received from the connection, or NULL
   * if parsing failed (e.g. connection was closed). Will first read in an integer indicating
   * the size of the message (number of bytes) and then the serialized message itself.
   */
  WSDDisambiguationRequest* nextRequest();

  /**
   * Write out a disambiguation request over the connection. Will first write an integer indicating
   * the size of the message (number of bytes) and then the serialized message itself.
   */
  WorkerConnection& operator<<(WSDDisambiguationRequest &r);

  /**
   * Read in a disambiguation request from the connection. Will first read in an integer indicating
   * the size of the message (number of bytes) and then the serialized message itself.
   */
  WorkerConnection& operator>>(WSDDisambiguationRequest &r);
};






#endif
