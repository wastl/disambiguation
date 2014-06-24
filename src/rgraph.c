#include <limits.h>
#include <float.h>


#include "rgraph.h"
#include "pqueue.h"

/**
 * Initialise an empty relatedness graph, ready for being updated.
 */
void init_rgraph(rgraph *graph, int reserve_vertices, int reserve_edges) {
  /* turn on attribute handling */
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  graph->uris  = kh_init(uris);
  graph->graph = malloc(sizeof(igraph_t));
  graph->vertices = malloc(2 * sizeof(char*));
  graph->labels = malloc(sizeof(igraph_vector_t));
  graph->weights = malloc(sizeof(igraph_vector_t));
  graph->prefixes = NULL;

  // init empty graph
  igraph_empty(graph->graph,0,GRAPH_MODE);

  graph->num_vertices = 0;
  graph->num_prefixes = 0;

  igraph_vector_init(graph->labels,0);
  igraph_vector_init(graph->weights,0);

  // apply initial sizes
  rgraph_reserve_vertices(graph,reserve_vertices);
  rgraph_reserve_edges(graph,reserve_edges);

#ifdef USE_THREADS
  pthread_rwlock_init(&graph->mutex_v,NULL);
  pthread_mutex_init(&graph->mutex_g,NULL);
#endif

  // helper structures are initialised lazily
  graph->sp_dist = 0;
  graph->sp_len  = 0;
  graph->sp_idx  = 0;
}


void rgraph_reserve_vertices(rgraph *graph, int reserve_vertices) {
  if(reserve_vertices < graph->num_vertices) {
    fprintf(stderr,"cannot reserve less vertices than those already present");
  } else {
    kh_resize(uris, graph->uris, reserve_vertices);
    igraph_vector_reserve(&graph->graph->os,   reserve_vertices + 1);
    igraph_vector_reserve(&graph->graph->is,   reserve_vertices + 1);
  }
}


void rgraph_reserve_edges(rgraph *graph, int reserve_edges) {
  if(reserve_edges < igraph_ecount(graph->graph)) {
    fprintf(stderr,"cannot reserve less edges than those already present");
  } else {
    igraph_vector_reserve(graph->labels, reserve_edges);
    igraph_vector_reserve(graph->weights, reserve_edges);

    igraph_vector_reserve(&graph->graph->from, reserve_edges);
    igraph_vector_reserve(&graph->graph->to,   reserve_edges);
    igraph_vector_reserve(&graph->graph->oi,   reserve_edges);
    igraph_vector_reserve(&graph->graph->ii,   reserve_edges);
  }
}


/**
 * Destroy all resources claimed by a relatedness graph
 */
void destroy_rgraph(rgraph *graph) {
  int i;

  igraph_destroy(graph->graph);
  free(graph->graph);

  // free strings in graph->vertices and graph->uris
  for(i=0; i<graph->num_vertices; i++) {
    free(graph->vertices[i]);
  }

  free(graph->vertices);

  igraph_vector_destroy(graph->labels);
  igraph_vector_destroy(graph->weights);

  free(graph->labels);
  free(graph->weights);

  if(graph->prefixes) {
    free(graph->prefixes);
  }

  kh_destroy(uris, graph->uris);

#ifdef USE_THREADS
  pthread_rwlock_destroy(&graph->mutex_v);
  pthread_mutex_destroy(&graph->mutex_g);
#endif

  if(graph->sp_idx) {
    free(graph->sp_idx);
  }
  if(graph->sp_dist) {
    free(graph->sp_dist);
  }
  if(graph->sp_len) {
    free(graph->sp_len);
  }
}


/**
 * lookup the vertice id of the vertice representing the given uri. Returns a pointer to the vertice
 * ID (type int*) if the URI is found or NULL otherwise.
 */
