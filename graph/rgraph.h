// -*- mode: c++; -*-
#ifndef HAVE_RGRAPH_H
#define HAVE_RGRAPH_H 1

#include <iostream>
#include <vector>
#include <pthread.h>
#include <string.h>
#include <igraph/igraph.h>
#include "khash.h"
#include "../config.h"


#define ATTR_LABEL "l"
#define ATTR_WEIGHT "w"


KHASH_MAP_INIT_STR(uris, int)

using namespace std;

namespace mico {

  namespace relatedness {
    class base;
    class shortest_path;
    class dfs;
    class cluster;
  }

  namespace graph {

    namespace rdf {
      class parser;
    }



    class rgraph {

    public:
      igraph_t        *graph;       /* IGraph representing the triples */
      int             num_vertices; /* number of vertices in trie and graph */
      vector<char*>   vertices;     /* map from vertice IDs to URIs */
      vector<int>     labels;       /* vector containing edge labels (property node IDs) */

      kh_uris_t       *uris;        /* map from URIs to vertice IDs */

    protected:
      friend class mico::graph::rdf::parser;
      friend class mico::relatedness::shortest_path;

      pthread_rwlock_t mutex_v;      /* vertice mutex */
      pthread_mutex_t  mutex_g;      /* graph mutex  */

      // override in subclasses in case more data needs to be written to the stream after the
      // initial data has been written
      virtual void dump_stream_hook(std::ostream& os) const {};

      // override in subclasses in case more data needs to be restored from the stream after the
      // initial data has been read
      virtual void restore_stream_hook(std::istream& is) {};

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
      virtual void reserve_vertices(int reserve_vertices);

      /**
       * Preallocate memory for the given number of edges. Can be used to
       * avoid excessive reallocation during graph construction in case the
       * number of vertices can be estimated in advance.
       */
      virtual void reserve_edges(int reserve_edges);


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


    /**
     * A version of an rgraph with edge weights available.
     */
    class rgraph_weighted : public virtual rgraph {

    protected:

      // write weights to stream
      virtual void dump_stream_hook(std::ostream& os) const;

      // read weights from stream
      virtual void restore_stream_hook(std::istream& is);


    public:
      std::vector<double> weights;     /* vector containing edge weights */


      /**
       * Initialise an empty relatedness graph, ready for being updated.
       */
      rgraph_weighted(int reserve_vertices = 0, int reserve_edges = 0)  : rgraph(reserve_vertices, reserve_edges) {};


      /**
       * Preallocate memory for the given number of edges. Can be used to
       * avoid excessive reallocation during graph construction in case the
       * number of vertices can be estimated in advance.
       */
      virtual void reserve_edges(int reserve_edges);

      

    };

    typedef int* cluster_t;

    /**
     * A version of an rgraph where each vertice has an array of hierarchical clusters it is
     * assigned to.
     */ 
    class rgraph_clustered : public virtual rgraph {           

      friend class mico::relatedness::cluster;

    protected:
      

      // write weights to stream
      virtual void dump_stream_hook(std::ostream& os) const;

      // read weights from stream
      virtual void restore_stream_hook(std::istream& is);


    public:
      int num_clusters;

      std::vector<cluster_t> clusters;     /* vector containing clusters */


      /**
       * Initialise an empty relatedness graph, ready for being updated.
       */
      rgraph_clustered(int num_clusters = 8, int reserve_vertices = 0, int reserve_edges = 0)  : rgraph(reserve_vertices, reserve_edges), num_clusters(num_clusters) {};

      /**
       * Destroy all resources claimed by a relatedness graph
       */
      ~rgraph_clustered();

      /**
       * Preallocate memory for the given number of vertices. Can be used to
       * avoid excessive reallocation during graph construction in case the
       * number of vertices can be estimated in advance.
       */
      virtual void reserve_vertices(int reserve_vertices);
      

    };


    /**
     * A complete rgraph with weights and clusters.
     */
    class rgraph_complete : public virtual rgraph_weighted, virtual public rgraph_clustered {

    protected:
      
      // write weights to stream
      virtual void dump_stream_hook(std::ostream& os) const {
	rgraph_weighted::dump_stream_hook(os);
	rgraph_clustered::dump_stream_hook(os);
      };

      // read weights from stream
      virtual void restore_stream_hook(std::istream& is) {
	rgraph_weighted::restore_stream_hook(is);
	rgraph_clustered::restore_stream_hook(is);
      };

    public:
      
      rgraph_complete(int num_clusters = 8, int reserve_vertices = 0, int reserve_edges = 0)  
	: rgraph(reserve_vertices, reserve_edges)
	, rgraph_weighted(reserve_vertices, reserve_edges)
	, rgraph_clustered(num_clusters, reserve_vertices, reserve_edges) {};

      
    };

  }
}


#endif
