// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_BASE_H
#define HAVE_RELATEDNESS_BASE_H 1

extern "C" {
#include "../graph/rgraph.h"
}

namespace mico {
  namespace relatedness {

    /**
     * Base class for different implementations of relatedness. Subclasses can hold their own thread
     * state for computing relatedness to improve performance for sequential calls; instances are never
     * shared between threads. 
     */
    class base {

    protected:
      rgraph* graph;

      base(rgraph* graph) : graph(graph) {};

    public:

      virtual ~base() {};


      /**
       * Compute the relatedness between the two URIs given as argument. The max_dist parameter is used
       * by some implementations to limit the maximum number of edges to take into account in the
       * knowledge graph.
       */
      virtual double relatedness(const char* from, const char* to) = 0;


    };

  }
}
#endif
