#include <iostream>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "disambiguation.h"
#include "../graph/rgraph.h"
#include "../communication/connection.h"

extern "C" {
#include "../communication/network.h"
}


using namespace mico::network;
using namespace mico::graph;

void usage(char *cmd) {
  printf("Usage: %s -i fileprefix [-e edges] [-v vertices]\n", cmd);
  printf("Options:\n");
  printf("  -p port          interact through the socket port given as argument\n");
  printf("  -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)\n");
  printf("  -e edges         hint on the number of edges in the graph (can improve startup performance)\n");
  printf("  -v vertices      hint on the number of vertices in the graph (improve startup performance)\n");
  exit(1);
}



void* worker(void* data) {
  connection<WSDDisambiguationRequest>* wsd = (connection<WSDDisambiguationRequest>*) data;


  WSDDisambiguationRequest* req = NULL;

  // read requests until finished
  try {
    while( (req = wsd->nextRequest()) != NULL) {
      std::cout << "received new request\n";
      std::cout << req->DebugString();
      req->disambiguation(wsd->graph);

      // std::cout << "response:\n";
      // std::cout << req->DebugString();
    
      *wsd << *req;
      delete req;
      req = NULL;
    }
  } catch(std::ios_base::failure) {
    std::cerr << "error writing response to network connection\n";
    if(req != NULL) {
      delete req;
    }
  }

  delete wsd;

  return 0;
}



int main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;
  int port = 0, socket;
  long int reserve_edges = 1<<16;
  long int reserve_vertices = 1<<12;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:p:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph graph;


    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

    // open socket if -p is specified on command line
    if(port) {
      socket = create_socket(port);
      int conn_fd;
#ifndef PROFILING
      while( (conn_fd = accept_connection(socket)) >= 0) {
#else
      if( (conn_fd = accept_connection(socket)) >= 0) {
#endif
	connection<WSDDisambiguationRequest>* wsd = new connection<WSDDisambiguationRequest>(conn_fd, &graph);

#if defined(USE_THREADS) && !defined(PROFILING)
	pthread_create(&wsd->thread, NULL, &worker, wsd);
#else
	worker(wsd);
#endif    
      }

    } else {
      connection<WSDDisambiguationRequest>* wsd = new connection<WSDDisambiguationRequest>(&graph);
      worker(wsd);
    }

    
    google::protobuf::ShutdownProtobufLibrary();

  } else {
    usage(argv[0]);
  }

  return 0;
}
