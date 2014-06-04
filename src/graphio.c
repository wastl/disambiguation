#include <string.h>

#include "art.h"
#include "graphio.h"

int _dump_callback(void *_data, const unsigned char *key, uint32_t key_len, void *value) {
  FILE *fout = (FILE*)_data;
  int* data = (int*)value;
  char uri[key_len+1];
  strncpy(uri,key,key_len);
  uri[key_len]='\0';

  fprintf(fout,"%d,%s\n",*data, uri);
  return 0;

}


/**
 * Dump the trie structure into a dumpfile as CSV of the form id,uri
 */
void dump_graph(rgraph *graph, FILE *verticefile, FILE *graphfile) {
  printf("dumping vertice mappings ...\n");
  art_iter(graph->uris,_dump_callback,verticefile);

  printf("dumping graph data ...\n");
  igraph_write_graph_edgelist(graph->graph, graphfile);
}



int _restore_callback(void *_data, const unsigned char *key, uint32_t key_len, void *value) {
  rgraph *graph = (rgraph*)_data;
  int    *id    = (int*)value;
  graph->vertices[*id] = malloc( (key_len+1) * sizeof(char) );
  memcpy(graph->vertices[*id], key, key_len);
  graph->vertices[*id][key_len] = '\0';
}

/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_graph(rgraph *graph, FILE *verticefile, FILE *graphfile) {
  // read line by line until EOF
  char* line    = NULL;
  size_t buf_len    = 0;
  int len = 0;
  char* uri_ptr;
  int*   id;
  
  printf("restoring vertice mappings ...\n");

  while((getline(&line,&buf_len,verticefile) != -1)) {
    // find first comma and copy uri
    uri_ptr = line;
    while(*uri_ptr && *uri_ptr != ',') {
      uri_ptr++;
    }
    if(*uri_ptr == ',') {
      id = malloc(sizeof(int));
      sscanf(line,"%d",id);
      len = strlen(uri_ptr+1);
      *(uri_ptr+len) = '\0';
      art_insert(graph->uris, uri_ptr+1, len, id);
      graph->num_vertices++;
    }
  }

  // restore ids by using a callback
  graph->vertices = realloc(graph->vertices, (graph->num_vertices / VINC + 1) * VINC * sizeof(char*));
  art_iter(graph->uris,_restore_callback,graph);

  printf("restoring graph data ...\n");
  igraph_read_graph_edgelist(graph->graph, graphfile, 0, 1);
}
