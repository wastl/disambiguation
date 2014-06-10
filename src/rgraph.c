#include "rgraph.h"

/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph) {
  /* turn on attribute handling */
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  graph->uris  = kh_init(uris);
  graph->graph = malloc(sizeof(igraph_t));
  graph->vertices = malloc(VINC * sizeof(char*));
  graph->labels = malloc(sizeof(igraph_vector_t));
  graph->weights = malloc(sizeof(igraph_vector_t));

  // init empty graph
  igraph_empty(graph->graph,0,GRAPH_MODE);

  graph->num_vertices = 0;

  igraph_vector_init(graph->labels,0);
  igraph_vector_init(graph->weights,0);
}


/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph) {
  int i;

  igraph_destroy(graph->graph);
  free(graph->graph);

  // free strings in graph->vertices and graph->uris
  for(i=0; i<graph->num_vertices; i++) {
    free(graph->vertices[i]);
  }

  free(graph->vertices);

  igraph_vector_destroy(graph->labels);
  igraph_vector_destroy(graph->weights);

  free(graph->labels);
  free(graph->weights);

  kh_destroy(uris, graph->uris);
  
}


/**
 * lookup the vertice id of the vertice representing the given uri. Returns a pointer to the vertice
 * ID (type int*) if the URI is found or NULL otherwise.
 */
inline int rgraph_get_vertice_id(rgraph *graph, const char* uri) {
  khiter_t k = kh_get(uris,graph->uris,uri);
  if(k == kh_end(graph->uris)) {
    return 0;
  } else {
    return kh_val(graph->uris, k);
  }
} 



inline void rgraph_set_vertice_id(rgraph *graph, const char* uri, int vid) {
  int err;
  khiter_t k = kh_put(uris, graph->uris, uri, &err);
  kh_val(graph->uris, k) = vid;
} 
