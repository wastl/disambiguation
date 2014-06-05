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


void usage() {
  printf("Usage: relatedness -i restorefile\n");
  exit(1);
}

/**
 * Compute relatedness by finding the shortest path between two vertices.
 */
double relatedness(rgraph* g, const char* from, const char* to) {
  int *fromid = art_search(g->uris,from, strlen(from));
  int *toid   = art_search(g->uris,to, strlen(to));

  if(!fromid || !toid) {
    return DBL_MAX;
  }

  // holds the edges of the shortest path
  igraph_vector_t edges;
  igraph_get_shortest_path_dijkstra(g->graph, NULL, &edges, *fromid, *toid, g->weights, IGRAPH_ALL);

  printf("distance: %d\n", igraph_vector_size(&edges));

  return igraph_vector_prod(&edges);
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
      usage();
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
    char *from    = malloc(2048 * sizeof(char));
    char *to      = malloc(2048 * sizeof(char));
    double r;
    while((getline(&line,&len,stdin) != -1)) {
      sscanf(line,"%2048s %2048s",from,to);
      printf("computing relatedness for %s and %s ... ",from,to);
      fflush(stdout);
      r = relatedness(&graph,from,to);
      printf("%.6f!\n",r);
    }

    destroy_rgraph(&graph);
  } else {
    usage();
  }

}
