#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <raptor2/raptor2.h>

#include "rgraph.h"
#include "parse_graph.h"


typedef struct graph_data {
  rgraph *graph;
  igraph_vector_t *edges;
  igraph_vector_t *labels;
  igraph_vector_t *weights;
} graph_data;


/**
 * Add a node to the URI->vertice trie in case it is an URI node and does not exist yet.
 * Returns the vertice id of the node, or -1 in case the node is not a URI.
 */
static inline int update_trie(rgraph *graph, raptor_term* node) {
  if(node->type == RAPTOR_TERM_TYPE_URI) {    
    int err, data;
    khiter_t k;
    unsigned char* uri = raptor_uri_as_string(node->value.uri);

#ifdef USE_THREADS
    pthread_rwlock_rdlock(&graph->mutex_v);
#endif
    k = kh_get(uris, graph->uris, uri);

    // uri not found
    if(k == kh_end(graph->uris)) {
#ifdef USE_THREADS
      pthread_rwlock_unlock(&graph->mutex_v);
#endif
      uri = raptor_uri_to_string(node->value.uri);
#ifdef USE_THREADS
      pthread_rwlock_wrlock(&graph->mutex_v);
#endif

      k = kh_put(uris, graph->uris, uri, &err);

      // add new ID to map
      kh_val(graph->uris, k) = graph->num_vertices++;

      // add URI to vertices
      if(kh_val(graph->uris, k) % VINC == 0) {
	graph->vertices = realloc(graph->vertices, (kh_val(graph->uris, k) + VINC) * sizeof(char*));
      }
      graph->vertices[kh_val(graph->uris, k)] = uri;
      
    }

    data = kh_val(graph->uris, k);
#ifdef USE_THREADS
    pthread_rwlock_unlock(&graph->mutex_v);
#endif

    return data;

  } else {
    return -1;
  }
}

/**
 * Check if the batch is full and updating the graph structure is required. Enlarge graph with
 * new vertices and edges if necessary.
 */
static inline void update_graph(graph_data *data, int force) {
  // commit batch of edges to graph
  if(igraph_vector_size(data->edges) >= BUFSIZE || force) {
#ifdef USE_THREADS
    int have_lock = 0;

    if(force) {
      pthread_mutex_lock(&data->graph->mutex_g);
      have_lock = 1;
    } else {
      have_lock = (pthread_mutex_trylock(&data->graph->mutex_g) == 0);
    }

    if(have_lock) {
#endif

    // check if we need to enlarge the graph
    if(igraph_vcount(data->graph->graph) < data->graph->num_vertices) {
      igraph_add_vertices(data->graph->graph, data->graph->num_vertices - igraph_vcount(data->graph->graph), 0);
    }

    // add edges to graph
    igraph_add_edges(data->graph->graph, data->edges, 0);

    // add edge labels
    igraph_vector_append(data->graph->labels, data->labels);
    
    // add edge weights
    igraph_vector_append(data->graph->weights, data->weights);

#ifdef USE_THREADS
    pthread_mutex_unlock(&data->graph->mutex_g);
#endif


    igraph_vector_clear(data->edges);
    igraph_vector_clear(data->labels);
    igraph_vector_clear(data->weights);
#ifdef USE_THREADS
    }
#endif
  }
}

/**
 * Statement handler. Adds subject, predicate and object to the URI->id trie in case they are URIs
 * and updates edges vector. In case the edges vector reaches BUFSIZE, the graph is also updated.
 */
void parse_edge_statement_handler(graph_data *data, const raptor_statement* statement) {

  int from  = update_trie(data->graph, statement->subject);
  int label = update_trie(data->graph, statement->predicate);
  int to    = update_trie(data->graph, statement->object);



  if(from >= 0 && to >= 0) {
    igraph_vector_push_back (data->edges, from);
    igraph_vector_push_back (data->edges, to);
    igraph_vector_push_back (data->labels, label);
    igraph_vector_push_back (data->weights, DBL_MAX);

    update_graph(data, 0);
  }

}



/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_graph(rgraph *graph, const char *data, size_t len, const char* format, const char* _base_uri) {
  raptor_world  *world  = raptor_new_world();
  raptor_parser *parser = raptor_new_parser(world,format);
  raptor_uri    *base_uri = raptor_new_uri(world, _base_uri);
  //raptor_iostream *in = raptor_new_iostream_from_string(world,data,len); 

  // collect all edges in this vector
  igraph_vector_t edges;
  igraph_vector_init(&edges,BUFSIZE);

  // collect edge labels in this vector
  igraph_vector_t labels;
  igraph_vector_init(&labels,BUFSIZE / 2);

  // collect edge weights in this vector
  igraph_vector_t weights;
  igraph_vector_init(&weights,BUFSIZE / 2);

  graph_data gdata = {graph, &edges, &labels, &weights};

  raptor_parser_set_statement_handler(parser, &gdata, parse_edge_statement_handler);
  //raptor_parser_parse_iostream(parser, in, base_uri);
  raptor_parser_parse_start(parser, base_uri); 
  raptor_parser_parse_chunk(parser, data, len, 1);

  // add all remaining edges to graph
  update_graph(&gdata, 1);

  igraph_vector_destroy(&edges);
  igraph_vector_destroy(&labels);
  igraph_vector_destroy(&weights);
  //raptor_free_iostream(in);
  raptor_free_uri(base_uri);
  raptor_free_parser(parser);
  raptor_free_world(world);
}


