#include <stdlib.h>

#include "pqueue.h"

#define weight(q,k) q->weights[q->queue[k]]
#define less(q,k,j) (weight(q,k) > weight(q,j))


inline void exch(pqueue_t* q, int s, int t) {
    int tmp = q->queue[s];
    q->queue[s] = q->queue[t];
    q->queue[t] = tmp;

    if(q->indexes) {
      q->indexes[q->queue[s]] = s;
      q->indexes[q->queue[t]] = t;
    }
}

inline void fixUp(pqueue_t* queue, register int k) {  
  while(k > 1 && less(queue,k/2,k)) {
    exch(queue,k,k/2);
    k = k/2;
  }
}


inline void fixDown(pqueue_t* queue, register int k, int N) {
  register int j;
  while(2*k <= N) {
    j = 2*k;
    if(j < N && less(queue,j,j+1)) j++;
    if(!less(queue,k,j) ) break;
    exch(queue,k,j);    
    k = j;    
  }
}

/**
 * Initialise the priority queue using the weights vector given as argument.
 * Callers need to ensure that all indexes stored in the queue are valid indexes in this vector.
 */
void pq_init(pqueue_t* queue, double* weights, int* indexes) {
  queue->queue    = malloc(INITIAL_CAPACITY * sizeof(int));
  queue->indexes = indexes;
  queue->weights = weights;
  queue->capacity = INITIAL_CAPACITY;
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
  fixUp(queue, queue->size);
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
    fixUp(queue,i);
  }
}


/**
 * Return the first value from the priority queue and reorder according to weights.
 */
int pq_first(pqueue_t* queue) {
  exch(queue,1,queue->size);
  fixDown(queue,1,queue->size-1);
  return queue->queue[queue->size--];
}


/**
 * Return true (1) in case the queue is empty.
 */
int pq_empty(pqueue_t* queue) {
  return queue->size == 0;
}


void pq_destroy(pqueue_t* queue) {
  free(queue->queue);
  queue->capacity = 0;
}
