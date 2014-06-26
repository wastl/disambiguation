// -*- mode: c++; -*-
#ifndef HAVE_THREAD_H
#define HAVE_THREAD_H 1

#include <pthread.h>

namespace mico {
  namespace threading {

    /**
     * Abstract thread base class.
     */
    class thread {
      
      enum tstate { CREATED, RUNNING, CANCELLED, FINISHED };

      tstate state;      

      pthread_t _thread;    // thread descriptor 

      // internal runner to be called by pthread_create
      static void * runner(void *);
      
      // internal handler to be used for cancelled threads
      static void cleaner(void *);

    public:

      thread() : state(CREATED) {};

      /**
       * Start the execution of the thread
       */
      void start();

      /**
       * Interrupt thread execution
       */
      void cancel();

      /**
       * Reset thread state to created so it can be started again. Optionally reset any resource
       * state in subclasses.
       */
      virtual void reset();

      /**
       * Check if the thread is running
       */
      inline bool is_running() { return state == RUNNING; };

      /**
       * Wait in calling thread for this thread's completion.
       */
      void join();

      /**
       * The method executed by the thread. Needs to be overridden by subclasses.
       */
      virtual void run() = 0;

      /**
       * Called when the thread has been manually cancelled. Can be overriden by subclasses.
       */
      virtual void cancelled() {};

      /**
       * Called when the thread has completed its execution. Can be overriden by subclasses.
       */
      virtual void finished() {};

      class exception {
      public:
	const char* msg;

	exception(const char* msg) : msg(msg) {};
      };
    };

  }
}

#endif
