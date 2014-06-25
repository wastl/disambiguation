// -*- mode: c++; -*-
#ifndef HAVE_RELATEDNESS_WORKER_H
#define HAVE_RELATEDNESS_WORKER_H 1

#include <vector>
#include <queue>
#include <igraph/igraph.h>

#include "../threading/thread.h"
#include "../relatedness/relatedness_base.h"

extern "C" {
#include "../graph/rgraph.h"
}


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
	rgraph*           graph;         

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

	relatedness_threadpool_base(rgraph* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist);
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


      template <class A> class relatedness_threadpool : public virtual relatedness_threadpool_base {
      private:

	mico::relatedness::base* create_algorithm() {
	  return new A(graph,max_dist);
	};
	

      public:

	relatedness_threadpool(rgraph* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist) 
	  : relatedness_threadpool_base(graph, wsd_graph, wsd_weights, max_dist) {};

      };

    }

  }

}

#endif
