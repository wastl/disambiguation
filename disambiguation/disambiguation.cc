#include <queue>

#include <limits.h>
#include <float.h>
#include <igraph/igraph.h>

extern "C" {
#include "../config.h"
}

#include "../graph/rgraph.h"
#include "disambiguation.h"
#include "wsd_relatedness_worker.h"
#include "../relatedness/relatedness_base.h"
#include "../relatedness/relatedness_shortest_path.h"
#include "../relatedness/relatedness_dfs.h"
#include "../relatedness/relatedness_cluster.h"

using namespace mico::graph;

inline int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

/** 
 * Compute the degree for each node.
 */
static void compute_degree(igraph_t* graph, igraph_vector_t* weights, igraph_vector_t* results) {
  long int i, j, node, eid;
  double v;
  for(node=0; node<igraph_vcount(graph); node++) {
    v = 0.0;

    j=(long int) VECTOR(graph->os)[node+1];
    for (i=(long int) VECTOR(graph->os)[node]; i<j; i++) {
      eid =  (long int)VECTOR(graph->oi)[i];

      v +=  igraph_vector_e(weights,eid);
    }

    j=(long int) VECTOR(graph->is)[node+1];
    for (i=(long int) VECTOR(graph->is)[node]; i<j; i++) {
      eid = (long int)VECTOR(graph->ii)[i];
    
      v +=  igraph_vector_e(weights,eid);
    }

    igraph_vector_push_back(results,v);

  }
}

void WSDDisambiguationRequest::disambiguation(rgraph_complete *graph) {
  using namespace  mico::disambiguation::wsd;

  int i, j, t, s, c = 0;

  int num_vertices=0;           // how many vertices in total
  int offsets[entities_size()]; // markers to see where in the graph vertice
			        // list the vertices for a term start

#define get_node_label(x,y) entities(x).candidates(y).uri().c_str()
#define get_node_id(x,y)    offsets[x]+y
#define N                   entities_size()
#define N_w(i)              entities(i).candidates_size()

  for(i = 0; i < entities_size(); i++) {
    offsets[i]    = num_vertices;
    num_vertices += entities(i).candidates_size();
  }

  // initialise graph with a vertice for every candidate of every term
  igraph_t wsd_graph;
  igraph_empty(&wsd_graph,num_vertices,IGRAPH_DIRECTED);

  igraph_vector_t wsd_weights;
  igraph_vector_init(&wsd_weights,0);
  

  // 1. build dependency graph (see http://www.cse.unt.edu/~rada/papers/sinha.ieee07.pdf)

  std::cout << "building dependency graph...\n";


  // create thread pool
  relatedness_threadpool_base* pool;

  switch(relatedness()) {
  case SHORTEST_PATH:
    pool = new relatedness_threadpool<mico::relatedness::shortest_path>(graph,wsd_graph,wsd_weights,maxdist());
    break;

  case DFS:
    pool = new relatedness_threadpool<mico::relatedness::dfs>(graph,wsd_graph,wsd_weights,maxdist());
    break;

  case PARTITION:
  default:
    pool = new relatedness_threadpool<mico::relatedness::cluster>(graph,wsd_graph,wsd_weights,maxdist());
    break;
  }
    

  // add tasks to threads in pool (round robin)
  for(i = 0; i < entities_size(); i++) {
    for(j = i+1; j <= i+maxdist() && j < entities_size(); j++) {
      for(t = 0; t < entities(i).candidates_size(); t++) {
	for(s = 0; s < entities(j).candidates_size(); s++) {
	  rtask task = {get_node_label(i,t), get_node_label(j,s), get_node_id(i,t),get_node_id(j,s), 0.0};
	  pool->add_task(task);
	}
      }
    }
  }

  pool->start();
  pool->join();

  // 2. compute centrality for each vertex and write back to
  // candidates
  igraph_vector_t wsd_centralities;
  igraph_vector_init(&wsd_centralities,0);

  igraph_arpack_options_t options;
  igraph_arpack_options_init(&options);

  igraph_vs_t vertice_s;
  igraph_vs_all(&vertice_s);

  igraph_real_t value = 1.0;

  std::cout << "computing centrality scores using algorithm " << centrality() << "...\n";

  // we support a number of different algorithms through igraph ...
  switch(centrality()) {
  case PAGERANK:
    // igraph version <0.7 and >= 0.5
    igraph_pagerank(&wsd_graph, &wsd_centralities, &value, igraph_vss_all(), 0, 0.85, &wsd_weights, &options);
    break;
  case CLOSENESS:
    igraph_closeness(&wsd_graph, &wsd_centralities, igraph_vss_all(), IGRAPH_ALL, &wsd_weights);
    break;
  case BETWEENNESS:
    igraph_betweenness(&wsd_graph, &wsd_centralities, igraph_vss_all(), 0, &wsd_weights, 0);
    break;
  case EIGENVECTOR:
    igraph_eigenvector_centrality(&wsd_graph, &wsd_centralities, NULL, 0, 1, &wsd_weights, &options);
    break;
  case DEGREE:
  default:
    compute_degree(&wsd_graph, &wsd_weights, &wsd_centralities);
    break;
  }  

  std::cout << "writing back disambiguation results...\n";

  // a bit of normalization

  // first normalize to minimum 0.0
  double min = igraph_vector_min(&wsd_centralities);
  igraph_vector_add_constant(&wsd_centralities, -min);

  // then scale to maximum -1.0
  double max = igraph_vector_max(&wsd_centralities);
  igraph_vector_scale(&wsd_centralities, -1.0/max);

  // then apply 1.0-value
  igraph_vector_add_constant(&wsd_centralities, 1.0);

  for(i = 0; i<N; i++) {
    for(j = 0; j<N_w(i); j++) {
      //      mutable_entities(i)->mutable_candidates(j)->set_confidence(1.0-round(ipow(10,PRECISION)*igraph_vector_e(&wsd_centralities,get_node_id(i,j)))/ipow(10,PRECISION));
      mutable_entities(i)->mutable_candidates(j)->set_confidence(igraph_vector_e(&wsd_centralities,get_node_id(i,j)));
    }
  }


  igraph_vector_destroy(&wsd_centralities);
  igraph_vector_destroy(&wsd_weights);
  igraph_destroy(&wsd_graph);
}



/**
 * Operator for writing to standard streams
 */
std::ostream& operator<<(std::ostream& out, const WSDDisambiguationRequest& r) {
  r.SerializeToOstream(&out);
  return out;
}

/**
 * Operator for reading from standard streams
 */
std::istream& operator>>(std::istream& in, WSDDisambiguationRequest& r) {
  r.ParseFromIstream(&in);
  return in;
}
