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
      vertices = (char**)malloc((rv > 0 ? rv : 2) * sizeof(char*));
      labels = new igraph_vector_t;
      weights = new igraph_vector_t;
      prefixes = NULL;

      // init empty graph
      igraph_empty(graph,0,GRAPH_MODE);

      num_vertices = 0;
      num_prefixes = 0;

      igraph_vector_init(labels,0);
      igraph_vector_init(weights,0);

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

	vertices = (char**)realloc(vertices, reserve_vertices * sizeof(char*));

      }
    }


    void rgraph::reserve_edges(int reserve_edges) {
      if(reserve_edges < igraph_ecount(graph)) {
	fprintf(stderr,"cannot reserve less edges than those already present");
      } else {
	igraph_vector_reserve(labels, reserve_edges);
	igraph_vector_reserve(weights, reserve_edges);

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

      free(vertices);

      igraph_vector_destroy(labels);
      igraph_vector_destroy(weights);

      delete labels;
      delete weights;

      if(prefixes) {
	free(prefixes);
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



    /**
     * Add a URI prefix to the list of prefixes. List will be expanded if necessary. The given string
     * will be duplicated.
     */
    void rgraph::add_prefix(const char* uri) {
      prefixes = (char**)realloc( prefixes, (++num_prefixes) * sizeof(char*) );
      prefixes[num_prefixes-1] = strdup(uri);
    }

    /**
     * Check if the given URI has one of the defined prefixes. Returns a pointer to the prefix. If
     * pos is not null, pos will contain the first position in the URI after the prefix.
     */
    char* rgraph::has_prefix(const char* uri, const char** pos) const {
      const char *ptr1, *ptr2; int npref = 0;
      for(npref; npref < num_prefixes; npref++) {
	ptr1 = uri; ptr2 = prefixes[npref];
	while(*ptr1 == *ptr2 && *ptr1 && *ptr2) {
	  ptr1++;
	  ptr2++;
	}
	if(!*ptr2) {
	  if(pos) {
	    *pos = ptr1;
	  }
	  return prefixes[npref];
	}
      }
      return NULL;
    }


  }
}


