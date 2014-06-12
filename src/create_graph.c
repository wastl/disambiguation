#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifdef USE_THREADS
#include <pthread.h>
#include <semaphore.h>
#endif


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


typedef struct rdffile {
  const char* filename;
  const char* format;
#ifdef USE_THREADS
  sem_t       *thread_s;
#endif
} rdffile;

static rgraph graph;


void usage(char *cmd) {
  printf("Usage: %s [-f format] [-o dumpfile] [-i restorefile] [-p] [-w] rdffile\n", cmd);
}


void process_file(void* data) {
  unsigned char *result;
  unsigned int len;
  clock_t start, end;
  rdffile *attrs = (rdffile*)data;
  int fd;

  struct stat buf;

#ifdef USE_THREADS
  sem_wait(attrs->thread_s);
  printf("parsing RDF file %s ... \n", attrs->filename);
#else
  printf("parsing RDF file %s ... ", attrs->filename);
  fflush(stdout);
#endif

  start = clock();

  // mmap file into memory
  fd = open(attrs->filename,O_RDONLY);
  if (fd < 0) {
    fprintf(stderr,"Error: Unable to read dictionary file %s\n",attrs->filename);
    return;
  }
  if (fstat(fd,&buf) < 0) {
    fprintf(stderr,"Error: Unable to determine file size\n");
    return;
  }
  len = (unsigned int)buf.st_size;
  result = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,fd,0);

  parse_graph(&graph, result, len, attrs->format, "http://localhost/");

  munmap(result,len);
  close(fd);
  end = clock();

#ifdef USE_THREADS
  printf("RDF file %s done (%d ms)!\n", attrs->filename, (end-start) * 1000 / CLOCKS_PER_SEC);
  sem_post(attrs->thread_s);
#else
  printf("done (%d ms)!\n", (end-start) * 1000 / CLOCKS_PER_SEC);
#endif

  free(attrs);
}

void main(int argc, char** argv) {
  int opt, i;
  int mode = 0;
  char *ofile, *ifile;
  char *format = "rdfxml";
  clock_t start, end;
  int reserve_edges = 1<<16;
  int reserve_vertices = 1<<12;

#ifdef USE_THREADS
  pthread_t* threads;
  sem_t      thread_s;

  // use semaphore to make sure not too many threads are working in
  // parallel (poor man's thread pool)
  sem_init(&thread_s,0,NUM_THREADS);
#endif


  // read options from command line
  while( (opt = getopt(argc,argv,"pwf:o:i:e:v:")) != -1) {
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

  if(mode == 0) {
    usage(argv[0]);
    exit(1);
  }

  // init empty graph
  init_rgraph(&graph, reserve_vertices, reserve_edges);

  // first restore existing dump in case -i is given
  if(mode & MODE_RESTORE) { 
    restore_graph(&graph,ifile);
  }

#ifdef USE_THREADS
  int thread_count = (argc-optind);
  threads = malloc ( thread_count * sizeof(pthread_t) );
  i=0;
#endif

  // add the new file(s) to the trie and graph
  for(; optind < argc; optind++) {
    rdffile *attrs = malloc(sizeof(rdffile));
    attrs->filename = argv[optind];
    attrs->format   = format;

#ifndef USE_THREADS
    process_file(attrs);
#else
    attrs->thread_s = &thread_s;
    pthread_create(&threads[i++], NULL, &process_file, attrs);
    
#endif
  }

#ifdef USE_THREADS
  for(i = 0; i<thread_count; i++) {
    pthread_join(threads[i],NULL);
  }
#endif


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

#ifdef USE_THREADS
  pthread_exit(NULL);
#endif
}
