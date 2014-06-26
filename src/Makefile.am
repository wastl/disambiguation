# compile protocol buffers for C
%.pb.cc %.pb.h: %.proto
	$(PROTOC) -I=$(dir $^) --cpp_out=$(dir $^) $^



# simple test program for querying the graph
wsd_query_SOURCES  = wsd-query.cc 
wsd_query_LDADD = ../graph/libgraph.a


noinst_PROGRAMS = wsd-query wsd-debug 
# debug program
wsd_debug_SOURCES  = wsd-debug.cc 
wsd_debug_LDADD = ../graph/libgraph.a

