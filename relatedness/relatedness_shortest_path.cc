#include <limits.h>
#include <float.h>
#include <strings.h>

#include <iostream>
#include <vector>

#include "relatedness_shortest_path.h"

// constructor: initialise helper structures
mico::relatedness::shortest_path::shortest_path(rgraph* graph, int max_dist) : mico::relatedness::base(graph), max_dist(max_dist) {
  dist = new double[graph->num_vertices];
  idx =  new int[graph->num_vertices];
  len =  new int[graph->num_vertices];

  pq_init(&queue,graph->num_vertices, dist,idx);
}

// destructor: free helper structures
mico::relatedness::shortest_path::~shortest_path() {
  pq_destroy(&queue);
  delete[] dist;
  delete[] idx;
  delete[] len;  
}

// BFS to look for all vertices up to a certain distance
inline void collect(rgraph* graph, int fromId, pqueue_t* queue, int depth) {
  int eid, v, x, y;

  igraph_es_t es;
  igraph_es_incident(&es,fromId,IGRAPH_ALL);

  igraph_eit_t eit;
  igraph_eit_create(graph->graph, es, &eit);


  while(!IGRAPH_EIT_END(eit)) {
    eid = IGRAPH_EIT_GET(eit);

    igraph_edge(graph->graph, eid, &x, &y); 
    v = x == fromId ? y : x;

    if(queue->indexes[v] == 0) {
      pq_insert(queue, v);
      if(depth > 1) {
	collect(graph, v, queue, depth-1);
      }
    }

    IGRAPH_EIT_NEXT(eit);
  }

  igraph_eit_destroy(&eit);
  igraph_es_destroy(&es);    
}

double mico::relatedness::shortest_path::relatedness(const char* sfrom, const char* sto) {

  int i, u, v, x, y;

  int eid;

  double alt;

  int from = rgraph_get_vertice_id(graph,sfrom);
  int to   = rgraph_get_vertice_id(graph,sto);

  if(from == -1 || to == -1) {
    return DBL_MAX;
  }

  bzero(idx, graph->num_vertices * sizeof(int));

  dist[from] = 0.0;
  len[from]  = 0;
  for(i=0; i<graph->num_vertices; i++) {
    if(i != from) {
      dist[i] = DBL_MAX;
      len[i]  = INT_MAX;
    }
  }

  // only insert those vertice IDs into the queue that are up to max_dist edges away from the
  // from node; this is an optimization to significantly reduce the size of the queue and improve
  // performance 
  pq_insert(&queue,from);
  if(max_dist > 0) {
    collect(graph,from,&queue,max_dist); 
  }
  

  while(!pq_empty(&queue)) {
    u = pq_first(&queue);

    if(u == to || len[u] > max_dist) {
      break;
    }

    igraph_es_t es;
    igraph_es_incident(&es,u,IGRAPH_ALL);

    igraph_eit_t eit;
    igraph_eit_create(graph->graph, es, &eit);


    while(!IGRAPH_EIT_END(eit)) {
      eid = IGRAPH_EIT_GET(eit);

      igraph_edge(graph->graph, eid, &x, &y); 
      v = x == u ? y : x;
      
      alt = dist[u] + igraph_vector_e(graph->weights,eid);
      if(alt < dist[v] && len[u] + 1 <= max_dist) {
	dist[v] = alt;
	len[v]  = len[u] + 1;
	if(queue.indexes[v] != 0) { // only decrease if the value is actually in the queue
	  pq_decrease(&queue,v);
	}
      }
      

      IGRAPH_EIT_NEXT(eit);
    }

    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
  }

  pq_clear(&queue);

  return dist[to];
}
