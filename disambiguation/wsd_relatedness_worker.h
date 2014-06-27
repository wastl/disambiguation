// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_WORKER_H
#define HAVE_RELATEDNESS_WORKER_H 1

#include <vector>
#include <queue>
#include <igraph/igraph.h>

#include "../threading/thread.h"
#include "../relatedness/relatedness_base.h"
#include "../graph/rgraph.h"

/**
 * This module implements a multi-threaded computation of relatedness values using a fixed-size
 * thread pool. Since we intend to support different relatedness algorithms, the main threadpool
 * class is a template class with a single method for creating new instances of the relatedness
 * algorithm. In case the standard constructor is not applicable for a certain algorithm, custom
 * subclassing of relatedness_threadpool_base is also possible.
 *
 * Usage:
 *   // create new pool for a given WSD graph and weights vector
 *   relatedness_threadpool<shortest_path> pool(graph,wsd_graph,wsd_weights,maxdist());
 *  
 *   // add relatedness tasks to the shared queue	  
 *   rtask task = {get_node_label(i,t), get_node_label(j,s), get_node_id(i,t),get_node_id(j,s), 0.0};
 *   pool.add_task(task);
 *
 *   // start execution
 *   pool.start();
 *
 *   // wait for completion
 *   pool.join()
 * 
 *   // reset for next executions
 *   pool.reset()
 */
namespace mico {
  namespace disambiguation {

    namespace wsd {

      // internal structure used by RelatednessWorker to represent "jobs"
      struct rtask {
	const char* from;   // the URI from which to start
	const char* to;     // the URI where to end
	int fromId, toId;   // node ids in the disambiguation graph
	double relatedness; // computation result
      };

      class relatedness_threadpool_base;

      /**
       * A relatedness worker is a thread initialised with a number of relatedness computation tasks
       * that are executed in order once the thread is started
       */
      class relatedness_worker : public virtual mico::threading::thread {

      private:
	int id;

	relatedness_threadpool_base* pool;

      public:

	relatedness_worker(int id, relatedness_threadpool_base* pool) 
	  : mico::threading::thread(), id(id), pool(pool) {};


	/**
         * Process all tasks. When finished, write results into the wsd_graph and wsd_weights using
         * a shared lock.
         */
	void run();


      };

      class relatedness_threadpool_base {
	
	// workers can access the pool fields
	friend class relatedness_worker;

      protected:
	bool initialised;

	// the workers used by the pool
	relatedness_worker*      pool[NUM_THREADS];
	
	// the worker states used by the pool
	mico::relatedness::base* states[NUM_THREADS];

	std::queue<rtask>        tasks;

	// the knowledge graph in the backend
	mico::graph::rgraph_complete*   graph;         

	// the WSD data structures
	igraph_t&         wsd_graph;
	igraph_vector_t&  wsd_weights;
	pthread_mutex_t   wsd_mutex;      /* graph mutex  */
	pthread_mutex_t   tsk_mutex;      /* queue mutex  */

	// algorithm configuration
	int max_dist;

	// abstract method for creating the algorithm states
	virtual mico::relatedness::base* create_algorithm() = 0;


      public:

	relatedness_threadpool_base(mico::graph::rgraph_complete* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist);
	~relatedness_threadpool_base();

	// add a relatedness task to the queue
	inline void add_task(rtask t) {
	  tasks.push(t);
	};

	// reset all workers to initial state
	void reset();

	// start all worker threads
	void start();

	// wait for completion of all worker threads
	void join();
      };


      /**
       * Templated relatedness_threadpool for algorithms with a constructor of the form 
       * C(rgraph* g, int max_dist)
       */
      template <class A> class relatedness_threadpool : public virtual relatedness_threadpool_base {
      private:

	mico::relatedness::base* create_algorithm() {
	  return new A(graph,max_dist);
	};
	

      public:

	relatedness_threadpool(mico::graph::rgraph_complete* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist) 
	  : relatedness_threadpool_base(graph, wsd_graph, wsd_weights, max_dist) {};

      };

    }

  }

}

#endif
