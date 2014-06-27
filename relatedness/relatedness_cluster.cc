#include "relatedness_cluster.h"

namespace mico {
  namespace relatedness {

    double cluster::relatedness(const char* sfrom, const char* sto) {

      int from = graph->get_vertice_id(sfrom);
      int to   = graph->get_vertice_id(sto);

      if(from == -1 || to == -1 || from >= graph->num_vertices || to >= graph->num_vertices) {
	return 1.0; // not related
      }


      double r = 1.0;
      int d = 2;
      for(int i = graph->num_clusters - 1; i>=0; i--, d <<=1 ) {
	if(graph->clusters[from][i] == graph->clusters[to][i]) {
	  r -= 1.0/d;
	}
      }

      return r;
    }

  }

}
