#include <string.h>

#include "graphio.h"

/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph(rgraph *graph, const char* file_prefix) {
  int len = strlen(file_prefix) + 3;
  char *vfile, *gfile, *lfile, *wfile;
  FILE *vin, *gin, *lin, *win;
  vfile = malloc( len * sizeof(char));
  gfile = malloc( len * sizeof(char));
  lfile = malloc( len * sizeof(char));
  wfile = malloc( len  * sizeof(char));

  snprintf(vfile, len, "%s.v", file_prefix);
  snprintf(gfile, len, "%s.e", file_prefix);
  snprintf(lfile, len, "%s.l", file_prefix);
  snprintf(wfile, len, "%s.w", file_prefix);

  printf("dumping graph data to %s.[v|g|l|w] ...\n",file_prefix);

  vin = fopen(vfile, "w");
  gin = fopen(gfile, "w");
  lin = fopen(lfile, "w");
  win = fopen(wfile, "w");

  dump_graph_files(graph,vin,gin,lin,win);

  fclose(vin);
  fclose(gin);
  fclose(lin);
  fclose(win);

  free(vfile);
  free(gfile);
  free(lfile);
  free(wfile);
}

/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph_files(rgraph *graph, FILE *verticefile, FILE *graphfile, FILE* labelfile, FILE* weightsfile) {
  int i, v;
  double w;
  khiter_t k;
  const char* key;

  printf("- dumping vertice data ...\n");
  kh_foreach(graph->uris,key,v, fprintf(verticefile,"%d,%s\n",v, key) )


  printf("- dumping edge data ...\n");
  igraph_write_graph_edgelist(graph->graph, graphfile);

  printf("- dumping label data ...\n");
  for(i=0; i<igraph_ecount(graph->graph); i++) {
    v = igraph_vector_e(graph->labels, i);
    //v = igraph_cattribute_EAN(graph->graph, ATTR_LABEL, i);
    fwrite(&v, sizeof(int), 1, labelfile);
    //fprintf(labelfile,"%d\n",v);
  }
  fflush(labelfile);

  printf("- dumping weight data ...\n");
  for(i=0; i<igraph_ecount(graph->graph); i++) {
    w = igraph_vector_e(graph->weights, i);
    //w = igraph_cattribute_EAN(graph->graph, ATTR_WEIGHT, i);
    fwrite(&w, sizeof(double), 1, weightsfile);
  }
  fflush(weightsfile);
}



/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void restore_graph(rgraph *graph, const char* file_prefix) {
  int len = strlen(file_prefix) + 3;
  char *vfile, *gfile, *lfile, *wfile;
  FILE *vin, *gin, *lin, *win;
  vfile = malloc( len * sizeof(char));
  gfile = malloc( len * sizeof(char));
  lfile = malloc( len * sizeof(char));
  wfile = malloc( len * sizeof(char));

  snprintf(vfile, len, "%s.v", file_prefix);
  snprintf(gfile, len, "%s.e", file_prefix);
  snprintf(lfile, len, "%s.l", file_prefix);
  snprintf(wfile, len, "%s.w", file_prefix);

  printf("restoring graph data from %s.[v|g|l|w] ...\n",file_prefix);

  vin = fopen(vfile, "r");
  gin = fopen(gfile, "r");
  lin = fopen(lfile, "r");
  win = fopen(wfile, "r");

  restore_graph_files(graph,vin,gin,lin,win);

  if(vin) fclose(vin);
  if(gin) fclose(gin);
  if(lin) fclose(lin);
  if(win) fclose(win);

  free(vfile);
  free(gfile);
  free(lfile);
  free(wfile);
}


void restore_vertice_data(rgraph *graph, FILE* verticefile) {
  khiter_t k;
  char *uri_ptr, *uri;
  int len = 0;
  int id;
  char* line    = NULL;
  size_t buf_len    = 0;
  int err;

  if(verticefile) {
    printf("- restoring vertice data ... ");
    fflush(stdout);

    while((getline(&line,&buf_len,verticefile) != -1)) {

      // find first comma and copy uri
      uri_ptr = strchr(line,',');
      if(uri_ptr) {
	len = strlen(uri_ptr+1);
	*(uri_ptr+len) = '\0';         // replace newline with end-of-string

	uri = strndup(uri_ptr+1, len);
	k = kh_put(uris, graph->uris, uri, &err);

	// read the integer vertice id
	sscanf(line,"%d",&(kh_val(graph->uris, k)));

	graph->num_vertices++;
      }
    }
    // restore ids by using a callback
    graph->vertices = realloc(graph->vertices, (graph->num_vertices / VINC + 1) * VINC * sizeof(char*));
    kh_foreach(graph->uris, uri, id, graph->vertices[id] = uri )  

    printf("%d vertices!\n",graph->num_vertices);
  } else {
    printf("- not restoring vertice data, file does not exist!\n");
  }

  if(line) {
    free(line);
  }
}


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_graph_files(rgraph *graph, FILE *verticefile, FILE *graphfile, FILE *labelfile, FILE *weightsfile) {
  int  id;
  int i;
  double d;

  restore_vertice_data(graph, verticefile);

  if(graphfile) {
    printf("- restoring edge data ... ");
    fflush(stdout);
    igraph_read_graph_edgelist(graph->graph, graphfile, 0, GRAPH_MODE);
    printf("%d edges!\n",igraph_ecount(graph->graph));
  } else {
    printf("- not restoring edge data, file does not exist!\n");
  }

  if(labelfile) {
    printf("- restoring label data ... ");
    fflush(stdout);

    while( fread(&id, sizeof(int), 1, labelfile) > 0) {
      igraph_vector_push_back(graph->labels,id);
    }
    printf("%d labels!\n",igraph_vector_size(graph->labels));
  } else {
    printf("- not restoring label data, file does not exist!\n");
  }

  if(weightsfile) {
    printf("- restoring weight data ... ");
    fflush(stdout);

    while( fread(&d, sizeof(double), 1, weightsfile) > 0) {
      igraph_vector_push_back(graph->weights,d);
    }
    printf("%d weights!\n",igraph_vector_size(graph->weights));
  } else {
    printf("- not restoring weight data, file does not exist!\n");
  }
}