inline int rgraph_get_vertice_id(rgraph *graph, const char* uri) {
  khiter_t k = kh_get(uris,graph->uris,uri);
  if(k == kh_end(graph->uris)) {
    return -1;
  } else {
    return kh_val(graph->uris, k);
  }
} 



inline void rgraph_set_vertice_id(rgraph *graph, const char* uri, int vid) {
  int err;
  khiter_t k = kh_put(uris, graph->uris, uri, &err);
  kh_val(graph->uris, k) = vid;
} 



/**
 * Add a URI prefix to the list of prefixes. List will be expanded if necessary. The given string
 * will be duplicated.
 */
void rgraph_add_prefix(rgraph *graph, const char* uri) {
  graph->prefixes = realloc( graph->prefixes, (++graph->num_prefixes) * sizeof(char*) );
  graph->prefixes[graph->num_prefixes-1] = strdup(uri);
}

/**
 * Check if the given URI has one of the defined prefixes. Returns a pointer to the prefix. If
 * pos is not null, pos will contain the first position in the URI after the prefix.
 */
char* rgraph_has_prefix(rgraph *graph, const char* uri, char** pos) {
  char *ptr1, *ptr2; int npref = 0;
  for(npref; npref < graph->num_prefixes; npref++) {
    ptr1 = uri; ptr2 = graph->prefixes[npref];
    while(*ptr1 == *ptr2 && *ptr1 && *ptr2) {
      ptr1++;
      ptr2++;
    }
    if(!*ptr2) {
      if(pos) {
	*pos = ptr1;
      }
      return graph->prefixes[npref];
    }
  }
  return NULL;
}





/**
 * Compute shortest path from one entity to the other, following at most max_dist edges
 */
double rgraph_shortest_path(rgraph *graph, const char* sfrom, const char* sto, int max_dist) {
  int i, u, v, x, y;

  int eid;

  double r, alt;

  int from = rgraph_get_vertice_id(graph,sfrom);
  int to   = rgraph_get_vertice_id(graph,sto);

  if(from == -1 || to == -1) {
    return DBL_MAX;
  }

  if(!graph->sp_dist) {
    graph->sp_dist = malloc(graph->num_vertices * sizeof(double));
  }
  if(!graph->sp_idx) {
    graph->sp_idx = malloc(graph->num_vertices * sizeof(int));
  }
  if(!graph->sp_len) {
    graph->sp_len = malloc(graph->num_vertices * sizeof(int));
  }

  double*  dist = graph->sp_dist;
  int*     idx  = graph->sp_idx;
  int*     len  = graph->sp_len;

  pqueue_t queue;
  pq_init(&queue,graph->num_vertices, dist,idx);

  dist[from] = 0.0;
  len[from]  = 0;
  for(i=0; i<graph->num_vertices; i++) {
    if(i != from) {
      dist[i] = DBL_MAX;
      len[i]  = INT_MAX;
    }
    pq_insert(&queue, i);
  }
  
  while(!pq_empty(&queue)) {
    u = pq_first(&queue);

    if(u == to || len[u] > max_dist) {
      break;
    }

    igraph_es_t es;
    igraph_es_incident(&es,u,IGRAPH_ALL);

    igraph_eit_t eit;
    igraph_eit_create(graph->graph, es, &eit);


    while(!IGRAPH_EIT_END(eit)) {
      eid = IGRAPH_EIT_GET(eit);

      igraph_edge(graph->graph, eid, &x, &y); 
      v = x == u ? y : x;
      
      alt = dist[u] + igraph_vector_e(graph->weights,eid);
      if(alt < dist[v] && len[u] + 1 <= max_dist) {
	dist[v] = alt;
	len[v]  = len[u] + 1;
	pq_decrease(&queue,v);
      }

      IGRAPH_EIT_NEXT(eit);
    }

    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
  }

  r = dist[to];

  pq_destroy(&queue);
  //  free(dist);
  //  free(idx);
  //  free(len);

  return r;
}
