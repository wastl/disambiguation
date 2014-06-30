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
be very large. The implementation is therefore in plain C++ instead of Java.


## Background

The disambiguation algorithm used by this implementation is based on so-called Word Sense
Disambiguation as described in [this paper](http://www.cse.unt.edu/~rada/papers/sinha.ieee07.pdf) by
Ravi Sinha and Rada Mihalcea (2007). However, where the original paper is concerned with classical
word sense disambiguation using WordNet, the MICO disambiguation tools use much bigger knowledge
graphs like DBPedia or Freebase and therefore have to apply different strategies for efficient
computation.

### Disambiguation Problem

The main task of the MICO disambiguation tools is to solve so-called _disambiguation problems_. A
disambiguation problem typically consists of a (ordered) list of words, each with a set of
suggestions (candidates) of concepts that describe this word. Such disambiguation problems typically
appear in automatic text annotation tools like [Apache Stanbol](http://stanbol.apache.org), because
they are typically based on label matching. Taking the sentence above, the disambiguation problem
(as generated by Apache Stanbol) looks as follows:

<table>
  <tr>
    <td>Entity:</td><td>Seattle</td><td>Washington</td><td>Portland</td>
  </tr>
  <tr>
    <td>Candidates:</td>
	<td>
		dbpedia:Seattle<br/>
		dbpedia:Seattle_metropolitan_area<br/>
		dbpedia:Seattle_Sounders_FC
	</td>
	<td>
		dbpedia:Washington_(state)<br/>
		dbpedia:George_Washington<br/>
		dbpedia:Washington,_D.C.
	</td>
	<td>
		dbpedia:Portland,_Oregon<br/>
		dbpedia:Isle_of_Portland<br/>
		dbpedia:Portland,_Maine<br/>
	</td>
</table>

The disambiguation algorithm tries to provide a ranking for each of the candidates so that the ones
with a better contextual fit are ranked higher than the others.

### Graph-Based Disambiguation

The MICO disambiguation tool is a graph-based approach to disambiguation (i.e. it does not use
machine learning or other statistical approaches). Conceptually, the algorithm builds on two
different graphs:

  * the _knowledge graph_ containing a comprehensive collection of concepts and relations between
    them; this graph can e.g. be based on DBPedia and Freebase and easily contain millions of even
    billions of nodes and edges; this graph is used for computing how closely related two given
    concepts are (using the terminology of Sinha & Mihalcea this would be "Word SenseSemantic Similarity")
  * the _disambiguation graph_ that is dynamically constructed for a given disambiguation problem;
    it is used to compute the relative centrality of each candidate for each entity

For efficiently computing relatedness on large RDF graphs, the knowledge graph can be precomputed
and stored in a more efficient binary format. This is done by the `wsd-create` tool. The
disambiguation graph needs to be dynamically computed for each request. This is handled by the
`wsd-disambiguation` tool.


## Training (wsd-create) 

The training tool is used to load a collection of RDF files, create an in-memory representation
of the graph, and compute edge weights for each edge in the graph as well as graph partitions for
the different relatedness algorithms. The output is written into a set of binary files that can be
used by the other tools for more efficiently working with the data. The tool can be called from
command line using the following options:

    Usage: wsd-create [-f format] [-o outfile] [-i infile] [-p] [-w] [-e num] [-v num] [-t threads] rdffiles...
    Options:
     -f format       the format of the RDF files (turtle,rdfxml,ntriples,trig,json)
     -o outfile      output file to write the result to (e.g. ~/dumps/dbpedia)
     -i infile       input file to read initial data from
     -t threads      maximum number of threads to use for parallel training 
     -v vertices     estimated number of graph vertices (for improved efficiency)
     -e edges        estimated number of graph edges (for improved efficiency)
     -c num          compute clusters before writing results (for relatedness method PARTITION)
     -w              calculate weights before writing result (for all relatedness measures)
     -p              print statistics about training when finished


The following call of the training tool would load a DBPedia dump with 8 threads in parallel,
compute edge weights and clusters to a depth of 16 (equal to 64k clusters), and write the result to a 
dump file:

    ./bin/wsd-create -f turtle -o /data/dumps/dbpedia -w -c 16 -p /data/dbpedia/*.ttl

The graph data will then be stored in /data/dumps/dbpedia using an efficient binary format.
Note that currently, node IDs are represented as 32bit integers, so the maximum number of nodes that
can be handled by the system is 4 billion.



## Disambiguation Server (wsd-disambiguation)

The disambiguation server is the main tool of this project. It loads a binary graph representation
created by create_graph from disk and opens a network socket, listening for incoming connections and
disambiguation requests. Since computing the complete disambiguation problem can be expensive
(depending on the chosen relatedness and centrality algorithms), the server does its best to
parallelize execution. By default, a maximum of 8 threads is started in parallel per request.

### Startup

The server is started from command line and initially loads a graph dump created by the `wsd-create`
tool. It then opens a network socket and listens for incoming disambiguation requests on this socket.

    Usage: wsd-disambiguation -i filename -p port
    Options:
      -i filename      load the data from the given file (e.g. /data/dbpedia)
	  -p port          tcp port to listen on for incoming requests


### Communication Protocol

A disambiguation request is represented using a Google Protobuf description
with client libraries in C++, Java and Python. The abstract protocol is defined as follows:

	message Candidate {
	    required string uri = 1;
	    optional double confidence = 2;
	}

	message Entity {
	    required string text = 1;
	    repeated Candidate candidates = 2;
	}

	message DisambiguationRequest {
	    enum CentralityAlgorithm {
            EIGENVECTOR = 1;
            BETWEENNESS = 2;
            CLOSENESS   = 3;
            PAGERANK    = 4;
	    }

	    enum RelatednessAlgorithm {
            SHORTEST_PATH = 1;
            MAXIMUM_FLOW  = 2;
            PARTITION     = 3;
	    }


	    repeated Entity entities = 1;
	    optional CentralityAlgorithm centrality = 2 [default = EIGENVECTOR];
	    optional RelatednessAlgorithm relatedness = 3 [default = SHORTEST_PATH];
	    optional int32 maxdist = 4;
	}

A disambiguation request typically consists of a list of entities (corresponding to text annotations
in a body of text), each with a list of candidate concepts (identified by their URIs). Once
computation is finished, the server will update the confidence values for each candidate and send
back the whole request to the client for further processing.

A disambiguation request can choose the algorithm to use for disambiguation.
  * the relatedness algorithm defines in which way to compute the relatedness between two concepts
    * SHORTEST_PATH: run a shortest path computation over the indexed graph (expensive!)
	* MAXIMUM_FLOW:  run a maximum flow computation over the indexed graph (expensive!)
	* PARTITION:     use a hierarchical graph partitioning to see how close to concepts are in the
      graph
  * the centrality algorithm defines how to compute confidences for each candidate in the
    disambiguation graph

Currently, only SHORTEST_PATH and PARTITION relatedness are implemented. The EIGENVECTOR centrality is giving the
best results for us.


### Client Libraries




## Relatedness Computation (wsd-relatedness) 

The relatedness tool is a debugging tool used to determine the relatedness between two concepts. It
loads a binary graph representation created by create_graph from harddisk and then computes shortest
paths in the graph using the weights computed by create_graph.

    Usage: wsd-relatedness -i fileprefix [-e edges] [-v vertices]\n", cmd);
    Options:
      -i filename      load the data from the given file (e.g. /data/dbpedia)
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

