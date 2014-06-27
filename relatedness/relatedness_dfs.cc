#include <limits.h>
#include <float.h>
#include <strings.h>

#include <iostream>
#include <vector>

#include "relatedness_dfs.h"

using namespace mico::graph;
using namespace mico::relatedness;

// constructor: initialise helper structures
mico::relatedness::dfs::dfs(rgraph* graph, int max_dist) : base(graph), max_dist(max_dist) {
  dist = new double[graph->num_vertices];
}

// destructor: free helper structures
mico::relatedness::dfs::~dfs() {
  delete[] dist;
}

// DFS to compute distances for all vertices up to a certain depth
inline void mico::relatedness::dfs::collect(int node, double pweight, int depth) {
  long int i, j, eid;
  igraph_integer_t v, w;

  j=(long int) VECTOR(graph->graph->os)[node+1];
  for (i=(long int) VECTOR(graph->graph->os)[node]; i<j; i++) {
    eid = (long int)VECTOR(graph->graph->oi)[i];
    v   = VECTOR(graph->graph->to)[eid];

    if(dist[v] > pweight + VECTOR(*graph->weights)[eid]) {
      dist[v] = pweight + VECTOR(*graph->weights)[eid];
      if(depth > 1) {
	collect(v, dist[v], depth-1);
      }
    }
  }

  j=(long int) VECTOR(graph->graph->is)[node+1];
  for (i=(long int) VECTOR(graph->graph->is)[node]; i<j; i++) {
    eid = (long int)VECTOR(graph->graph->ii)[i];
    v   = VECTOR(graph->graph->from)[eid];
    
    if(dist[v] > pweight + VECTOR(*graph->weights)[eid]) {
      dist[v] = pweight + VECTOR(*graph->weights)[eid];
      if(depth > 1) {
	collect(v, dist[v], depth-1);
      }
    }
  }

}

double mico::relatedness::dfs::relatedness(const char* sfrom, const char* sto) {
  long int i, j, u, v, eid;

  double alt;

  int from = graph->get_vertice_id(sfrom);
  int to   = graph->get_vertice_id(sto);

  if(from == -1 || to == -1 || from >= graph->num_vertices || to >= graph->num_vertices) {
    return DBL_MAX;
  }

  // initialise distances 
  for(i=0; i<graph->num_vertices; i++) {
    dist[i] = DBL_MAX;
  }
  dist[from] = 0.0;

  if(max_dist > 0) {
    collect(from,dist[from],max_dist); 
  }  

  return dist[to];
}
