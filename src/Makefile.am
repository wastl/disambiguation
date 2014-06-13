bin_PROGRAMS = wsd-create wsd-relatedness wsd-query
wsd_create_SOURCES = wsd-create.c graphio.c parse_graph.c rgraph.c weights_combi.c
wsd_relatedness_SOURCES  = wsd-relatedness.c graphio.c rgraph.c
wsd_query_SOURCES  = wsd-query.c graphio.c rgraph.c
wsd_create_LDADD = @rdflibs@ @threads@
wsd_relatedness_LDADD = @threads@
wsd_query_LDADD = @threads@

