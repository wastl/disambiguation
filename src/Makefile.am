bin_PROGRAMS = create_graph relatedness query
create_graph_SOURCES = create_graph.c graphio.c parse_graph.c rgraph.c weights_combi.c
relatedness_SOURCES  = relatedness.c graphio.c rgraph.c
query_SOURCES  = query.c graphio.c rgraph.c
create_graph_LDADD = @rdflibs@ @threads@
relatedness_LDADD = @threads@
query_LDADD = @threads@

