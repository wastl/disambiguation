#include <iostream>
#include <vector>

#include <float.h>
#include <limits.h>

#include "../threading/thread.h"
#include "wsd_relatedness_worker.h"

using namespace mico::graph;

namespace mico {
  namespace disambiguation {
    namespace wsd {

      /**
       * Execute worker. While the shared threadpool queue contains more relatedness tasks, take
       * next task, compute relatedness, and update the WSD graph and weights.
       */
      void relatedness_worker::run() {

	// loop until queue of thread pool is empty
	while(1) {

	  // atomic queue access
	  pthread_mutex_lock(&pool->tsk_mutex);
	  if(!pool->tasks.empty()) {
	    // take next task and unlock
	    rtask t = pool->tasks.front();
	    pool->tasks.pop();
	    pthread_mutex_unlock(&pool->tsk_mutex);

	    // compute relatedness
	    t.relatedness = pool->states[id]->relatedness(t.from, t.to);

	    // if relatedness value is relevant, add it to the results
	    if(t.relatedness < DBL_MAX) {
	      pthread_mutex_lock(&pool->wsd_mutex);
	      igraph_add_edge(&pool->wsd_graph,t.fromId,t.toId);
	      igraph_vector_push_back (&pool->wsd_weights, t.relatedness);
	      pthread_mutex_unlock(&pool->wsd_mutex);
	    }

      
	  } else {
	    // unlock and end, no more tasks
	    pthread_mutex_unlock(&pool->tsk_mutex);
	    break;
	  } 
    
	}


      }

      /**
       * Constructor. Initialise instance variables and mutexes.
       */
      relatedness_threadpool_base::relatedness_threadpool_base(rgraph_complete* graph, igraph_t& wsd_graph, igraph_vector_t& wsd_weights, int max_dist) 
	: graph(graph), wsd_graph(wsd_graph), wsd_weights(wsd_weights), max_dist(max_dist) {
	pthread_mutex_init(&wsd_mutex,NULL);
	pthread_mutex_init(&tsk_mutex,NULL);
    
	initialised = false; // indicate threads still need to be initialised

      };

      /**
       * Destructor. If thread pool was initialised, clean it up. Destroy mutexes afterwards.
       */
      relatedness_threadpool_base::~relatedness_threadpool_base()  {
	// destroy threads and states
	if(initialised) {
	  for(int i=0; i<NUM_THREADS; i++) {
	    delete pool[i];
	    delete states[i];
	  }
	}

	pthread_mutex_destroy(&wsd_mutex);
	pthread_mutex_destroy(&tsk_mutex);

      };


      /**
       * Start working on the tasks currently contained in the queue. If necessary, the thread pool
       * is first initialised.
       */
      void relatedness_threadpool_base::start()  {
	if(!initialised) {
	  // create threads and states
	  for(int i=0; i<NUM_THREADS; i++) {
	    states[i] = create_algorithm();
	    pool[i]   = new relatedness_worker(i,this);
	  }
	  initialised = true;
	}

	// destroy threads and states
	for(int i=0; i<NUM_THREADS; i++) {
	  pool[i]->start();
	}

      };

      /**
       * Reset all worker threads in the pool.
       */
      void relatedness_threadpool_base::reset()  {
	// destroy threads and states
	if(initialised) {
	  for(int i=0; i<NUM_THREADS; i++) {
	    pool[i]->reset();
	  }
	}

      };

      /**
       * Wait until all workers have finished processing their tasks.
       */
      void relatedness_threadpool_base::join()  {
	// destroy threads and states
	if(initialised) {
	  for(int i=0; i<NUM_THREADS; i++) {
	    pool[i]->join();
	  }
	}

      };


    }
  }
}
