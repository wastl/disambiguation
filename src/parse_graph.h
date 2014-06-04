#ifndef HAVE_PARSE_GRAPH_H
#define HAVE_PARSE_GRAPH_H 1

#include "relatedness.h"


/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_graph(rgraph *graph, FILE* rdffile, const char* format, const char* base_uri);


/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph(rgraph *graph, FILE *dumpfile);


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_graph(rgraph *graph, FILE *dumpfile);


#endif
