#ifndef HAVE_GRAPHIO_H
#define HAVE_GRAHPIO_H 1

#include "rgraph.h"

/**
 * Dump the graph data to a set of files starting with the given prefix
 */
void dump_graph(rgraph *graph, const char* file_prefix);


/**
 * Dump the graph data to the given vertice file, edge file,
 * label file, and weights file
 */
void dump_graph_files(rgraph *graph, FILE *verticefile, FILE *graphfile, FILE* labelfile, FILE* weightsfile);


/**
 * Restore the graph data from a set of files starting with the given prefix
 */
void restore_graph(rgraph *graph, const char* file_prefix);

/**
 * Restore the graph data from the given vertice file, edge file,
 * label file, and weights file
 */
void restore_graph_files(rgraph *graph, FILE *verticefile, FILE *edgefile, FILE *labelfile, FILE *weightsfile);


#endif
