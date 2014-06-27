
# common static C++ libraries
noinst_LIBRARIES = librelatedness.a
librelatedness_a_SOURCES = relatedness_shortest_path.cc relatedness_dfs.cc relatedness_cluster.cc pqueue.c


bin_PROGRAMS = wsd-relatedness 

# program for computing relatedness values over the graph
wsd_relatedness_SOURCES  = wsd-relatedness.cc
wsd_relatedness_LDADD = ../graph/libgraph.a librelatedness.a

