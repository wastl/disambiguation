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


static int err;

/**
 * Add a node to the URI->vertice trie in case it is an URI node and does not exist yet.
 * Returns the vertice id of the node, or -1 in case the node is not a URI.
 */
static inline int update_trie(rgraph *graph, raptor_term* node) {
  if(node->type == RAPTOR_TERM_TYPE_URI) {    
    int data;
    size_t len = 0;
    unsigned char* uri = raptor_uri_as_counted_string(node->value.uri , &len);
    char *uric;

    khiter_t k = kh_get(uris,graph->uris,uri);

    // uri not found
    if(k == kh_end(graph->uris)) {
      data = graph->num_vertices++;

      // add new ID to map
      uric = strndup(uri,len);
      k = kh_put(uris, graph->uris, uric, &err);
      kh_val(graph->uris, k) = data;

      // add URI to vertices
      if(data % VINC == 0) {
	graph->vertices = realloc(graph->vertices, (data + VINC) * sizeof(char*));
      }
      graph->vertices[data] = uric;
    } else {
      data = kh_val(graph->uris, k);
    }

    return data;
  } else {
    return -1;
  }
}

/**
 * Check if the batch is full and updating the graph structure is required. Enlarge graph with
 * new vertices and edges if necessary.
 */
void update_graph(graph_data *data) {
  // commit batch of edges to graph
  if(igraph_vector_size(data->edges) >= BUFSIZE) {

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


    igraph_vector_clear(data->edges);
    igraph_vector_clear(data->labels);
    igraph_vector_clear(data->weights);
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
  }

  update_graph(data);
}



/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_graph(rgraph *graph, FILE* rdffile, const char* format, const char* _base_uri) {
  raptor_world  *world  = raptor_new_world();
  raptor_parser *parser = raptor_new_parser(world,format);
  raptor_uri    *base_uri = raptor_new_uri(world, _base_uri);

  // collect all edges in this vector
  igraph_vector_t edges;
  igraph_vector_init(&edges,BUFSIZE);

  // collect edge labels in this vector
  igraph_vector_t labels;
  igraph_vector_init(&labels,BUFSIZE / 2);

  // collect edge weights in this vector
  igraph_vector_t weights;
  igraph_vector_init(&weights,BUFSIZE / 2);

  graph_data data = {graph, &edges, &labels, &weights};

  raptor_parser_set_statement_handler(parser, &data, parse_edge_statement_handler);
  raptor_parser_parse_file_stream(parser, rdffile, NULL, base_uri);

  // add all remaining edges to graph
  update_graph(&data);

  igraph_vector_destroy(&edges);
  igraph_vector_destroy(&labels);
  igraph_vector_destroy(&weights);
  raptor_free_uri(base_uri);
  raptor_free_parser(parser);
  raptor_free_world(world);
}


