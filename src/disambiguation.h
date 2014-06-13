#ifndef HAVE_DISAMBIGUATION_H
#define HAVE_DISAMBIGUATION_H 1

#include "rgraph.h"

/**
 * Structure for representing a candidate entity for a word.
 */
typedef struct wsd_candidate {
  const char* uri;
  double rank;
} wsd_candidate_t;


/**
 * Structure for representing a term with a list of candidates.
 */
typedef struct wsd_term {
  const char*     term;
  wsd_candidate_t *candidates;
  int             num_candidates;
} wsd_term_t;

/**
 * Solve the disambiguation problem for the given list of
 * terms. Results are written back into the rank field of the
 * wsd_candidate structure.
 */
void disambiguation(rgraph *graph, wsd_term_t *terms, int num_terms, int max_dist);

#endif
