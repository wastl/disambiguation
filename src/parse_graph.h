#ifndef HAVE_PARSE_GRAPH_H
#define HAVE_PARSE_GRAPH_H 1

#include "rgraph.h"
#include "config.h"



/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_graph(rgraph *graph, FILE* rdffile, const char* format, const char* base_uri);


#endif
