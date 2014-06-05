#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H 1

#include <igraph/igraph.h>

/**
 * Vector increment. Enlarge the array of vertices by this amount of
 * pointers when needed
 */
#define VINC 1000

/**
 * Buffer size for edge buffer before adding parsed edges to the
 * graph. Larger values mean more efficient graph extension, lower
 * values more efficient space usage.
 */
#define BUFSIZE 1000



//#define GRAPH_MODE IGRAPH_UNDIRECTED
#define GRAPH_MODE IGRAPH_DIRECTED

#endif
