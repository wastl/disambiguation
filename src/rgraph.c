#include "rgraph.h"

/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph) {
  /* turn on attribute handling */
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  graph->uris  = malloc(sizeof(art_tree));
  graph->graph = malloc(sizeof(igraph_t));
  graph->vertices = malloc(VINC * sizeof(char*));
  graph->labels = malloc(sizeof(igraph_vector_t));
  graph->weights = malloc(sizeof(igraph_vector_t));

    // init empty graph
  igraph_empty(graph->graph,0,GRAPH_MODE);

  // init empty trie for URI->vertice mapping
  init_art_tree(graph->uris);

  graph->num_vertices = 0;

  igraph_vector_init(graph->labels,0);
  igraph_vector_init(graph->weights,0);
}


int _destroy_trie_values_cb(void *_data, const unsigned char *key, uint32_t key_len, void *value) {
  free(value);
}

/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph) {
  int i;

  igraph_destroy(graph->graph);
  free(graph->graph);

  for(i=0; i<graph->num_vertices; i++) {
    free(graph->vertices[i]);
  }

  free(graph->vertices);

  igraph_vector_destroy(graph->labels);
  igraph_vector_destroy(graph->weights);

  free(graph->labels);
  free(graph->weights);

  art_iter(graph->uris,_destroy_trie_values_cb,NULL);  

  destroy_art_tree(graph->uris);
  free(graph->uris);
  
}
