#include <limits.h>
#include <float.h>
#include <strings.h>

#include <iostream>
#include <vector>

#include "relatedness_shortest_path.h"

using namespace mico::graph;
using namespace mico::relatedness;

// constructor: initialise helper structures
mico::relatedness::shortest_path::shortest_path(rgraph* graph, int max_dist) : base(graph), max_dist(max_dist) {
  dist = new double[graph->num_vertices];
  idx =  new int[graph->num_vertices];

  pq_init(&queue,graph->num_vertices, dist,idx);
}

// destructor: free helper structures
mico::relatedness::shortest_path::~shortest_path() {
  pq_destroy(&queue);
  delete[] dist;
  delete[] idx;
}

// BFS to look for all vertices up to a certain distance
inline void mico::relatedness::shortest_path::collect(int node, int depth) {
  long int i, j, v;


  // copied partly from igraph type_indexededgelist.c
  j=(long int) VECTOR(graph->graph->os)[node+1];
  for (i=(long int) VECTOR(graph->graph->os)[node]; i<j; i++) {
    v   = VECTOR(graph->graph->to)[  (long int)VECTOR(graph->graph->oi)[i] ];

    if(idx[v] == 0) {
      pq_insert(&queue, v);
      if(depth > 1) {
	collect(v, depth-1);
      }
    }
  }

  j=(long int) VECTOR(graph->graph->is)[node+1];
  for (i=(long int) VECTOR(graph->graph->is)[node]; i<j; i++) {
    v   = VECTOR(graph->graph->from)[ (long int)VECTOR(graph->graph->ii)[i] ];
    
    if(idx[v] == 0) {
      pq_insert(&queue, v);
      if(depth > 1) {
	collect(v, depth-1);
      }
    }
  }


}

double mico::relatedness::shortest_path::relatedness(const char* sfrom, const char* sto) {
  long int i, j, u, v, eid;

  double alt;

  int from = graph->get_vertice_id(sfrom);
  int to   = graph->get_vertice_id(sto);

  if(from == -1 || to == -1 || from >= graph->num_vertices || to >= graph->num_vertices) {
    return DBL_MAX;
  }

  // clear index before starting computation
  bzero(idx, graph->num_vertices * sizeof(int));

  // clear queue before starting computation
  pq_clear(&queue);

  // initialise distances 
  dist[from] = 0.0;
  for(i=0; i<graph->num_vertices; i++) {
    if(i != from) {
      dist[i] = DBL_MAX;
    }
  }

  // only insert those vertice IDs into the queue that are up to max_dist edges away from the
  // from node; this is an optimization to significantly reduce the size of the queue and improve
  // performance 
  pq_insert(&queue,from);
  if(max_dist > 0) {
    collect(from,max_dist); 
  }
  

  while(!pq_empty(&queue)) {
    u = pq_first(&queue);

    if(u == to) {
      break;
    }

    // process outgoing edges and vertices
    j=(long int) VECTOR(graph->graph->os)[u+1];
    for (i=(long int) VECTOR(graph->graph->os)[u]; i<j; i++) {
      eid = VECTOR(graph->graph->oi)[i];
      v   = VECTOR(graph->graph->to)[(long int)eid];

      alt = dist[u] + VECTOR(*graph->weights)[eid];
      if(alt < dist[v]) {
	dist[v] = alt;
	if(queue.indexes[v] != 0) { // only decrease if the value is actually in the queue
	  pq_decrease(&queue,v);
	}
      }
    }

    // process incoming edges and vertices
    j=(long int) VECTOR(graph->graph->is)[u+1];
    for (i=(long int) VECTOR(graph->graph->is)[u]; i<j; i++) {
      eid = VECTOR(graph->graph->ii)[i];
      v   = VECTOR(graph->graph->from)[(long int)eid];
      

      alt = dist[u] + VECTOR(*graph->weights)[eid];
      if(alt < dist[v]) {
	dist[v] = alt;
	if(queue.indexes[v] != 0) { // only decrease if the value is actually in the queue
	  pq_decrease(&queue,v);
	}
      }
    }
  }

  return dist[to];
}
