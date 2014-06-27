#include "rgraph.h"


namespace mico {
  namespace graph {

    /**
     * Initialise an empty relatedness graph, ready for being updated.
     */
    rgraph::rgraph(int rv, int re) {
      /* turn on attribute handling */
      igraph_i_set_attribute_table(&igraph_cattribute_table);

      uris  = kh_init(uris);
      graph = new igraph_t;

      // init empty graph
      igraph_empty(graph,0,GRAPH_MODE);

      num_vertices = 0;

      // apply initial sizes
      if(rv > 0)
	reserve_vertices(rv);
      
      if(re > 0)
	reserve_edges(re);

      pthread_rwlock_init(&mutex_v,NULL);
      pthread_mutex_init(&mutex_g,NULL);

    }


    void rgraph::reserve_vertices(int reserve_vertices) {
      if(reserve_vertices < num_vertices) {
	fprintf(stderr,"cannot reserve less vertices than those already present");
      } else {
	kh_resize(uris, uris, reserve_vertices);
	igraph_vector_reserve(&graph->os,   reserve_vertices + 1);
	igraph_vector_reserve(&graph->is,   reserve_vertices + 1);

	vertices.reserve(reserve_vertices);

      }
    }


    void rgraph::reserve_edges(int reserve_edges) {
      if(reserve_edges < igraph_ecount(graph)) {
	fprintf(stderr,"cannot reserve less edges than those already present");
      } else {
	labels.reserve(reserve_edges);

	igraph_vector_reserve(&graph->from, reserve_edges);
	igraph_vector_reserve(&graph->to,   reserve_edges);
	igraph_vector_reserve(&graph->oi,   reserve_edges);
	igraph_vector_reserve(&graph->ii,   reserve_edges);
      }
    }


    /**
     * Destroy all resources claimed by a relatedness graph
     */
    rgraph::~rgraph() {
      int i;

      igraph_destroy(graph);
      delete graph;

      // free strings in graph->vertices and graph->uris
      for(i=0; i<num_vertices; i++) {
	free(vertices[i]);
      }


      kh_destroy(uris, uris);

      pthread_rwlock_destroy(&mutex_v);
      pthread_mutex_destroy(&mutex_g);

    }


    /**
     * lookup the vertice id of the vertice representing the given uri. Returns a pointer to the vertice
     * ID (type int*) if the URI is found or NULL otherwise.
     */
    int rgraph::get_vertice_id(const char* uri) const {
      khiter_t k = kh_get(uris,uris,uri);
      if(k == kh_end(uris)) {
	return -1;
      } else {
	return kh_val(uris, k);
      }
    } 



    void rgraph::set_vertice_id(const char* uri, int vid) {
      int err;
      khiter_t k = kh_put(uris, uris, uri, &err);
      kh_val(uris, k) = vid;
    } 





  }
}


