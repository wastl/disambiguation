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
     * An implementation of relatedness using shortest path computation (up to a maximum number of
     * edges) over the knowledge graph.
     */
    class shortest_path : public virtual base {

    private:
      // helper structures (not thread safe!)
      pqueue_t queue;

      double* dist;
      int*    len;
      int*    idx;

    public:

      shortest_path(rgraph* graph);
      ~shortest_path();

      double relatedness(const char* from, const char* to, int max_dist);


    };
  }
}

#endif