#include <iostream>
#include <fstream>

#include <string.h>
#include <arpa/inet.h>
#include "rgraph.h"


namespace mico {
  namespace graph {


    /**
     * Dump the complete graph data structure to a output stream. Uses rgraph's internal binary
     * format for graph representation.
     */
    void rgraph::dump_stream(std::ostream& os) const {
      int i, v, vh, eid, from, to, len;
      double w;
      khiter_t k;
      const char* key;

      igraph_es_t edge_s;
      igraph_eit_t edge_it;


      // 1. dump information about the number of vertices and edges (used for pre-allocating
      // enough memory on restore and for knowing how much more data to read in the next phases)
      int vc = htonl(num_vertices);
      int ec = htonl(igraph_ecount(graph));

      os.write((char*)&vc, sizeof(int));
      os.write((char*)&ec, sizeof(int));

      // 2. dump vertice uri information; format: 4 bytes node id, 4 bytes uri length, then array of characters until
      // the end-of-string character \0
      std::cout << "- dumping vertice URI data ...\n";
      kh_foreach(uris,key, v, len=strlen(key); os.write((char*)&v,sizeof(int)); os.write((char*)&len,sizeof(int)); os.write(key,len); );



      // 3. dump edge data; format: 4 bytes from node id, 4 bytes to node id
      std::cout << "- dumping edge data ...\n";

      igraph_es_all(&edge_s, IGRAPH_EDGEORDER_ID);
      igraph_eit_create(graph, edge_s, &edge_it);

      while(!IGRAPH_EIT_END(edge_it)) {
	eid = IGRAPH_EIT_GET(edge_it);
    
	igraph_edge(graph, eid, &from, &to);
	os.write((char*)&from, sizeof(int));
	os.write((char*)&to,   sizeof(int));
    
	IGRAPH_EIT_NEXT(edge_it);
      }
  
      igraph_eit_destroy(&edge_it);

      // 4. dump edge label data; format: 4 bytes node id per edge
      std::cout << "- dumping edge label data ...\n";
      for(i=0; i<igraph_ecount(graph); i++) {
	v = igraph_vector_e(labels, i);
	os.write((char*)&v, sizeof(int));
      }

      // 5. dump edge weight data for shortest path
      std::cout << "- dumping weight data (shortest path) ...\n";
      for(i=0; i<igraph_ecount(graph); i++) {
	w = igraph_vector_e(weights, i);
	os.write((char*)&w, sizeof(double));
      }
    }

    /**
     * Dump the complete graph data structure to a binary file. Uses rgraph's internal binary
     * format for graph representation.
     */
    void rgraph::dump_file(const char* filename) const {
      std::ofstream os(filename);
      dump_stream(os);
    }

      
    /**
     * Restore the complete graph data structure from an input stream. Uses rgraph's internal
     * binary format for graph representation.
     */
    void rgraph::restore_stream(std::istream &is) {

      // 1. read in number of vertices and edges, and allocate enough memory for each
      int vc, ec, vcount, ecount;
      is.read((char*)&vc, sizeof(int));
      is.read((char*)&ec, sizeof(int));

      vcount = ntohl(vc);
      ecount = ntohl(ec);

      std::cout << "- dump information: "<<vcount<<" vertices, "<<ecount<<" edges ... \n";

      reserve_vertices(vcount);
      reserve_edges(ecount);

      num_vertices = vcount;

      // set number of vertices in graph
      igraph_add_vertices(graph, num_vertices, 0);

    
      // 2. read vertice URI information, read vcount entries of the form 4 bytes vertice id, 4
      // bytes URI length, then a string representing the uri
      std::cout << "- restoring vertice URI data ... ";
      std::cout.flush();
      
      int i, id, len, err;
      for(i=0; i<vcount; i++) {
	is.read((char*)&id,sizeof(int));
	is.read((char*)&len,sizeof(int));
	
	vertices[i] = (char*)malloc( (len+1) * sizeof(char));
	is.read(vertices[i], len);
	vertices[i][len]='\0';
	kh_put(uris, uris, vertices[i], &err);
      }
      std::cout << i << " URIs!\n";

      // 3. read edge data, read ecount entries of the form 4 bytes fromId, 4 bytes toId
      std::cout << "- restoring edge data ... ";
      std::cout.flush();

      igraph_vector_t edges;
      igraph_vector_init(&edges,0);

      for(i=0; i<ecount; i++) {
	is.read((char*)&id,sizeof(int));
	igraph_vector_push_back(&edges,id);
	
      }
      igraph_add_edges(graph, &edges, 0);

      igraph_vector_destroy(&edges);

      std::cout << igraph_ecount(graph) << " edges!\n";
      
      // 4. restore edge label data of the form 4 bytes label id
      std::cout << "- restoring edge label data ... ";
      std::cout.flush();

      for(i=0; i<ecount; i++) {
	is.read((char*)&id,sizeof(int));
	igraph_vector_push_back(labels,id);	
      }
      std::cout << igraph_vector_size(labels) << " labels!\n";

      // 5. restore weight data of the form 4 bytes double value
      std::cout << "- restoring weight data ... ";
      std::cout.flush();

      double w;
      for(i=0; i<ecount; i++) {
	is.read((char*)&w, sizeof(double));
	igraph_vector_push_back(weights,w);	
      }
      std::cout << igraph_vector_size(weights) << " weights!\n";

      std::cout << "done!\n";
    }
    


    /**
     * Restore the complete graph data structure from a binary file. Uses rgraph's internal
     * binary format for graph representation.
     */
    void rgraph::restore_file(const char* filename) {
      std::ifstream is(filename);
      restore_stream(is);
    }

  }
}
