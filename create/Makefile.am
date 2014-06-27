# executable programs
bin_PROGRAMS = wsd-create 

# program for creating a (binary) graph representation
wsd_create_SOURCES = parse_graph.cc weights_combi.cc clustering_metis.cc wsd-create.cc
wsd_create_LDADD = @rdflibs@ @metislibs@ ../graph/libgraph.a ../threading/libthreading.a
