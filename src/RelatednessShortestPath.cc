#include <limits.h>
#include <float.h>

#include "RelatednessShortestPath.h"

// constructor: initialise helper structures
RelatednessShortestPath::RelatednessShortestPath(rgraph* graph) : RelatednessBase(graph) {
  dist = new double[graph->num_vertices];
  idx =  new int[graph->num_vertices];
  len =  new int[graph->num_vertices];

  pq_init(&queue,graph->num_vertices, dist,idx);
}

// destructor: free helper structures
RelatednessShortestPath::~RelatednessShortestPath() {
  pq_destroy(&queue);
  delete[] dist;
  delete[] idx;
  delete[] len;  
}


double RelatednessShortestPath::relatedness(const char* sfrom, const char* sto, int max_dist) {

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
