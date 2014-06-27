// -*- mode: c++; -*-
#ifndef HAVE_WEIGHTS_COMBI_H
#define HAVE_WEIGHTS_COMBI_H 1

#include "../graph/rgraph.h"

namespace mico {
  namespace graph {
    namespace weights {

      /**
       * Compute the edge weights according to the combined information content as described in
       * https://ub-madoc.bib.uni-mannheim.de/35464/1/schuhmacher14a.pdf
       *
       * - for each predicate (label), we have to determine the relative
       *   selectivity over the whole graph
       * - for each object (target vertice), we have to determine the
       *   relative selectivity over the graph (this is simply the in-degree
       *   divided by the total number of edges)
       */ 
      class rgraph_weights_combi : public rgraph_complete {

      public:

	/**
	 * Initialise an empty relatedness graph, ready for being updated.
	 */
	rgraph_weights_combi(int reserve_vertices = 0, int reserve_edges = 0) : rgraph(reserve_vertices, reserve_edges) {};


	/**
	 * Compute the edge weights.
	 */ 
	void compute_weights();

      };
    }
  }
}

#endif
