#include <limits.h>
#include <float.h>

#include "relatedness_shortest_path.h"

// constructor: initialise helper structures
mico::relatedness::shortest_path::shortest_path(rgraph* graph) : mico::relatedness::base(graph) {
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


double mico::relatedness::shortest_path::relatedness(const char* sfrom, const char* sto, int max_dist) {

  int i, u, v, x, y;

  int eid;

  double alt;

  int from = rgraph_get_vertice_id(graph,sfrom);
  int to   = rgraph_get_vertice_id(graph,sto);

  if(from == -1 || to == -1) {
    return DBL_MAX;
  }


  dist[from] = 0.0;
  len[from]  = 0;
  for(i=0; i<graph->num_vertices; i++) {
    if(i != from) {
      dist[i] = DBL_MAX;
      len[i]  = INT_MAX;
    }
    // TODO: it would be clever to insert vertices not in ID order but wrt distance from the from
    // node; this would significantly reduce the time needed by fixUp
    pq_insert(&queue, i);
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
	pq_decrease(&queue,v);
      }

      IGRAPH_EIT_NEXT(eit);
    }

    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
  }

  pq_clear(&queue);

  return dist[to];
}
