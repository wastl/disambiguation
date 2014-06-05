#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "art.h"
#include "rgraph.h"
#include "graphio.h"
#include "parse_graph.h"


#define MODE_PRINT   1
#define MODE_DUMP    2
#define MODE_RESTORE 4


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



void usage() {
  printf("Usage: create_graph [-f format] [-o dumpfile] [-p] rdffile\n");
  printf("       create_graph -i dumpfile [-p]\n");
}


void main(int argc, char** argv) {
  int opt;
  int mode = 0;
  char *ofile, *ifile;
  char *format = "rdfxml";
  FILE *f1;

  rgraph graph;

  // read options from command line
  while( (opt = getopt(argc,argv,"pf:o:i:")) != -1) {
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
    case 'f':
      format = optarg;
      break;
    default:
      usage();
    }
  }

  if(mode == 0) {
    usage();
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
    f1 = fopen(argv[optind],"r");
    parse_graph(&graph, f1, format, "http://localhost/");
    fclose(f1);
  }




  if(mode & MODE_DUMP) { 
    dump_graph(&graph,ofile);
  }

		 
  if(mode & MODE_PRINT) {   
    art_iter(graph.uris,print_uri,NULL);
    printf("Total number of vertices: %d\n",graph.num_vertices);

    printf("number of vertices: %d\n", igraph_vcount(graph.graph));
    printf("number of edges: %d\n", igraph_ecount(graph.graph));

  }

  destroy_rgraph(&graph);

}
