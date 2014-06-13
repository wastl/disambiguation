
#include <igraph/igraph.h>

#include "rgraph.h"
#include "relatedness.h"
#include "disambiguation.h"


void disambiguation(rgraph *graph, wsd_term_t *terms, int num_terms, int max_dist, wsd_centrality_algorithm algorithm) {
  int i, j, t, s;

  int num_vertices=0;     // how many vertices in total
  int offsets[num_terms]; // markers to see where in the graph vertice
			  // list the vertices for a term start

#define get_node_label(x,y) terms[offsets[x]].candidates[y].uri
#define get_node_id(x,y)    offsets[x]+y
#define N                   num_terms
#define N_w(i)              terms[i].num_candidates

  for(i = 0; i < num_terms; i++) {
    offsets[i]    = num_vertices;
    num_vertices += terms[i].num_candidates;
  }

  // initialise graph with a vertice for every candidate of every term
  igraph_t wsd_graph;
  igraph_empty(&wsd_graph,num_vertices,IGRAPH_DIRECTED);

  igraph_vector_t wsd_weights;
  igraph_vector_init(&wsd_weights,0);
  

  // 1. build dependency graph (see http://www.cse.unt.edu/~rada/papers/sinha.ieee07.pdf)

  // TODO: this part can easily be parallelized, especially computing
  // edge weights could benefit!
  for(i = 0; i < num_terms; i++) {
    for(j = i+1; j <= i+max_dist && j < num_vertices; j++) {
      for(t = 0; t < N_w(i); t++) {
	for(s = 0; s < N_w(j); s++) {
	  double w = relatedness(graph, get_node_label(i,t), get_node_label(j,s), NULL);
	  igraph_add_edge(&wsd_graph,get_node_id(i,t),get_node_id(j,s));
          igraph_vector_push_back (&wsd_weights, w);
	}
      }
    }
  }

  // 2. compute centrality for each vertex and write back to
  // candidates
  igraph_vector_t wsd_centralities;
  igraph_vector_init(&wsd_centralities,0);

  igraph_arpack_options_t options;
  igraph_arpack_options_init(&options);

  igraph_vs_t vertice_s;
  igraph_vs_all(&vertice_s);

  // we support a number of different algorithms through igraph ...
  switch(algorithm) {
  case ALGO_PAGERANK:
    // igraph version <0.7 and >= 0.5
    igraph_pagerank(&wsd_graph, &wsd_centralities, 0, igraph_vss_all(), 0, 0.85, &wsd_weights, 0);
    break;
  case ALGO_CLOSENESS:
    igraph_closeness(&wsd_graph, &wsd_centralities, igraph_vss_all(), IGRAPH_ALL, &wsd_weights);
    break;
  case ALGO_BETWEENNESS:
    igraph_betweenness(&wsd_graph, &wsd_centralities, igraph_vss_all(), 0, &wsd_weights, 0);
    break;
  case ALGO_EIGENVECTOR:
  default:
    igraph_eigenvector_centrality(&wsd_graph, &wsd_centralities, NULL, 0, 1, &wsd_weights, &options);
    break;
  }  

  for(i = 0; i<N; i++) {
    for(j = 0; j<N_w(i); j++) {
      terms[i].candidates[j].rank = igraph_vector_e(&wsd_centralities,get_node_id(i,j));
    }
  }


  igraph_vector_destroy(&wsd_centralities);
  igraph_vector_destroy(&wsd_weights);
  igraph_destroy(&wsd_graph);
}
