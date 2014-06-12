#ifndef HAVE_RGRAPH_H
#define HAVE_RGRAPH_H 1


#ifdef USE_THREADS
#include <pthread.h>
#endif


#include <string.h>
#include <igraph/igraph.h>
#include "khash.h"
#include "config.h"


#define ATTR_LABEL "l"
#define ATTR_WEIGHT "w"


KHASH_MAP_INIT_STR(uris, int)

typedef struct rgraph {
  igraph_t        *graph;       /* IGraph representing the triples */
  kh_uris_t       *uris;        /* map from URIs to vertice IDs */
  char            **vertices;   /* map from vertice IDs to URIs */
  char            **prefixes;   /* list of commonly used prefixes (for shortening URIs) */
  int             num_vertices; /* number of vertices in trie and graph */
  int             num_prefixes; /* number of predefined prefixes */
  igraph_vector_t *labels;      /* vector containing edge labels (property node IDs) */
  igraph_vector_t *weights;     /* vector containing edge weights */
#ifdef USE_THREADS
  pthread_rwlock_t mutex_v;      /* vertice mutex */
  pthread_mutex_t  mutex_g;      /* graph mutex  */
#endif
} rgraph;


/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph, int reserve_vertices, int reserve_edges);

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
int rgraph_get_vertice_id(rgraph *graph, const char* uri);


/**
 * set the vertice id of the vertice representing the given uri. Overrides any previously existing
 * value. The uri is NOT duplicated, so callers need to make sure the string remains on the heap.
 */
void rgraph_set_vertice_id(rgraph *graph, const char* uri, int vid); 


/**
 * Add a URI prefix to the list of prefixes. List will be expanded if necessary. The given string
 * will be duplicated.
 */
void rgraph_add_prefix(rgraph *graph, const char* uri);

/**
 * Check if the given URI has one of the defined prefixes. Returns a pointer to the prefix. If
 * pos is not null, pos will contain the first position in the URI after the prefix.
 */
char* rgraph_has_prefix(rgraph *graph, const char* uri, char** pos);

#endif
