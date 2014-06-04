#ifndef HAVE_GRAPHIO_H
#define HAVE_GRAHPIO_H 1

#include "relatedness.h"

/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph(rgraph *graph, FILE *verticefile, FILE *graphfile);


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_graph(rgraph *graph, FILE *verticefile, FILE *graphfile);


#endif
