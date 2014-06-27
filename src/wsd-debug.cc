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
#define MODE_CLUSTERS 8

using namespace mico::graph;

void usage(char *cmd) {
  printf("Usage: %s -i restorefile [-w] [-e] [-l] [-c]\n", cmd);
  exit(1);
}


int main(int argc, char** argv) {
  int opt, i,j;
  char *ifile = NULL;

  int mode = 0;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:welc")) != -1) {
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
    case 'c':
      mode |= MODE_CLUSTERS;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    rgraph_complete graph;

    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

    if(mode & MODE_WEIGHTS) {
      printf("Weights: \n");

      for(i=0; i< igraph_ecount(graph.graph); i++) {
	printf("%d: %.4f\n",i, graph.weights[i]);
      }

    }

    if(mode & MODE_CLUSTERS) {
      printf("Clusters: \n");

      for(i=0; i< igraph_vcount(graph.graph); i++) {
	printf("%d: {",i, graph.weights[i]);
	for(j=0; j<graph.num_clusters; j++) {
	  if(j+1 < graph.num_clusters) {
	    printf("%d, ",graph.clusters[i][j]);
	  } else {
	    printf("%d",graph.clusters[i][j]);
	  }
	}
	printf("}\n");
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

    if(mode & MODE_LABELS) {
      printf("Hash Keys:\n");
      const char* key; int v;
      kh_foreach(graph.uris,key, v, printf("%s; %d\n", key, v); );

    }

    return 0;
  } else {
    usage(argv[0]);
    return 1;
  }

}


