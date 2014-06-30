#include <ext/stdio_filebuf.h>
#include <iostream>
#include <unistd.h> // close

#include "network.h"
#include "connection.h"


namespace mico {
  namespace network {

    using namespace std;
    using namespace __gnu_cxx;
    using namespace mico::graph;

    /**
     * Create a connection for sending/receiving requests from standard input/output
     */
    BaseConnection::BaseConnection() : conn(0) {
      cout << "created new local connection...\n";

      in = &cin;
      out = &cout;
    }

    /**
     * Create a connection for sending/receiving requests from a file descriptor
     */
    BaseConnection::BaseConnection(int conn) : conn(conn) {
      cout << "created new network connection...\n";

      stdio_filebuf<char>* ibuf = new stdio_filebuf<char>(conn, std::ios::in);
      in = new istream(ibuf);

      stdio_filebuf<char>* obuf = new stdio_filebuf<char>(conn, std::ios::out);
      out = new ostream(obuf);
    }


    BaseConnection::~BaseConnection() {
      if(conn) {
	delete in->rdbuf();
	delete out->rdbuf();

	delete in;
	delete out;

	close(conn);
      }

    }

  }
}

