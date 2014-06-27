// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_SHORTESTPATH
#define HAVE_RELATEDNESS_SHORTESTPATH 1

#include "relatedness_base.h"

extern "C" {
#include "pqueue.h"
}

namespace mico {

  namespace relatedness {

    /**
     * An implementation of relatedness using shortest path computation (Dijkstra) up to a maximum number of
     * edges over the knowledge graph.
     */
    class shortest_path : public virtual base {

      int max_dist;

      mico::graph::rgraph_weighted* graph;

      // helper structures (not thread safe!)
      pqueue_t queue; // priority queue used by the Dijkstra algorithm

      double* dist;   // vector with current distance used by Dijkstra algorithm
      int*    idx;    // reverse lookup index pointing from vertice ids to queue positions


      void collect(int node, int depth);

    public:
      
      /**
       * Initialise a shortest path computation over the given graph up to the given maximum distance.
       */
      shortest_path(mico::graph::rgraph_weighted* graph, int max_dist);

      /**
       * Cleanup helper structures
       */
      ~shortest_path();

      /**
       * Relatedness computation via shortest path computation in the underlying graph up to a maximum
       * distance for improved performance. Since it will be called very often, this method is heavily
       * optimized for the internal igraph data structures. It uses shared instance data structures, so
       * calling this method on the same instance in multiple threads is not safe.
       */
      double relatedness(const char* from, const char* to);


    };
  }
}

#endif
