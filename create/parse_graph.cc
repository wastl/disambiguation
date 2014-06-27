#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <raptor2/raptor2.h>
#include <assert.h>

#include "../graph/rgraph.h"
#include "parse_graph.h"



namespace mico {
  namespace graph {
    namespace rdf {


      /**
      * Statement handler. Adds subject, predicate and object to the URI->id trie in case they are URIs
      * and updates edges vector. In case the edges vector reaches BUFSIZE, the graph is also updated.
      */
      void parse_edge_statement_handler(void *data, raptor_statement* statement) {
	parser* p = (parser*)data;

	int from  = p->update_trie(statement->subject);
	int label = p->update_trie(statement->predicate);
	int to    = p->update_trie(statement->object);



	if(from >= 0 && to >= 0) {
	  igraph_vector_push_back (p->edges, from);
	  igraph_vector_push_back (p->edges, to);
	  p->labels.push_back(label);

	  p->update_graph(false);
	}

      }



      parser::parser(rgraph_complete& graph, const char* format, const char* _base_uri) : graph(graph) {
	world  = raptor_new_world();
	raptor = raptor_new_parser(world,format);
	base_uri = raptor_new_uri(world, (const unsigned char*)_base_uri);

	edges   = new igraph_vector_t;

	igraph_vector_init(edges,BUFSIZE);

	labels.reserve(BUFSIZE);

	raptor_parser_set_statement_handler(raptor, this, parse_edge_statement_handler);
      }

      parser::~parser() {
	igraph_vector_destroy(edges);

	delete edges;

	raptor_free_uri(base_uri);
	raptor_free_parser(raptor);
	raptor_free_world(world);
      }


      /**
       * Add a node to the URI->vertice trie in case it is an URI node and does not exist yet.
       * Returns the vertice id of the node, or -1 in case the node is not a URI.
       */
      inline int parser::update_trie(raptor_term* node) {
	if(node->type == RAPTOR_TERM_TYPE_URI) {    
	  int err, data;
	  khiter_t k;
	  char* uri = (char*)raptor_uri_as_string(node->value.uri);

	  graph.lock_vertices_rd();
	  k = kh_get(uris, graph.uris, uri);

	  // uri not found
	  if(k == kh_end(graph.uris)) {
	    graph.unlock_vertices();
	    uri = (char*)raptor_uri_to_string(node->value.uri);
	    graph.lock_vertices_rw();

	    k = kh_put(uris, graph.uris, uri, &err);

	    // add new ID to map
	    kh_val(graph.uris, k) = graph.num_vertices++;

	    // add URI to vertices
	    graph.vertices.push_back(uri);

	    assert(graph.vertices.size() == graph.num_vertices);
	  }

	  data = kh_val(graph.uris, k);
	  graph.unlock_vertices();

	  return data;

	} else {
	  return -1;
	}
      }

      /**
       * Check if the batch is full and updating the graph structure is required. Enlarge graph with
       * new vertices and edges if necessary.
       */
      inline void parser::update_graph(bool force) {
	// commit batch of edges to graph
	if(igraph_vector_size(edges) >= BUFSIZE || force) {
	  bool have_lock = 0;

	  if(force) {
	    
	    graph.lock_graph();
	    have_lock = true;
	  } else {
	    have_lock = graph.trylock_graph();
	  }

	  if(have_lock) {

	    // check if we need to enlarge the graph
	    if(igraph_vcount(graph.graph) < graph.num_vertices) {
	      igraph_add_vertices(graph.graph, graph.num_vertices - igraph_vcount(graph.graph), 0);
	    }

	    // add edges to graph
	    igraph_add_edges(graph.graph, edges, 0);

	    // add edge labels
	    graph.labels.insert(graph.labels.end(), labels.begin(), labels.end());
    
	    // add edge weights and clusters
	    graph.weights.insert(graph.weights.end(), igraph_ecount(graph.graph) - graph.weights.size(), DBL_MAX);

	    graph.unlock_graph();

	    igraph_vector_clear(edges);
	    labels.clear();
	  }
	}
      }



      /**
       * Parse all URI nodes contained in rdffile and add them to the trie
       * passed as argument. The trie must have already been initialised.
       */
      void parser::parse(const unsigned char *data, size_t len) {
	igraph_vector_clear(edges);
	labels.clear();

	//raptor_parser_parse_iostream(parser, in, base_uri);
	raptor_parser_parse_start(raptor, base_uri); 
	raptor_parser_parse_chunk(raptor, data, len, 1);

	// add all remaining edges to graph
	update_graph(true);
      }


    }
  }
}
