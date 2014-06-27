#include <iostream>

#include <metis.h>

#include "clustering_metis.h"

namespace mico {
  namespace graph {
    namespace clustering {
      void rgraph_clustering_metis::compute_clusters() {
	long int node, i, j, eid, v;

	igraph_integer_t vcount = igraph_vcount(graph); 
	igraph_integer_t ecount = igraph_ecount(graph); 

	std::cout << "calculating clusters for " << vcount << " vertices and " << ecount << " edges ... \n";

	idx_t options[METIS_NOPTIONS];

	METIS_SetDefaultOptions(options);
	
	// prepare METIS' compressed graph structure ...
	std::cout << "- preparing METIS data structures ... \n";

	idx_t* xadj = new idx_t[num_vertices+1];
	idx_t* adjncy = new idx_t[2*igraph_ecount(graph)];
	idx_t* adjwgt = new idx_t[2*igraph_ecount(graph)];
	idx_t* parts  = new idx_t[num_vertices+1];

	long int adjncy_cur = 0;
	
	// for each node, get adjacent nodes (outgoing and incoming) from igraph
	for(node=0; node<num_vertices; node++) {

	  xadj[node] = adjncy_cur;

	  // outgoing
	  j=(long int) VECTOR(graph->os)[node+1];
	  for (i=(long int) VECTOR(graph->os)[node]; i<j; i++, adjncy_cur++) {
	    eid = (long int)VECTOR(graph->oi)[i];
	    adjncy[adjncy_cur] = VECTOR(graph->to)[eid];
	    adjwgt[adjncy_cur] = 1.0/weights[eid];	    
	  }

	  // incoming
	  j=(long int) VECTOR(graph->is)[node+1];
	  for (i=(long int) VECTOR(graph->is)[node]; i<j; i++, adjncy_cur++) {
	    eid = (long int)VECTOR(graph->ii)[i];
	    adjncy[adjncy_cur] = VECTOR(graph->from)[eid];    
	    adjwgt[adjncy_cur] = 1.0/weights[eid];	    
	  }
	}
	xadj[num_vertices] = adjncy_cur;

	int nparts = 2, ncon = 1, edgecut;
	for(i=0; i<num_clusters; i++, nparts <<= 1) {
	  std::cout << "- computing cluster with " << nparts << " partitions ... \n";
	  if(nparts <= 8) {
	    METIS_PartGraphRecursive(&vcount, &ncon, &xadj[0], &adjncy[0], NULL, NULL, &adjwgt[0], &nparts, NULL, NULL, options, &edgecut, &parts[0]);
	  } else {
	    METIS_PartGraphKway(&vcount, &ncon, &xadj[0], &adjncy[0], NULL, NULL, &adjwgt[0], &nparts, NULL, NULL, options, &edgecut, &parts[0]);
	  }

	  for(j=0; j<num_vertices; j++) {
	    clusters[j][i] = (int)parts[j];
	  }
	}

	delete xadj;
	delete adjncy;
	delete adjwgt;
	delete parts;
      }
    }
  }
}
