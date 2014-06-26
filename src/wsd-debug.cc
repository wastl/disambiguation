#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <time.h>

#include "../graph/rgraph.h"


#define MODE_WEIGHTS 1
#define MODE_EDGES   2
#define MODE_LABELS  4

using namespace mico::graph;

void usage(char *cmd) {
  printf("Usage: %s -i restorefile [-w] [-e] [-l]\n", cmd);
  exit(1);
}


int main(int argc, char** argv) {
  int opt, i;
  char *ifile = NULL;

  int mode = 0;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:wel")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    case 'w':
      mode |= MODE_WEIGHTS;
      break;
    case 'e':
      mode |= MODE_EDGES;
      break;
    case 'l':
      mode |= MODE_LABELS;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph graph;

    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

    if(mode & MODE_WEIGHTS) {
      printf("Weights: \n");

      for(i=0; i< igraph_ecount(graph.graph); i++) {
	printf("%d: %.4f\n",i, igraph_vector_e(graph.weights, i));
      }

    }

    if(mode & MODE_EDGES) {
      printf("Edges: \n");

      for(i=0; i < igraph_ecount(graph.graph); i++) {
	int from, to;
	igraph_edge(graph.graph, i, &from, &to);
	
	printf("%d: %s --> %s\n",i, graph.vertices[from], graph.vertices[to]);
      }

    }


    return 0;
  } else {
    usage(argv[0]);
    return 1;
  }

}


