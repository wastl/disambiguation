#include <iostream>
#include "rgraph.h"


namespace mico {
  namespace graph {


    void rgraph_weighted::reserve_edges(int re) {
      rgraph::reserve_edges(re);
      weights.reserve(re);
    }


    void rgraph_weighted::dump_stream_hook(std::ostream& os) const {
      // 5. dump edge weight data for shortest path
      std::cout << "- dumping weight data (shortest path) ...\n";
      for(int i=0; i<igraph_ecount(graph); i++) {
	os.write((char*)&weights[i], sizeof(double));
      }
    }


    void rgraph_weighted::restore_stream_hook(std::istream& is) {
      // 5. restore weight data of the form 4 bytes double value
      std::cout << "- restoring weight data ... ";
      std::cout.flush();

      double w;
      for(int i=0; i<igraph_ecount(graph); i++) {
	is.read((char*)&w, sizeof(double));
	weights.push_back(w);
      }
      std::cout << weights.size() << " weights!\n";
    }
  }
}
