// -*- mode: c++; -*-
#ifndef HAVE_CLUSTERING_METIS_H
#define HAVE_CLUSTERING_METIS_H 1

#include "../graph/rgraph.h"

namespace mico {
  namespace graph {
    namespace clustering {

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
      class rgraph_clustering_metis : public virtual rgraph_complete {

      public:

	/**
	 * Initialise an empty relatedness graph, ready for being updated.
	 */
	rgraph_clustering_metis(int num_clusters = 8, int reserve_vertices = 0, int reserve_edges = 0) : rgraph_complete(num_clusters, reserve_vertices, reserve_edges) {};



	/**
	 * Compute hierarchical clusters and assign vertices.
	 */
	void compute_clusters();

      };
    }
  }
}

#endif
