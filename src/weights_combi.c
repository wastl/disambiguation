#include <igraph/igraph.h>
#include <float.h>

#include "weights_combi.h"

/**
 * Compute the edge weights according to the combined information content as described in
 * https://ub-madoc.bib.uni-mannheim.de/35464/1/schuhmacher14a.pdf
 *
 * - for each predicate (label), we have to determine the relative
 *   selectivity over the whole graph
 * - for each object (target vertice), we have to determine the
 *   relative selectivity over the graph (this is simply the in-degree
 *   divided by the total number of edges)
 */ 
void compute_weights_combi(rgraph *graph) {
  igraph_integer_t vcount = igraph_vcount(graph->graph); 
  igraph_integer_t ecount = igraph_ecount(graph->graph); 


  // first step: calculate number of occurrences of each predicate and
  // object in the edges list
  int *p_pred = calloc(vcount, sizeof(int));
  int *p_obj  = calloc(vcount, sizeof(int));

  igraph_es_t edge_s;
  igraph_es_all(&edge_s, IGRAPH_EDGEORDER_ID);

  igraph_eit_t edge_it;
  igraph_eit_create(graph->graph, edge_s, &edge_it);

  igraph_integer_t from, pred, to, eid;
  while(!IGRAPH_EIT_END(edge_it)) {
    eid = IGRAPH_EIT_GET(edge_it);
    
    // increase counter for object
    igraph_edge(graph->graph, eid, &from, &to);    
    p_obj[to]++;


    // increase counter for label
    pred = igraph_vector_e(graph->labels,eid);
    p_pred[pred]++;

    IGRAPH_EIT_NEXT(edge_it);
  }
  
  igraph_eit_destroy(&edge_it);


  // second step: iterate over all vertices and compute the
  // information content value
  
  double ic_pred[vcount];
  double ic_obj[vcount];

  igraph_vs_t vertice_s;
  igraph_vs_all(&vertice_s);
  
  igraph_vit_t vertice_it;
  igraph_vit_create(graph->graph, vertice_s, &vertice_it);

  igraph_integer_t vid;
  while(!IGRAPH_VIT_END(vertice_it)) {
    vid = IGRAPH_VIT_GET(vertice_it);

    ic_pred[vid] = p_pred[vid] > 0 ? -log( (double)p_pred[vid] / ecount) : DBL_MAX;
    ic_obj[vid] = p_obj[vid] > 0   ? -log( (double)p_obj[vid] / ecount)  : DBL_MAX;

    IGRAPH_VIT_NEXT(vertice_it);
  }

  igraph_vit_destroy(&vertice_it);


  // third step: iterate again over all edges and assign combiIC
  // weight
  igraph_vector_resize(graph->weights, igraph_ecount(graph->graph));
  igraph_vector_fill(graph->weights, 1.0);

  igraph_eit_create(graph->graph, edge_s, &edge_it);

  while(!IGRAPH_EIT_END(edge_it)) {
    eid = IGRAPH_EIT_GET(edge_it);
    
    igraph_edge(graph->graph, eid, &from, &to); 

    pred = igraph_vector_e(graph->labels,eid);

    if(ic_pred[pred] < DBL_MAX && ic_obj[to] < DBL_MAX) {
      igraph_vector_set(graph->weights, eid, 1.0 / (ic_pred[pred] + ic_obj[to]));
    }

    //printf("edge weight %d: %.3f\n", eid, 1.0 / (ic_pred[pred] + ic_obj[to]));

    IGRAPH_EIT_NEXT(edge_it);
  }
  
  igraph_eit_destroy(&edge_it);

  free(p_pred);
  free(p_obj);
}
