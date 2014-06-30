/*
 * Relatedness Graph training utility. Reads a number of RDF files into an efficient in-memory graph
 * and calculates relatedness values for edges according to different algorithms. Results are then
 * written out into a set of binary files that can be loaded by the relatedness and disambiguation
 * tools. 
 */

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>


#include "../graph/rgraph.h"
#include "../threading/thread.h"
#include "parse_graph.h"
#include "weights_combi.h"
#include "clustering_metis.h"

#ifdef TIMING
#include <boost/timer/timer.hpp>

using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;
#endif

#define MODE_PRINT   1
#define MODE_DUMP    2
#define MODE_RESTORE 4
#define MODE_WEIGHTS 8
#define MODE_CLUSTERS 16


// internal representation of an RDF file
struct rdffile {
  const char* filename;
  const char* format;
  sem_t       *thread_s;

  rdffile(const char* filename, const char* format, sem_t* thread_s) : filename(filename), format(format), thread_s(thread_s) {};
};


using namespace mico::graph;
using namespace mico::graph::rdf;
using namespace mico::graph::weights;
using namespace mico::graph::clustering;
using namespace mico::threading;



/**
 * Merged class for computing weights and clusters
 */
class rgraph_cw : public rgraph_weights_combi, public rgraph_clustering_metis {
  
};


static rgraph_cw graph;


/**
 * Process a single RDF input file. MMaps the file into memory, then calls parse_graph on the loaded data.
 */
class file_processor : public virtual thread {

  const char* filename;
  const char* format;
  sem_t       *thread_s;

public:

  file_processor(const char* filename, const char* format, sem_t* thread_s) : thread(), filename(filename), format(format), thread_s(thread_s) {};

  void run() {
    unsigned char *result;
    unsigned int len;
    int fd;

    struct stat buf;

    sem_wait(thread_s);
    std::cout << "parsing RDF file " << filename << " ... \n";

#ifdef TIMING
    cpu_timer timer;
#else
    clock_t start, end;
    start = clock();
#endif

    // mmap file into memory
    fd = open(filename,O_RDONLY);
    if (fd < 0) {
      std::cerr << "Error: Unable to read dictionary file " << filename << "\n";
      return;
    }
    if (fstat(fd,&buf) < 0) {
      std::cerr << "Error: Unable to determine file size\n";
      return;
    }
    len = (unsigned int)buf.st_size;
    result = (unsigned char*)mmap(0,len,PROT_READ,MAP_FILE|MAP_PRIVATE,fd,0);

    parser p(graph,format,"http://localhost/");

    p.parse(result, len);

    munmap(result,len);
    close(fd);

#ifdef TIMING
    std::cout << "RDF file " << filename << " done (" << timer.format()  << " )!\n";
#else
    end = clock();
    std::cout << "RDF file " << filename << " done (" << ((end-start) * 1000 / CLOCKS_PER_SEC)  << " ms)!\n";
#endif
    sem_post(thread_s);

  };

};


void usage(char *cmd) {
  printf("Usage: %s [-f format] [-o outprefix] [-i inprefix] [-p] [-w] [-c] [-e num] [-v num] [-t threads] rdffiles...\n", cmd);
  printf("Options:\n");
  printf(" -f format       the format of the RDF files (turtle,rdfxml,ntriples,trig,json)\n");
  printf(" -o outprefix    prefix of the output files to write the result to (e.g. ~/dumps/dbpedia)\n");
  printf(" -i inprefix     prefix of the input files to read initial data from\n");
  printf(" -t threads      maximum number of threads to use for parallel training (if threading supported)\n");
  printf(" -v vertices     estimated number of graph vertices (for improved efficiency)\n");
  printf(" -e edges        estimated number of graph edges (for improved efficiency)\n");
  printf(" -w              calculate weights before writing result\n");
  printf(" -c              calculate clusters before writing result (requires weights)\n");
  printf(" -p              print statistics about training when finished\n");
}


void process_file(void* data) {
}

int main(int argc, char** argv) {
  const char *format = "rdfxml";

  int opt, i;
  int mode = 0;
  char *ofile, *ifile;
  clock_t start, end;
  int reserve_edges = 1<<16;
  int reserve_vertices = 1<<12;
  int num_clusters = 8;

  int num_threads = NUM_THREADS;

  file_processor** threads;    // an array of thread descriptors
  sem_t            thread_s;   // a semaphore for restricting the maximum number of threads running in
                               // parallel (poor man's thread pool)


  // read options from command line
  while( (opt = getopt(argc,argv,"pwc:f:o:i:e:v:t:")) != -1) {
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
    case 'c':
      mode |= MODE_CLUSTERS;
      num_clusters = atoi(optarg);
      break;
    case 'f':
      format = optarg;
      break;
    case 'e':
      reserve_edges = atoi(optarg);
      break;
    case 'v':
      reserve_edges = atoi(optarg);
      break;
    case 't':
      num_threads = atoi(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  if(mode == 0) {
    usage(argv[0]);
    exit(1);
  }

  sem_init(&thread_s,0,num_threads);



  // init empty graph
  graph.reserve_vertices(reserve_vertices);
  graph.reserve_edges(reserve_edges);
  graph.num_clusters = num_clusters;
  

  // 1. restore existing dump in case -i is given
  if(mode & MODE_RESTORE) { 
    graph.restore_file(ifile);
  }

  // 2. add the new RDF files to the graph, using multi-threading if enabled
  int thread_count = (argc-optind);
  threads = new file_processor*[thread_count];
  for(i=0; optind < argc; optind++, i++) {
    threads[i] = new file_processor(argv[optind], format, &thread_s);
    threads[i]->start();
  }

  // wait for all threads to finish computation
  for(i = 0; i<thread_count; i++) {
    threads[i]->join();
  }

  // 3. compute edge weights according to selected algorithm (currently only "combi")
  if(mode & MODE_WEIGHTS) { 
    start = clock();
    std::cout << "computing edge weights ... ";
    std::cout.flush();

    graph.compute_weights();
    end = clock();

    std::cout << "done (" << ((end-start) * 1000 / CLOCKS_PER_SEC) << "ms)!\n";


  }

  if(mode & MODE_CLUSTERS) {
      std::cout << "computing clusters ... ";
    if(mode & MODE_WEIGHTS) {
      std::cout.flush();
      start = clock();
      graph.compute_clusters();
      end = clock();

      std::cout << "done (" << ((end-start) * 1000 / CLOCKS_PER_SEC) << "ms)!\n";

    } else {
      std::cout << "cannot compute clusters without weights\n";
    }
  }


  // 4. write out results to the dump files
  if(mode & MODE_DUMP) { 
    graph.dump_file(ofile);
  }

		 
  // 5. print some statistics about the processed graph
  if(mode & MODE_PRINT) {   
    std::cout << "number of vertices: " << graph.vertice_count() << "\n";
    std::cout << "number of edges: "    << graph.edge_count() << "\n";

  }


  pthread_exit(NULL);

  return 0;
}
