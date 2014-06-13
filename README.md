# MICO Disambiguation Tools

This repository contains a collection of tools for disambiguation of entities in large RDF graph
structures as found in the Linked Data Cloud. The general Linked Data disambiguation problem is as
follows: given a list of words, each with a list of entity candidates, determine for each word the
candidate that is most likely according to the context. Consider the following example:

"After visiting _Seattle_ we left _Washington_ to visit _Portland_."

For a human reader, it is most likely clear from the context which Seattle, which Washington, and
which Portland the text refers to. An automatic linking tool will, however, create a list of
alternative candidates for each of the words without knowing the correct ones. Disambiguation tries
to select the most likely candidates for each of the words.

The MICO disambiguation tools consist of a number of tools that provide the building blocks for
disambiguation over large graphs. Particular emphasis has been on providing a highly efficient
solution, as the disambiguation problem is computationally expensive and the size of the graphs can
be very large. The implementation is therefore in plain C instead of Java.

The repository currently contains the following tools:

## Training (wsd-create) 

The training tool is used to load a collection of RDF files, create an in-memory representation
of the graph, and compute edge weights for each edge in the graph. The output is written into a set
of binary files that can be used by the other tools for more efficiently working with the data. The
tool can be called from command line using the following options:

    Usage: wsd-create [-f format] [-o outprefix] [-i inprefix] [-p] [-w] [-e num] [-v num] [-t threads] rdffiles...
    Options:
     -f format       the format of the RDF files (turtle,rdfxml,ntriples,trig,json)
     -o outprefix    prefix of the output files to write the result to (e.g. ~/dumps/dbpedia)
     -i inprefix     prefix of the input files to read initial data from
     -t threads      maximum number of threads to use for parallel training (if threading supported)
     -v vertices     estimated number of graph vertices (for improved efficiency)
     -e edges        estimated number of graph edges (for improved efficiency)
     -w              calculate weights before writing result
     -p              print statistics about training when finished


The following call of the training tool would load a DBPedia dump with 8 threads in parallel,
compute edge weights, and write the result to a dump file:

    ./bin/wsd-create -f turtle -o /data/dumps/dbpedia -w -p /data/dbpedia/*.ttl

The graph data will then be stored in the following files in /data/dumps:

    dbpedia.v       vertice mappings from URI to 32bit node ID (CSV)
	dbpedia.e       edge list   (binary, pairs of 32bit from/to node IDs)
	dbpedia.l       edge labels (binary, 32bit predicate node IDs per edge)
	dbpedia.w       edge weights (binary, 64bit double values)

Note that currently, node IDs are represented as 32bit integers, so the maximum number of nodes that
can be handled by the system is 4 billion.


## Relatedness Computation (wsd-relatedness) 

The relatedness tool is used to determine the relatedness between two concepts. It loads a binary
graph representation created by create_graph from harddisk and then computes shortest paths in the
graph using the weights computed by create_graph.

    Usage: wsd-relatedness -i fileprefix [-e edges] [-v vertices]\n", cmd);
    Options:
      -i fileprefix    load the data from the files with the given prefix (e.g. /data/dbpedia)
      -e edges         hint on the number of edges in the graph (can improve startup performance)
      -v vertices      hint on the number of vertices in the graph (improve startup performance)

Once the tool has started up (can take some seconds depending on graph size), it accepts pairs of
concept URIs on standard input, one pair per line, separated by a simple space (" "). For each pair,
it will calculate the relatedness (smaller values are better) and outputs information about which
path it considered the shortest path between the two concepts.

The following call loads a previously created dump file from disk and then accepts input from stdin:

    ./bin/wsd-relatedness -i /data/dumps/dbpedia

A typical interaction with the tool looks as follows:

    > http://dbpedia.org/resource/Berlin http://dbpedia.org/resource/Germany
    computing relatedness for http://dbpedia.org/resource/Berlin and http://dbpedia.org/resource/Germany ... 
    http://dbpedia.org/resource/Germany --- http://dbpedia.org/ontology/capital --> http://dbpedia.org/resource/Berlin
    relatedness = 0.053227

