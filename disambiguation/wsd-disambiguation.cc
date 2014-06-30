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
#include "../threading/thread.h"
#include "../communication/connection.h"
#include "../communication/network.h"



using namespace mico::threading;
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


typedef Connection<WSDDisambiguationRequest> connection_t;

class worker : public virtual thread {

  rgraph_complete& graph;
  connection_t*    connection;

public:
  
  worker(connection_t* connection, rgraph_complete& graph) : thread(), connection(connection), graph(graph) {};
  

  void run() {
    WSDDisambiguationRequest* req = NULL;

    // read requests until finished
    try {
      while( (req = connection->nextRequest()) != NULL) {
	std::cout << "received new request\n";
	std::cout << req->DebugString();
	req->disambiguation(&graph);

	// std::cout << "response:\n";
	// std::cout << req->DebugString();
    
	*connection << *req;
	delete req;
	req = NULL;
      }
    } catch(std::ios_base::failure) {
      std::cerr << "error writing response to network connection\n";
      if(req != NULL) {
	delete req;
      }
    }

    delete connection;
  };

};


int main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;
  int port = 0;
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
    rgraph_complete graph;


    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

    // open socket if -p is specified on command line
    if(port) {
      Socket<WSDDisambiguationRequest> socket(port);
      Connection<WSDDisambiguationRequest>* conn;
#ifndef PROFILING
      while( (conn = socket.accept()) != NULL) {
#else
	if( (conn = socket.accept()) != NULL) {
#endif
	worker* w = new worker(conn, graph);
	w->start();
#ifdef PROFILING
	w->join();
#endif    
      }

    } else {
	worker* w = new worker(new Connection<WSDDisambiguationRequest>(), graph);
	w->start();
	w->join();
    }

    
    google::protobuf::ShutdownProtobufLibrary();

  } else {
    usage(argv[0]);
  }

  return 0;
}
