#include <ext/stdio_filebuf.h>
#include <iostream>

#include "connection.h"

extern "C" {
#include "network.h"
}

namespace mico {
  namespace network {

    using namespace std;
    using namespace __gnu_cxx;
    using namespace mico::graph;

    /**
     * Create a connection for sending/receiving requests from standard input/output
     */
    base_connection::base_connection(rgraph *graph) : conn(0), graph(graph) {
      in = &cin;
      out = &cout;
    }

    /**
     * Create a connection for sending/receiving requests from a file descriptor
     */
    base_connection::base_connection(int conn, rgraph *graph) : conn(conn), graph(graph) {
      stdio_filebuf<char>* ibuf = new stdio_filebuf<char>(conn, std::ios::in);
      in = new istream(ibuf);

      stdio_filebuf<char>* obuf = new stdio_filebuf<char>(conn, std::ios::out);
      out = new ostream(obuf);
    }


    base_connection::~base_connection() {
      if(conn) {
	delete in->rdbuf();
	delete out->rdbuf();

	delete in;
	delete out;

	close_connection(conn);
      }

    }

  }
}

