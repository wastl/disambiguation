#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

#include "art.h"
#include "rgraph.h"
#include "graphio.h"
#include "parse_graph.h"
#include "weights_combi.h"

#define MODE_PRINT   1
#define MODE_DUMP    2
#define MODE_RESTORE 4
#define MODE_WEIGHTS 8


/*
 * The implementation for loading an RDF graph uses a two-pass scan through the RDF file. In the
 * first pass, we build up a TRIE structure for representing all URI nodes found in the document. In
 * the second pass, we build up an igraph with all vertices and edges. When the datastructures have
 * been created, we compute relatedness values between nodes and save the whole graph to disk.
 */




int print_uri(void *_data, const unsigned char *key, uint32_t key_len, void *value) {
  int* data = (int*)value;
  char uri[key_len+1];
  strncpy(uri,key,key_len);
  uri[key_len]='\0';

  printf("%05d: uri %s\n",*data, uri);
  return 0;
}



void usage(char *cmd) {
  printf("Usage: %s [-f format] [-o dumpfile] [-i restorefile] [-p] [-w] rdffile\n", cmd);
}


void main(int argc, char** argv) {
  int opt;
  int mode = 0;
  char *ofile, *ifile;
  char *format = "rdfxml";
  FILE *f1;
  clock_t start, end;

  rgraph graph;

  // read options from command line
  while( (opt = getopt(argc,argv,"pwf:o:i:")) != -1) {
    switch(opt) {
    case 'o':
      ofile = optarg;
      mode |= MODE_DUMP;
      break;
    case 'i':
      ifile = optarg;
      mode |= MODE_RESTORE;
      break;
    case 'p':
      mode |= MODE_PRINT;
      break;
    case 'w':
      mode |= MODE_WEIGHTS;
      break;
    case 'f':
      format = optarg;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(mode == 0) {
    usage(argv[0]);
    exit(1);
  }

  // init empty graph
  init_rgraph(&graph);

  // first restore existing dump in case -i is given
  if(mode & MODE_RESTORE) { 
    restore_graph(&graph,ifile);
  }

  // add the new file(s) to the trie and graph
  for(; optind < argc; optind++) {
    start = clock();
    printf("parsing RDF file %s ... ", argv[optind]);
    fflush(stdout);
    f1 = fopen(argv[optind],"r");
    parse_graph(&graph, f1, format, "http://localhost/");
    fclose(f1);
    end = clock();
    printf("done (%d ms)!\n", (end-start) * 1000 / CLOCKS_PER_SEC);
  }


  if(mode & MODE_WEIGHTS) { 
    start = clock();
    printf("computing edge weights ... ");
    fflush(stdout);
    compute_weights_combi(&graph);
    end = clock();
    printf("done (%d ms)!\n", (end-start) * 1000 / CLOCKS_PER_SEC);
  }


  if(mode & MODE_DUMP) { 
    dump_graph(&graph,ofile);
  }

		 
  if(mode & MODE_PRINT) {   
    printf("Total number of vertices: %d\n",graph.num_vertices);

    printf("number of vertices: %d\n", igraph_vcount(graph.graph));
    printf("number of edges: %d\n", igraph_ecount(graph.graph));

  }

  destroy_rgraph(&graph);

}
