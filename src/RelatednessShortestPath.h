#ifndef HAVE_RELATEDNESS_SHORTESTPATH
#define HAVE_RELATEDNESS_SHORTESTPATH 1

#include "RelatednessBase.h"

extern "C" {
#include "pqueue.h"
}

/**
 * An implementation of relatedness using shortest path computation (up to a maximum number of
 * edges) over the knowledge graph.
 */
class RelatednessShortestPath : public virtual RelatednessBase {

private:
  // helper structures (not thread safe!)
  pqueue_t queue;

  double* dist;
  int*    len;
  int*    idx;

public:

  RelatednessShortestPath(rgraph* graph);
  ~RelatednessShortestPath();

  double relatedness(const char* from, const char* to, int max_dist);


};

#endif
