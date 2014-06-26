// -*- mode: c++; -*-
#ifndef HAVE_RGRAPH_H
#define HAVE_RGRAPH_H 1

#include <iostream>
#include <pthread.h>
#include <string.h>
#include <igraph/igraph.h>
#include "khash.h"
#include "../config.h"


#define ATTR_LABEL "l"
#define ATTR_WEIGHT "w"


KHASH_MAP_INIT_STR(uris, int)

namespace mico {

  namespace relatedness {
    class base;
    class shortest_path;
  }

  namespace graph {

    namespace rdf {
      class parser;
    }



    class rgraph {

    public:
      igraph_t        *graph;       /* IGraph representing the triples */
      char            **vertices;   /* map from vertice IDs to URIs */
      int             num_vertices; /* number of vertices in trie and graph */
      int             num_prefixes; /* number of predefined prefixes */
      igraph_vector_t *weights;     /* vector containing edge weights */
      igraph_vector_t *labels;      /* vector containing edge labels (property node IDs) */

      kh_uris_t       *uris;        /* map from URIs to vertice IDs */

    protected:
      friend class mico::graph::rdf::parser;
      friend class mico::relatedness::shortest_path;

      char            **prefixes;   /* list of commonly used prefixes (for shortening URIs) */

      pthread_rwlock_t mutex_v;      /* vertice mutex */
      pthread_mutex_t  mutex_g;      /* graph mutex  */


    public:

      /**
       * Initialise an empty relatedness graph, ready for being updated.
       */
      rgraph(int reserve_vertices = 0, int reserve_edges = 0);

      /**
       * Destroy all resources claimed by a relatedness graph
       */
      ~rgraph();

      /**
       * Preallocate memory for the given number of vertices. Can be used to
       * avoid excessive reallocation during graph construction in case the
       * number of vertices can be estimated in advance.
       */
      void reserve_vertices(int reserve_vertices);

      /**
       * Preallocate memory for the given number of edges. Can be used to
       * avoid excessive reallocation during graph construction in case the
       * number of vertices can be estimated in advance.
       */
      void reserve_edges(int reserve_edges);


      inline int vertice_count() const { return num_vertices; };

      inline int edge_count() const { return igraph_ecount(graph); };

      /**
       * lookup the vertice URI of the vertice with the given ID. NUL in case the ID is smaller 0 or
       * larger than num_vertices
       */
      inline char* get_vertice_uri(int id) const { 
	return (id >= 0 && id < num_vertices ? vertices[id] : NULL);
      }

      /**
       * lookup the vertice id of the vertice representing the given uri. Returns a pointer to the vertice
       * ID (type int*) if the URI is found or NULL otherwise.
       */
      int get_vertice_id(const char* uri) const;


      /**
       * set the vertice id of the vertice representing the given uri. Overrides any previously existing
       * value. The uri is NOT duplicated, so callers need to make sure the string remains on the heap.
       */
      void set_vertice_id(const char* uri, int vid); 


      /**
      * Add a URI prefix to the list of prefixes. List will be expanded if necessary. The given string
      * will be duplicated.
      */
      void add_prefix(const char* uri);

      /**
       * Check if the given URI has one of the defined prefixes. Returns a pointer to the prefix. If
       * pos is not null, pos will contain the first position in the URI after the prefix.
       */
      char* has_prefix(const char* uri, const char** pos) const;

      
      /**
       * Dump the complete graph data structure to a output stream. Uses rgraph's internal binary
       * format for graph representation.
       */
      void dump_stream(std::ostream& os) const;

      /**
       * Dump the complete graph data structure to a binary file. Uses rgraph's internal binary
       * format for graph representation.
       */
      void dump_file(const char* filename) const;

      
      /**
       * Restore the complete graph data structure from an input stream. Uses rgraph's internal
       * binary format for graph representation.
       */
      void restore_stream(std::istream &is);


      /**
       * Restore the complete graph data structure from a binary file. Uses rgraph's internal
       * binary format for graph representation.
       */
      void restore_file(const char* filename);


      // thread locking for multithreaded access to the graph structures

      /**
       * Lock the whole graph for exclusive thread access
       */
      inline void lock_graph() {
	pthread_mutex_lock(&mutex_g);
      }

      /**
       * Try lock the whole graph for exclusive thread access
       */
      inline bool trylock_graph() {
	return (pthread_mutex_trylock(&mutex_g) == 0);
      }


      /**
       * Unlock the graph previously locked with lock_graph()
       */
      inline void unlock_graph() {
	pthread_mutex_unlock(&mutex_g);
      }

      /**
       * Lock the vertices for read access
       */
      inline void lock_vertices_rd() {
	pthread_rwlock_rdlock(&mutex_v);
      }


      /**
       * Lock the vertices for read and write access
       */
      inline void lock_vertices_rw() {
	pthread_rwlock_wrlock(&mutex_v);
      }

      /**
       * Unlock the vertices.
       */
      inline void unlock_vertices() {
	pthread_rwlock_unlock(&mutex_v);
      }

    };

  }
}


#endif
