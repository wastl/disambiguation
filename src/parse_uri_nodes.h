#ifndef HAVE_PARSE_URI_NODES_H
#define HAVE_PARSE_URI_NODES_H 1

#include "art.h"
#include "relatedness.h"

/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_uri_nodes(rgraph *graph, FILE* rdffile, const char* format, const char* base_uri);


/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_uri_nodes(rgraph *graph, FILE *dumpfile);


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_uri_nodes(rgraph *graph, FILE *dumpfile);


#endif
