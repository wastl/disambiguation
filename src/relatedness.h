#ifndef HAVE_RELATEDNESS_H
#define HAVE_RELATEDNESS_H 1

#include <igraph/igraph.h>
#include "art.h"

typedef struct rgraph {
  art_tree *uris;   /* map from URIs to vertice IDs */
  igraph_t *graph;  /* IGraph representing the triples */
  int num_vertices; /* number of vertices in trie and graph */
} rgraph;


/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph);

/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph);


#endif
