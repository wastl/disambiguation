#include <ext/stdio_filebuf.h>
#include <iostream>

#include "worker.h"

extern "C" {
#include "network.h"
}

WorkerConnection::WorkerConnection(rgraph *graph) : connection(0), graph(graph) {
  in = &std::cin;
  out = &std::cout;
}


WorkerConnection::WorkerConnection(int connection, rgraph *graph) : connection(connection), graph(graph) {
  __gnu_cxx::stdio_filebuf<char>* ibuf = new __gnu_cxx::stdio_filebuf<char>(connection, std::ios::in);
  in = new std::istream(ibuf);

  __gnu_cxx::stdio_filebuf<char>* obuf = new __gnu_cxx::stdio_filebuf<char>(connection, std::ios::out);
  out = new std::ostream(obuf);
}

WorkerConnection::~WorkerConnection() {
  if(connection) {
    delete in;
    delete out;

    close_connection(connection);
  }

}



WorkerConnection& WorkerConnection::operator<<(WSDDisambiguationRequest &r) {
  int length = r.ByteSize();
  std::cout << "sending a response of "<<length<<" bytes ...\n";

  char buf[length];
  r.SerializeToArray(buf,length);

  std::cout << "first byte: '" << (int)buf[0] << "', second byte: '"<<(int)buf[1]<<"'\n";

  out->write((char*)&length, sizeof(int));
  out->write(buf,length);
  out->flush();

  return *this;
}


WorkerConnection& WorkerConnection::operator>>(WSDDisambiguationRequest &r) {

  if( !in->eof() ) {
    // read length of next message
    int length;
    *in >> length;

    char buf[length];  
    in->read(buf,length);

    if(*in) {
      r.ParseFromArray(buf, length);
    }
  }
  return *this;
}



WSDDisambiguationRequest* WorkerConnection::nextRequest() {
  if( in->eof() ) {
    return NULL;
  }

  // read length of next message
  int length;
  *in >> length;

  char buf[length];  
  in->read(buf,length);

  std::cout << "reading in next message of " << length << " bytes\n";

  if(*in) {
    WSDDisambiguationRequest* r = new WSDDisambiguationRequest(); 
    if(r->ParseFromArray(buf, length)) {
      return r;
    } else {
      delete r;
      return NULL;
    }
  } else {
    return NULL;
  }
}
