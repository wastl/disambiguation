#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "thread.h"


class mythread : public mico::threading::thread {
public:

  void run() {
    std::cout << "run started!\n";
    sleep(5);
    std::cout << "run ended!\n";
  };
  
  void finished() {
    std::cout << "thread finalizing!\n";
  };


};

int main(void) {
  mythread t;

  t.start();

  std::cout << "thread started, waiting for completion!\n";

  t.join();

  std::cout << "completed!\n";
  
  exit(0);
}
