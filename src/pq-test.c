#include <stdio.h>

#include "pqueue.h"


void main() {
  int i;

  pqueue_t queue;
  double weights[256];
  int    indexes[256];
  for(i=0; i<256; i++) {
    weights[i] = 1.0/ (i+1);
  }

  pq_init(&queue, weights, indexes);

  // insert every second value
  for(i=0; i<128; i++) {
    pq_insert(&queue, i*2);
  }

  while(!pq_empty(&queue)) {
    int n = pq_first(&queue);
    printf("%d(%d): %.4f\n",n, indexes[n], weights[n]);
  }

  pq_destroy(&queue);
}
