# compile protocol buffers for C
%.pb.cc %.pb.h: %.proto
	$(PROTOC) -I=$(dir $^) --cpp_out=$(dir $^) $^

# common static C libraries
noinst_LIBRARIES = libdisambiguation.a 
libdisambiguation_a_SOURCES = proto-disambiguation.pb.cc disambiguation.cc network.c 

# executable programs
bin_PROGRAMS = wsd-disambiguation

# program for computing the disambiguation problem (C++)
wsd_disambiguation_SOURCES  =  worker.cc wsd-disambiguation.cc 
wsd_disambiguation_LDADD = @threads@ @protobuf_libs@ libdisambiguation.a ../graph/libgraph.a ../relatedness/librelatedness.a


# simple test program for querying the graph
wsd_query_SOURCES  = wsd-query.c 
wsd_query_LDADD = @threads@ ../graph/libgraph.a


noinst_PROGRAMS = wsd-query wsd-debug 
# debug program
wsd_debug_SOURCES  = wsd-debug.c 
wsd_debug_LDADD = @threads@ ../graph/libgraph.a

