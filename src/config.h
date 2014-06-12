#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H 1

#include <igraph/igraph.h>

/**
 * Vector increment. Enlarge the array of vertices by this amount of
 * pointers when needed
 */
#define VINC 50000

/**
 * Buffer size for edge buffer before adding parsed edges to the
 * graph. Larger values mean more efficient graph extension, lower
 * values more efficient space usage.
 */
#define BUFSIZE 200000


#define NUM_THREADS 8

/**
 * Initially reserve this amount of room for vertices; higher values
 * consume more memory but give better performance 
 */
#define RESERVE_VERTICES 1<<20


/**
 * Initially reserve this amount of room for edges; higher values
 * consume more memory but give better performance 
 */
#define RESERVE_EDGES 1<<24


//#define GRAPH_MODE IGRAPH_UNDIRECTED
#define GRAPH_MODE IGRAPH_DIRECTED

#endif
