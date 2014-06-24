# compile protocol buffers for C
%.pb.cc %.pb.h: %.proto
	$(PROTOC) -I=$(dir $^) --cpp_out=$(dir $^) $^

# common static C libraries
noinst_LIBRARIES = libdisambiguation.a libgraph.a
libdisambiguation_a_SOURCES = proto-disambiguation.pb.cc disambiguation.cc relatedness.c network.c 
libgraph_a_SOURCES = graphio.c rgraph.c

# executable programs
bin_PROGRAMS = wsd-create wsd-relatedness wsd-query wsd-disambiguation

# program for computing the disambiguation problem (C++)
wsd_disambiguation_SOURCES  =  worker.cc wsd-disambiguation.cc 
wsd_disambiguation_LDADD = @threads@ @protobuf_libs@ libdisambiguation.a libgraph.a

# program for creating a (binary) graph representation
wsd_create_SOURCES = wsd-create.c parse_graph.c weights_combi.c
wsd_create_LDADD = @rdflibs@ @threads@ libgraph.a

# program for computing relatedness values over the graph
wsd_relatedness_SOURCES  = wsd-relatedness.c relatedness.c 
wsd_relatedness_LDADD = @threads@ libgraph.a


# simple test program for querying the graph
wsd_query_SOURCES  = wsd-query.c 
wsd_query_LDADD = @threads@ libgraph.a

