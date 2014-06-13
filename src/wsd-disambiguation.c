#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "rgraph.h"
#include "graphio.h"
#include "relatedness.h"
#include "disambiguation.h"

void usage(char *cmd) {
  printf("Usage: %s -i fileprefix [-e edges] [-v vertices]\n", cmd);
  printf("Options:\n");
  printf("  -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)\n");
  printf("  -e edges         hint on the number of edges in the graph (can improve startup performance)\n");
  printf("  -v vertices      hint on the number of vertices in the graph (improve startup performance)\n");
  exit(1);
}


void print_results(wsd_term_t *terms, int num_terms) {
  // one term per line, then candidate URI followed by rank
  int i, j;
  for(i = 0; i<num_terms; i++) {
    printf("term: %s; candidates:", terms[i].term);
    for(j = 0; j<terms[i].num_candidates; j++) {
      printf(" %s=%.4f", terms[i].candidates[j].uri, terms[i].candidates[j].rank);
    }
    printf("\n");
  }
}


void main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;
  long int reserve_edges = 1<<16;
  long int reserve_vertices = 1<<12;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:e:v:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    case 'e':
      sscanf(optarg,"%ld",&reserve_edges);;
      break;
    case 'v':
      sscanf(optarg,"%ld",&reserve_vertices);;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph graph;

    // init empty graph
    init_rgraph(&graph, reserve_vertices, reserve_edges);

    // first restore existing dump in case -i is given
    restore_graph(&graph,ifile);

    // read from stdin pairs of vertices and compute relatedness
    char *line    = NULL;
    size_t len    = 0;
    char *ptr;
    wsd_term_t *terms = 0;
    char **candidates = 0;
    int num_terms = 0, num_candidates = 0;
    double r;
    int i, j;

    printf("> ");
    fflush(stdout);
    while((getline(&line,&len,stdin) != -1)) {

      if(strcmp("\n",line) == 0) {
	// input finished, start calculation and print results
	printf("computing disambiguation values ...\n");

	disambiguation(&graph, terms, num_terms, 5, ALGO_EIGENVECTOR);
	print_results(terms,num_terms);

	// free all allocated memory (strdup, malloc, realloc)
	for(i = 0; i<num_terms; i++) {
	  free(terms[i].term);
	  for(j=0; j<terms[i].num_candidates; j++) {
	    free(terms[i].candidates[j].uri);
	  }
	  free(terms[i].candidates);
	}
	free(terms);
	num_terms = 0;

	printf("> ");
	fflush(stdout);	
      } else {
	// add a new term per line
	num_candidates = 0;

	// find all candidates, starting with the URI after the first
	// blank
	ptr = line;
	while(*ptr) {
	  ptr++;
	  while(isspace(*ptr) && *ptr) {
	    // erase all space characters and replace them with
	    // end-of-string markers
	    *(ptr++) = '\0';
	    if(!isspace(*ptr) && *ptr) {
	      candidates = realloc( candidates, (++num_candidates) * sizeof(char*));
	      candidates[num_candidates-1] = ptr;
	    }
	  }
	}
      
	// create new term entry
	terms = realloc(terms, ++num_terms * sizeof(wsd_term_t));
	terms[num_terms-1].term = strdup(line); // string now terminated at first space
	terms[num_terms-1].num_candidates = num_candidates;
	terms[num_terms-1].candidates = malloc( num_candidates * sizeof(wsd_candidate_t) );
	for(i = 0; i<num_candidates; i++) {
	  terms[num_terms-1].candidates[i].uri = strdup(candidates[i]);
	}
      }
    }

    free(candidates);
    free(line);

    destroy_rgraph(&graph);
    } else {
    usage(argv[0]);
    }

}
