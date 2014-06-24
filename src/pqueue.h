#ifndef HAVE_PQUEUE_H
#define HAVE_PQUEUE_H 1

#define INITIAL_CAPACITY 128

/**
 * Heap-based priority queue implementation where the elements are int values but their order is
 * determined by the weight value with the given index.
 *
 * Modified version of the heap structure in "Algorithms in C, Third Edition,"by Robert Sedgewick, Addison-Wesley, 1998.
 */
typedef struct pqueue {
  int *queue;
  int *indexes;
  int size;
  int capacity;
  double* weights;
} pqueue_t;


/**
 * Initialise the priority queue using the weights vector given as argument.
 * Callers need to ensure that all indexes stored in the queue are valid indexes in this vector.
 */
void pq_init(pqueue_t* queue, int initial_capacity, double* weights, int* indexes);


/**
 * Insert the value v into the priority queue. The priority is determined according to the value in
 * the weights vector using v as index.
 */
void pq_insert(pqueue_t* queue, int v);

/**
 * Notify that the priority of item v has decreased and therefore the heap needs rebalancing.
 */
void pq_decrease(pqueue_t* queue, int v);

/**
 * Return the first value from the priority queue and reorder according to weights.
 */
int pq_first(pqueue_t* queue);


/**
 * Return true (1) in case the queue is empty.
 */
int pq_empty(pqueue_t* queue);


/**
 * Clear the priority queue (setting size to 0);
 */
void pq_clear(pqueue_t* queue);

/**
 * Destroy the priority queue
 */
void pq_destroy(pqueue_t* queue);

#endif
