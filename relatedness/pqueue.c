#include <stdlib.h>

#include "pqueue.h"

#define weight(q,w,k) w[q[k]]
#define less(q,w,k,j) (weight(q,w,k) > weight(q,w,j))

#ifdef PROFILING
#define __PQINLINE__ __attribute__((noinline))
#else
#define __PQINLINE__ inline
#endif

static __PQINLINE__ void exch(int* queue, int* indexes, int s, int t) {
    int tmp = queue[s];
    queue[s] = queue[t];
    queue[t] = tmp;

    if(indexes) {
      indexes[queue[s]] = s;
      indexes[queue[t]] = t;
    }
}

static __PQINLINE__ void fixUp(int* queue, int* indexes, double* weights, register int k) {  
  while(k > 1 && less(queue,weights,k>>1,k)) {
    exch(queue,indexes,k,k>>1);
    k >>= 1;
  }
}


static __PQINLINE__ void fixDown(int* queue, int* indexes, double* weights, register int k, int N) {
  register int j;
  while(k<<1 <= N) {
    j = k<<1;
    if(j < N && less(queue,weights,j,j+1)) j++;
    if(!less(queue,weights,k,j) ) break;
    exch(queue,indexes,k,j);    
    k = j;    
  }
}

/**
 * Initialise the priority queue using the weights vector given as argument.
 * Callers need to ensure that all indexes stored in the queue are valid indexes in this vector.
 */
void pq_init(pqueue_t* queue, int initial_capacity, double* weights, int* indexes) {
  queue->queue    = malloc(initial_capacity * sizeof(int));
  queue->indexes = indexes;
  queue->weights = weights;
  queue->capacity = initial_capacity;
  queue->size = 0;
}


/**
 * Insert the value v into the priority queue. The priority is determined according to the value in
 * the weights vector using v as index.
 */
void pq_insert(pqueue_t* queue, int v) {
  if(queue->size+1 >= queue->capacity) {
    queue->capacity = 2 * queue->capacity;
    queue->queue = realloc(queue->queue, queue->capacity * sizeof(int));
  }

  queue->queue[ ++queue->size ] = v;
  if(queue->indexes) {
    queue->indexes[v] = queue->size;
  }
  fixUp(queue->queue, queue->indexes, queue->weights, queue->size);
}

/**
 * Notify that the priority of item v has decreased and therefore the heap needs rebalancing.
 */
void pq_decrease(pqueue_t* queue, int v) {
  // first need to find element index of v; we first check if we can look up in queue->indexes,
  // otherwise we do a linear search
  int i;
  if(queue->indexes) {
    i = queue->indexes[v];
  } else {
    for(i=1; i<=queue->size && queue->queue[i] != v; i++);
  }

  if(i <= queue->size && i>0) {
    fixUp(queue->queue, queue->indexes, queue->weights,i);
  }
}


/**
 * Return the first value from the priority queue and reorder according to weights.
 */
int pq_first(pqueue_t* queue) {
  exch(queue->queue,queue->indexes,1,queue->size);
  fixDown(queue->queue, queue->indexes, queue->weights, 1, queue->size-1);
  return queue->queue[queue->size--];
}


/**
 * Return true (1) in case the queue is empty.
 */
int pq_empty(pqueue_t* queue) {
  return queue->size == 0;
}


void pq_clear(pqueue_t* queue) {
  queue->size = 0;
}

void pq_destroy(pqueue_t* queue) {
  free(queue->queue);
  queue->capacity = 0;
}
