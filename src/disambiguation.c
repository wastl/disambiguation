
#include <igraph/igraph.h>

#include "rgraph.h"
#include "relatedness.h"
#include "disambiguation.h"


void disambiguation(rgraph *graph, wsd_term_t *terms, int num_terms, int max_dist) {
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
  

  // build dependency graph (see http://www.cse.unt.edu/~rada/papers/sinha.ieee07.pdf)

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


  igraph_vector_destroy(&wsd_weights);
  igraph_destroy(&wsd_graph);
}
