// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_CLUSTER
#define HAVE_RELATEDNESS_CLUSTER 1

#include "relatedness_base.h"

using mico::graph::rgraph_clustered;

namespace mico {

  namespace relatedness {

    /**
     * An implementation of relatedness using a cluster comparison for the two resources. The
     * smaller the clusters the two resources have in common, the stronger the relation.
     */
    class cluster : public virtual base {

      rgraph_clustered* graph;


    public:
      
      /**
       * Initialise a shortest path computation over the given graph up to the given maximum distance.
       */
      cluster(rgraph_clustered* graph, int max_dist) : graph(graph) {};


      /**
       * Relatedness computation via cluster comparison of the two nodes. 
       */
      double relatedness(const char* from, const char* to);


    };
  }
}

#endif
