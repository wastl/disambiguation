#ifndef HAVE_WEIGHTS_COMBI_H
#define HAVE_WEIGHTS_COMBI_H 1

#include "../graph/rgraph.h"

/**
 * Compute the edge weights according to the combined information content as described in
 * https://ub-madoc.bib.uni-mannheim.de/35464/1/schuhmacher14a.pdf
 *
 */ 
void compute_weights_combi(rgraph *graph);

#endif
