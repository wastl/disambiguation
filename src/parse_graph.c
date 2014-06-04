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


void parse_edge_statement_handler(graph_data *data, const raptor_statement* statement) {

  int from = lookup_node_id(data,statement->subject);
  int to   = lookup_node_id(data,statement->object);

  if(from >= 0 && to >= 0) {
    igraph_vector_push_back (data->edges, from);
    igraph_vector_push_back (data->edges, to);
  }

  // commit batch of edges to graph
  if(igraph_vector_size(data->edges) >= BUFSIZE) {
    igraph_add_edges(data->graph->graph, data->edges, 0);
    igraph_vector_clear(data->edges);
  }
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

  // make graph bigger if needed
  if(igraph_vcount(graph->graph) < graph->num_vertices) {
    igraph_add_vertices(graph->graph, graph->num_vertices - igraph_vcount(graph->graph), 0);
  }


  raptor_parser_set_statement_handler(parser, &data, parse_edge_statement_handler);
  raptor_parser_parse_file_stream(parser, rdffile, NULL, base_uri);

  

  // add all remaining edges to graph
  igraph_add_edges(graph->graph, &edges, 0);

  igraph_vector_destroy(&edges);
  raptor_free_uri(base_uri);
  raptor_free_parser(parser);
  raptor_free_world(world);
}


/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph(rgraph *graph, FILE *dumpfile) {
}


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_graph(rgraph *graph, FILE *dumpfile) {
}
