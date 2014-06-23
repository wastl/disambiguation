#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "rgraph.h"
#include "graphio.h"
#include "relatedness.h"
#include "disambiguation.h"
#include "network.h"

#include "proto-disambiguation.pb-c.h"

typedef struct wsd_worker {
  int connection;   // file descriptor of connection
  FILE* in;         // input stream
  FILE* out;        // output stream
  rgraph* graph;    // shared reference to the graph
#ifdef USE_THREADS
  pthread_t thread; // thread descriptor in case threading is enabled
#endif
} wsd_worker_t;


void usage(char *cmd) {
  printf("Usage: %s -i fileprefix [-e edges] [-v vertices]\n", cmd);
  printf("Options:\n");
  printf("  -p port          interact through the socket port given as argument\n");
  printf("  -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)\n");
  printf("  -e edges         hint on the number of edges in the graph (can improve startup performance)\n");
  printf("  -v vertices      hint on the number of vertices in the graph (improve startup performance)\n");
  exit(1);
}


void print_results(wsd_term_t *terms, int num_terms, FILE* stream) {
  // one term per line, then candidate URI followed by rank
  int i, j;
  for(i = 0; i<num_terms; i++) {
    fprintf(stream,"term: %s; candidates:", terms[i].term);
    for(j = 0; j<terms[i].num_candidates; j++) {
      fprintf(stream," %s=%.4f", terms[i].candidates[j].uri, terms[i].candidates[j].rank);
    }
    fprintf(stream,"\n");
  }
}


void* worker(void* data) {
  wsd_worker_t* wsd = (wsd_worker_t*) data;

  // read from stdin pairs of vertices and compute relatedness
  char *line    = NULL;
  size_t len    = 0;
  char *ptr;
  wsd_term_t *terms = 0;
  char **candidates = 0;
  int num_terms = 0, num_candidates = 0;
  double r;
  int i, j;


  fprintf(wsd->out,"> ");
  fflush(wsd->out);
  while((getline(&line,&len,wsd->in) != -1)) {

    if(strcmp("\n",line) == 0) {
      // input finished, start calculation and print results
      fprintf(wsd->out,"computing disambiguation values ...\n");

      disambiguation(wsd->graph, terms, num_terms, 5, ALGO_EIGENVECTOR);
      print_results(terms, num_terms, wsd->out);

      // free all allocated memory (strdup, malloc, realloc)
      for(i = 0; i<num_terms; i++) {
	free(terms[i].term);
	for(j=0; j<terms[i].num_candidates; j++) {
	  free(terms[i].candidates[j].uri);
	}
	free(terms[i].candidates);
      }
      free(terms);
      num_terms = 0;

      fprintf(wsd->out,"> ");
      fflush(wsd->out);	
    } else {
      // add a new term per line
      num_candidates = 0;

      // find all candidates, starting with the URI after the first
      // blank
      ptr = line;
      while(*ptr) {
	ptr++;
	while(isspace(*ptr) && *ptr) {
	  // erase all space characters and replace them with
	  // end-of-string markers
	  *(ptr++) = '\0';
	  if(!isspace(*ptr) && *ptr) {
	    candidates = realloc( candidates, (++num_candidates) * sizeof(char*));
	    candidates[num_candidates-1] = ptr;
	  }
	}
      }
      
      // create new term entry
      terms = realloc(terms, ++num_terms * sizeof(wsd_term_t));
      terms[num_terms-1].term = strdup(line); // string now terminated at first space
      terms[num_terms-1].num_candidates = num_candidates;
      terms[num_terms-1].candidates = malloc( num_candidates * sizeof(wsd_candidate_t) );
      for(i = 0; i<num_candidates; i++) {
	terms[num_terms-1].candidates[i].uri = strdup(candidates[i]);
      }
    }
  }

  free(candidates);
  free(line);

  if(wsd->connection) {
    close_connection(wsd->connection, &wsd->in, &wsd->out);
  }

  free(wsd);

  return 0;
}



void main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;
  int port = 0, socket, connection;
  long int reserve_edges = 1<<16;
  long int reserve_vertices = 1<<12;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:e:v:p:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'e':
      reserve_edges = atol(optarg);
      break;
    case 'v':
      reserve_vertices = atol(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph graph;


    // init empty graph
    init_rgraph(&graph, reserve_vertices, reserve_edges);

    // first restore existing dump in case -i is given
    restore_graph(&graph,ifile);

    // open socket if -p is specified on command line

    wsd_worker_t* wsd = (wsd_worker_t*)malloc(sizeof(wsd_worker_t));
    if(port) {
      socket = create_socket(port);
      while( (wsd->connection = accept_connection(socket, &wsd->in, &wsd->out)) >= 0) {
	wsd->graph = &graph;	

#ifdef USE_THREADS
	pthread_create(&wsd->thread, NULL, &worker, wsd);
#else
	worker(wsd);
#endif    
	// prepare memory for next connection
	wsd = malloc(sizeof(wsd_worker_t));
      }

    } else {
      wsd->in         = stdin;
      wsd->out        = stdout;
      wsd->graph      = &graph;
      wsd->connection = 0;

      worker(wsd);
    }

    


    destroy_rgraph(&graph);
    } else {
    usage(argv[0]);
    }

}
