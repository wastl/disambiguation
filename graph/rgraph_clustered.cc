#include <iostream>
#include "rgraph.h"


namespace mico {
  namespace graph {


    rgraph_clustered::~rgraph_clustered() {
      for(int i=0; i<clusters.size(); i++) {
	delete clusters[i];
      }
    }


    void rgraph_clustered::reserve_vertices(int re) {
      rgraph::reserve_vertices(re);
      clusters.reserve(re);
    }



    void rgraph_clustered::dump_stream_hook(std::ostream& os) const {
      // 5. dump cluster array for each vertice
      std::cout << "- dumping cluster data ...\n";
      os.write((char*)&num_clusters, sizeof(int));
      for(int i=0; i<igraph_vcount(graph); i++) {
	os.write((char*)&clusters[i], num_clusters*sizeof(char));
      }
    }


    void rgraph_clustered::restore_stream_hook(std::istream& is) {
      // 5. restore cluster array for each vertice
      std::cout << "- restoring cluster data ... ";
      std::cout.flush();

      is.read((char*)&num_clusters, sizeof(int));

      char* c;
      for(int i=0; i<igraph_vcount(graph); i++) {
	c = new char[num_clusters];
	is.read((char*)c, num_clusters*sizeof(char));
	clusters.push_back(c);
      }
      std::cout << clusters.size() << " entries!\n";
    }
  }
}
