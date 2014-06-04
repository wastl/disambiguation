#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <raptor2/raptor2.h>

#include "art.h"
#include "relatedness.h"
#include "parse_graph.h"

#define BUFSIZE 100

typedef struct graph_data {
  rgraph *graph;
  igraph_vector_t *edges;
} graph_data;


int lookup_node_id(graph_data *gdata, raptor_term *node) {
  if(node->type == RAPTOR_TERM_TYPE_URI) {    
    size_t len = 0;
    unsigned char* uri = raptor_uri_as_counted_string(node->value.uri , &len);

    int* data = art_search(gdata->graph->uris, uri, len);
    if(data) {
      return *data;
    }
  }

  return -1;
}


int update_trie(rgraph *graph, raptor_term* node) {
  if(node->type == RAPTOR_TERM_TYPE_URI) {    
    size_t len = 0;
    unsigned char* uri = raptor_uri_as_counted_string(node->value.uri , &len);

    int* data = art_search(graph->uris, uri, len);

    if(!data) {
      data = malloc(sizeof(int));
      *data = graph->num_vertices++;
      art_insert(graph->uris, uri, len, data);
    }

    return *data;
  } else {
    return -1;
  }
}

void update_graph(graph_data *data) {
  // commit batch of edges to graph
  if(igraph_vector_size(data->edges) >= BUFSIZE) {
    // check if we need to enlarge the graph
    if(igraph_vcount(data->graph->graph) < data->graph->num_vertices) {
      igraph_add_vertices(data->graph->graph, data->graph->num_vertices - igraph_vcount(data->graph->graph), 0);
    }

    // add edges to graph
    igraph_add_edges(data->graph->graph, data->edges, 0);
    igraph_vector_clear(data->edges);
  }
}

/**
 * Statement handler. Adds subject, predicate and object to the URI->id trie in case they are URIs
 * and updates edges vector. In case the edges vector reaches BUFSIZE, the graph is also updated.
 */
void parse_edge_statement_handler(graph_data *data, const raptor_statement* statement) {

  int from = update_trie(data->graph, statement->subject);
  int attr = update_trie(data->graph, statement->predicate);
  int to   = update_trie(data->graph, statement->object);

  if(from >= 0 && to >= 0) {
    igraph_vector_push_back (data->edges, from);
    igraph_vector_push_back (data->edges, to);
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

  graph_data data = {graph, &edges};

  raptor_parser_set_statement_handler(parser, &data, parse_edge_statement_handler);
  raptor_parser_parse_file_stream(parser, rdffile, NULL, base_uri);

  // add all remaining edges to graph
  update_graph(&data);

  igraph_vector_destroy(&edges);
  raptor_free_uri(base_uri);
  raptor_free_parser(parser);
  raptor_free_world(world);
}


