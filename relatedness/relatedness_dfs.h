// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_DFS
#define HAVE_RELATEDNESS_DFS 1

#include "relatedness_base.h"

extern "C" {
#include "pqueue.h"
}

namespace mico {

  namespace relatedness {

    /**
     * An implementation of relatedness using depth first partial shortes path computation up to a maximum number of
     * edges over the knowledge graph.
     */
    class dfs : public virtual base {

      mico::graph::rgraph_weighted* graph;

      int max_dist;

      double* dist;   // vector with current distance used by DFS algorithm

      void collect(int node, double pweight, int depth);

    public:
      
      /**
       * Initialise a shortest path computation over the given graph up to the given maximum distance.
       */
      dfs(mico::graph::rgraph_weighted* graph, int max_dist);

      /**
       * Cleanup helper structures
       */
      ~dfs();

      /**
       * Relatedness computation via depth first partial shortest path computation in the underlying
       * graph up to a maximum distance for improved performance. Since it will be called very
       * often, this method is heavily optimized for the internal igraph data structures. It uses
       * shared instance data structures, so calling this method on the same instance in multiple
       * threads is not safe.
       */
      double relatedness(const char* from, const char* to);


    };
  }
}

#endif
