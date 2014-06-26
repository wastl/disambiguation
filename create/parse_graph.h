// -*- mode: c++; -*-
#ifndef HAVE_PARSE_GRAPH_H
#define HAVE_PARSE_GRAPH_H 1

#include <raptor2/raptor2.h>

#include "../graph/rgraph.h"
#include "../config.h"


namespace mico {
  namespace graph {
    namespace rdf {

      void parse_edge_statement_handler(void *data, raptor_statement* statement);

      class parser {
	friend void parse_edge_statement_handler(void *data, raptor_statement* statement);

	rgraph& graph;

	raptor_world  *world;
	raptor_parser *raptor;
	raptor_uri    *base_uri;

	igraph_vector_t* edges;
	igraph_vector_t* labels;
	igraph_vector_t* weights;

	inline int  update_trie(raptor_term* node);
	inline void update_graph(bool force);

      public:
	/**
	 * Initialise new parser for the given graph, format, and base_uri
	 */
	parser(rgraph& graph, const char* format, const char* base_uri);

	~parser();

	/**
	 * Parse all URI nodes contained in rdffile and add them to the trie
	 * passed as argument. The trie must have already been initialised.
	 */
	void parse(const unsigned char *data, size_t len);

      };
    }
  }
}


#endif
