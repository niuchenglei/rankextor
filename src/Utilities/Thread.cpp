#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include <sys/wait.h>

#include "Interfaces/Thread.h"

namespace rank {

static void *__exec__(void *arg) {
  Thread *pth = static_cast<Thread *>(arg);
  pth->_exit_code = pth->Run();
  pthread_exit(0);
}

Thread::Thread() : _sign_stop(false), _is_stopped(true) {}

Thread::~Thread() {
  Stop();
  Wait();
}

unsigned long int Thread::getCurrentThreadId() {
  pthread_t tid;
  tid = pthread_self();

  return (unsigned long int)tid;
}

void Thread::Sleep(int second) {
  // usleep(millisecond);
  sleep(second);
}

int Thread::Exec() {
  while (!_sign_stop) {
    sleep(2);
  }
  return 0;
}

void Thread::Start() {
  _is_stopped = false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  int err = pthread_create(&_pid, &attr, __exec__, this);

  if (err != 0) {
    _is_stopped = true;
    throw std::runtime_error("can't create thread.");
  }
  // VERIFYNRV_MSG(err==0, "can't create thread!")
}

int Thread::Run() {
  _is_stopped = false;
  _sign_stop = false;
  int r = Exec();
  _is_stopped = true;
  _sign_stop = false;
  return r;
}

void Thread::Stop() {
  _sign_stop = true;
  // pthread_cancel(_pid);   // it is important
}

int Thread::Wait() {
  while (!_is_stopped) usleep(100000);

  // pthread_join(_pid, NULL);
  return _exit_code;
}

bool Thread::Running() const { return !_is_stopped; }

bool Thread::getSignStop() const { return _sign_stop; }
}  // namespace rank
