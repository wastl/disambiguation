#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <time.h>

#include "art.h"
#include "rgraph.h"
#include "graphio.h"


void usage(char *cmd) {
  printf("Usage: %s -i restorefile\n", cmd);
  exit(1);
}

/**
 * Compute relatedness by finding the shortest path between two vertices.
 */
double relatedness(rgraph* g, const char* from, const char* to) {
  int *fromid = art_search(g->uris,from, strlen(from));
  int *toid   = art_search(g->uris,to, strlen(to));

  if(!fromid || !toid) {
    if (!fromid) {
      printf("<%s> not found in the graph!\n", from);
    }
    if (!toid) {
      printf("<%s> not found in the graph!\n", to);
    }
    return DBL_MAX;
  }

  // holds the edges of the shortest path
  igraph_vector_t edges;
  igraph_vector_init(&edges,0);
  igraph_get_shortest_path_dijkstra(g->graph, NULL, &edges, *fromid, *toid, g->weights, IGRAPH_ALL);

  double r = 0.0;

  int i;
  for(i=0; i<igraph_vector_size(&edges); i++) {
    printf("%s ", g->vertices[(int)igraph_vector_e(g->labels,igraph_vector_e(&edges,i))]);
    //printf("%d ", (int)igraph_vector_e(g->labels,igraph_vector_e(&edges,i)));
    r += igraph_vector_e(g->weights,igraph_vector_e(&edges,i));
  }
  printf("\n");

  igraph_vector_destroy(&edges);

  return r;
}


void main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph graph;

    // init empty graph
    init_rgraph(&graph);

    // first restore existing dump in case -i is given
    restore_graph(&graph,ifile);

    // read from stdin pairs of vertices and compute relatedness
    char *line    = NULL;
    size_t len    = 0;
    char *from, *to, *send;
    double r;
    while((getline(&line,&len,stdin) != -1)) {
      from = line;
      to   = line;
      while(*to != ' ' && to) {
	to++;
      }
      *to='\0'; to++;
      send = to;
      while(send && *send != '\n') {
	send++;
      }
      *send = '\0';

      printf("computing relatedness for %s and %s ... ",from,to);
      fflush(stdout);
      r = relatedness(&graph,from,to);
      printf("%.6f!\n",r);
    }

    free(line);

    destroy_rgraph(&graph);
  } else {
    usage(argv[0]);
  }

}
