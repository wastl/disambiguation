#ifndef HAVE_RELATEDNESS_H
#define HAVE_RELATEDNESS_H 1

#include <limits.h>
#include <float.h>

#include <igraph/igraph.h>
#include "rgraph.h"

/**
 * Compute relatedness by finding the shortest path between two
 * vertices. Returns the relatedness value (smaller values are
 * stronger relatedness). If the optional pointer to an empty edges
 * vector is given, the vector will afterwards contain a list of edges
 * used in the computation.
 */
double relatedness(rgraph* g, const char* from, const char* to, int max_dist);



#endif
