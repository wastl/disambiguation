#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "relatedness_base.h"
#include "relatedness_shortest_path.h"

extern "C" {
#include "../graph/rgraph.h"
#include "../graph/graphio.h"
}

void usage(char *cmd) {
  printf("Usage: %s -i fileprefix [-e edges] [-v vertices]\n", cmd);
  printf("Options:\n");
  printf("  -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)\n");
  printf("  -e edges         hint on the number of edges in the graph (can improve startup performance)\n");
  printf("  -v vertices      hint on the number of vertices in the graph (improve startup performance)\n");
  exit(1);
}


void print_path(rgraph *g, igraph_vector_t *edges) {
  int i, from, to, eid;
  for(i=0; i<igraph_vector_size(edges); i++) {
    eid = igraph_vector_e(edges,i);
    igraph_edge(g->graph, eid, &from, &to);

    const char *s = g->vertices[from];
    const char *p = g->vertices[(int)igraph_vector_e(g->labels,eid)];
    const char *o = g->vertices[to];

    printf("%s --- %s --> %s\n", s, p, o);
  }
}

int main(int argc, char** argv) {
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
    char *from, *to, *send;
    double r;

    mico::relatedness::base* alg_rel = new mico::relatedness::shortest_path(&graph,3);


    printf("> ");
    fflush(stdout);
    while((getline(&line,&len,stdin) != -1)) {
      from = line;
      to   = line;
      while(*to != ' ' && to) {
	to++;
      }
      *to='\0'; to++;
      send = to;
      while(send && *send != '\n') {
	send++;
      }
      *send = '\0';

      printf("computing relatedness for %s and %s ... \n",from,to);
      fflush(stdout);
      r = alg_rel->relatedness(from,to);

      printf("relatedness = %.6f\n",r);

      printf("> ");
      fflush(stdout);
    }

    free(line);

    delete alg_rel;

    destroy_rgraph(&graph);
  } else {
    usage(argv[0]);
  }

}
