#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "thread.h"

void mico::threading::thread::cleaner(void * t) {
  mico::threading::thread* _thread = (mico::threading::thread*)t;
  _thread->cancelled();
};

void * mico::threading::thread::runner(void * t) {
  mico::threading::thread* _thread = (mico::threading::thread*)t;

  _thread->state = RUNNING;

  pthread_cleanup_push(&cleaner, _thread);
  _thread->run();
  pthread_cleanup_pop(0);

  _thread->state = FINISHED;


  _thread->finished();

  return NULL;
};



void mico::threading::thread::start() {  
  if(state == CREATED) {
    int s = pthread_create(&_thread, NULL, &runner, this);
    if(s != 0) {
      errno = s; perror("error creating thread"); exit(1);
    }
  } else {
    throw exception("thread already running or cancelled");
  }
}


void mico::threading::thread::cancel() {
  if(state == RUNNING) {
    pthread_cancel(_thread);
    state = CANCELLED;
  } // otherwise silently ignore
}


void mico::threading::thread::reset() {
  if(state == RUNNING) {
    throw exception("running thread cannot be reset");
  } else {
    state = CREATED;
  }
}


void mico::threading::thread::join() {
  if(state == RUNNING) {
    pthread_join(_thread, NULL);
  }
}

