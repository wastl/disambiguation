#ifndef HAVE_RGRAPH_H
#define HAVE_RGRAPH_H 1

#include <string.h>
#include <igraph/igraph.h>
#include "art.h"
#include "config.h"


#define ATTR_LABEL "l"
#define ATTR_WEIGHT "w"

typedef struct rgraph {
  art_tree        *uris;        /* map from URIs to vertice IDs */
  char            **vertices;   /* map from vertice IDs to URIs */
  igraph_t        *graph;       /* IGraph representing the triples */
  int             num_vertices; /* number of vertices in trie and graph */
  igraph_vector_t *labels;      /* vector containing edge labels (property node IDs) */
  igraph_vector_t *weights;     /* vector containing edge weights */
} rgraph;


/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph);

/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph);

/**
 * lookup the vertice URI of the vertice with the given ID. NUL in case the ID is smaller 0 or
 * larger than num_vertices
 */
#define rgraph_get_vertice_uri(rgraph, id) (id >= 0 && id < (rgraph)->num_vertices ? (rgraph)->vertices[id] : NULL)

/**
 * lookup the vertice id of the vertice representing the given uri. Returns a pointer to the vertice
 * ID (type int*) if the URI is found or NULL otherwise.
 */
#define rgraph_get_vertice_id(rgraph, uri) art_search(rgraph->uris, uri, strlen(uri))

#endif
