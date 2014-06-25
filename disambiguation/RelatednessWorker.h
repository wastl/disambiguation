#ifndef HAVE_RELATEDNESS_WORKER_H
#define HAVE_RELATEDNESS_WORKER_H 1


// internal structure used by RelatednessWorker to represent "jobs"
struct RelatednessTask {
  const char* from;   // the URI from which to start
  const char* to;     // the URI where to end
  int fromId, toId;   // node ids in the disambiguation graph
  int max_dist;       // maximum distance for relatedness computation
  double relatedness; // computation result
};


/**
 *
 */
class RelatednessWorker {

private:

  RelatednessBase& relatedness;

  std::vector<RelatednessTask> tasks;

public:

#ifdef USE_THREADS
  pthread_t thread;    // thread descriptor in case threading is enabled
#endif

  RelatednessWorker(RelatednessBase& base) : relatedness(base) {};

  void addTask(RelatednessTask* t);

  


};

#endif
