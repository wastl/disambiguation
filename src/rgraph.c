#include "rgraph.h"

/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph) {
  /* turn on attribute handling */
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  graph->uris  = kh_init(uris);
  graph->graph = malloc(sizeof(igraph_t));
  graph->vertices = malloc(2 * sizeof(char*));
  graph->labels = malloc(sizeof(igraph_vector_t));
  graph->weights = malloc(sizeof(igraph_vector_t));
  graph->prefixes = NULL;

  // init empty graph
  igraph_empty(graph->graph,0,GRAPH_MODE);

  graph->num_vertices = 0;
  graph->num_prefixes = 0;

  igraph_vector_init(graph->labels,0);
  igraph_vector_init(graph->weights,0);

#ifdef USE_THREADS
  pthread_rwlock_init(&graph->mutex_v,NULL);
  pthread_mutex_init(&graph->mutex_g,NULL);
#endif
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

  if(graph->prefixes) {
    free(graph->prefixes);
  }

  kh_destroy(uris, graph->uris);

#ifdef USE_THREADS
  pthread_rwlock_destroy(&graph->mutex_v);
  pthread_mutex_destroy(&graph->mutex_g);
#endif
  
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



/**
 * Add a URI prefix to the list of prefixes. List will be expanded if necessary. The given string
 * will be duplicated.
 */
void rgraph_add_prefix(rgraph *graph, const char* uri) {
  graph->prefixes = realloc( graph->prefixes, (++graph->num_prefixes) * sizeof(char*) );
  graph->prefixes[graph->num_prefixes-1] = strdup(uri);
}

/**
 * Check if the given URI has one of the defined prefixes. Returns a pointer to the prefix. If
 * pos is not null, pos will contain the first position in the URI after the prefix.
 */
char* rgraph_has_prefix(rgraph *graph, const char* uri, char** pos) {
  char *ptr1, *ptr2; int npref = 0;
  for(npref; npref < graph->num_prefixes; npref++) {
    ptr1 = uri; ptr2 = graph->prefixes[npref];
    while(*ptr1 == *ptr2 && *ptr1 && *ptr2) {
      ptr1++;
      ptr2++;
    }
    if(!*ptr2) {
      if(pos) {
	*pos = ptr1;
      }
      return graph->prefixes[npref];
    }
  }
  return NULL;
}
