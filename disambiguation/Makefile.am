
# executable programs
bin_PROGRAMS = wsd-disambiguation

# program for computing the disambiguation problem (C++)
wsd_disambiguation_SOURCES  =  wsd-disambiguation.cc disambiguation.cc wsd_relatedness_worker.cc
wsd_disambiguation_LDADD = @protobuf_libs@ ../communication/libcommunication.a ../graph/libgraph.a ../relatedness/librelatedness.a ../threading/libthreading.a 


