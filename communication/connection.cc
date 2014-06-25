#include <ext/stdio_filebuf.h>
#include <iostream>

#include "connection.h"

extern "C" {
#include "network.h"
}

mico::network::base_connection::base_connection(rgraph *graph) : conn(0), graph(graph) {
  in = &std::cin;
  out = &std::cout;
}


mico::network::base_connection::base_connection(int conn, rgraph *graph) : conn(conn), graph(graph) {
  __gnu_cxx::stdio_filebuf<char>* ibuf = new __gnu_cxx::stdio_filebuf<char>(conn, std::ios::in);
  in = new std::istream(ibuf);

  __gnu_cxx::stdio_filebuf<char>* obuf = new __gnu_cxx::stdio_filebuf<char>(conn, std::ios::out);
  out = new std::ostream(obuf);
}


mico::network::base_connection::~base_connection() {
  if(conn) {
    delete in->rdbuf();
    delete out->rdbuf();

    delete in;
    delete out;

    close_connection(conn);
  }

}

