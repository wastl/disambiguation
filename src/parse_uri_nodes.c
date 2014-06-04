#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <raptor2/raptor2.h>

#include "art.h"
#include "parse_uri_nodes.h"


static int nodeid = 0;

void update_trie(rgraph *graph, raptor_term* node) {

  if(node->type == RAPTOR_TERM_TYPE_URI) {    
    size_t len = 0;
    unsigned char* uri = raptor_uri_as_counted_string(node->value.uri , &len);

    if(art_search(graph->uris, uri, len) == NULL) {
      int* data = malloc(sizeof(int));
      *data = graph->num_vertices++;
      art_insert(graph->uris, uri, len, data);
    }
  }
}


void parse_uri_statement_handler(rgraph *graph, const raptor_statement* statement) {
  update_trie(graph, statement->subject);
  update_trie(graph, statement->predicate);
  update_trie(graph, statement->object);

}


/**
 * Parse all URI nodes contained in rdffile and add them to the trie
 * passed as argument. The trie must have already been initialised.
 */
void parse_uri_nodes(rgraph *graph, FILE *rdffile, const char* format, const char* _base_uri) {
  raptor_world  *world  = raptor_new_world();
  raptor_parser *parser = raptor_new_parser(world,format);
  raptor_uri    *base_uri = raptor_new_uri(world, _base_uri);
  
  raptor_parser_set_statement_handler(parser, graph, parse_uri_statement_handler);
  raptor_parser_parse_file_stream(parser, rdffile, NULL, base_uri);
  
  raptor_free_uri(base_uri);
  raptor_free_parser(parser);
  raptor_free_world(world);
}



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
void dump_uri_nodes(rgraph *graph, FILE *dumpfile) {
  art_iter(graph->uris,_dump_callback,dumpfile);
}


/**
 * Restore the trie structure from a dumpfile as CSV of the form id,uri
 */
void restore_uri_nodes(rgraph *graph, FILE *dumpfile) {
  // read line by line until EOF
  char* line    = NULL;
  size_t buf_len    = 0;
  int len = 0;
  char* uri_ptr;
  int*   id;
  

  while((getline(&line,&buf_len,dumpfile) != -1)) {
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

}
