#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../graph/rgraph.h"

#include "relatedness_base.h"
#include "relatedness_shortest_path.h"



using namespace mico::graph;

void usage(char *cmd) {
  printf("Usage: %s -i fileprefix\n", cmd);
  printf("Options:\n");
  printf("  -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)\n");
  exit(1);
}



int main(int argc, char** argv) {
  int opt;
  char *ifile = NULL;

  // read options from command line
  while( (opt = getopt(argc,argv,"i:")) != -1) {
    switch(opt) {
    case 'i':
      ifile = optarg;
      break;
    default:
      usage(argv[0]);
    }
  }

  if(ifile) {
    mico::graph::rgraph graph;

    // first restore existing dump in case -i is given
    graph.restore_file(ifile);

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

  } else {
    usage(argv[0]);
  }

}
