#include "relatedness.h"

/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph) {
  graph->uris  = malloc(sizeof(art_tree));
  graph->graph = malloc(sizeof(igraph_t));

    // init empty graph
  igraph_empty(graph->graph,0,IGRAPH_DIRECTED);

  // init empty trie for URI->vertice mapping
  init_art_tree(graph->uris);

  graph->num_vertices = 0;
}

/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph) {
  igraph_destroy(graph->graph);
  free(graph->graph);

  destroy_art_tree(graph->uris);
  free(graph->uris);
  
}
