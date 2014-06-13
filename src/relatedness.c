
#include "relatedness.h"

/**
 * Compute relatedness by finding the shortest path between two
 * vertices. Returns the relatedness value (smaller values are
 * stronger relatedness). If the optional pointer to an empty edges
 * vector is given, the vector will afterwards contain a list of edges
 * used in the computation.
 */
double relatedness(rgraph* g, const char* from, const char* to, igraph_vector_t *edges) {
  int fromid = rgraph_get_vertice_id(g,from);
  int toid   = rgraph_get_vertice_id(g,to);

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
  igraph_vector_t *_edges;
  if(edges) {
    _edges = edges;
  } else {
    _edges = malloc(sizeof(igraph_vector_t));
  }

  igraph_vector_init(_edges,0);
  igraph_get_shortest_path_dijkstra(g->graph, NULL, _edges, fromid, toid, g->weights, IGRAPH_ALL);

  double r = 0.0;

  int i;
  for(i=0; i<igraph_vector_size(_edges); i++) {
    r += igraph_vector_e(g->weights,igraph_vector_e(_edges,i));
  }

  if(!edges) {
    igraph_vector_destroy(_edges);
    free(_edges);
  }

  return r;
}
