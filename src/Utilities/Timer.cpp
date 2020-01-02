#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include <sys/wait.h>

#include "Interfaces/Timer.h"

namespace rank {

Timer::Timer() : count_number(1), interval(1), callback_func(NULL) {}

Timer::~Timer() {}

void Timer::setCount(int count) { count_number = count; }

void Timer::setInterval(int seconds) { interval = seconds; }

void Timer::setCallback(CALLBACK_FUNC callback, void* _data) {
  callback_func = callback;
  data = _data;
}

int Timer::Exec() {
  int i = count_number;

  while (i > 0 && !getSignStop()) {
    // THREAD_CANCEL_POINT
    for (int j = 0; j < interval; j++) {
      if (!getSignStop())
        Thread::Sleep(1);
      else
        return 0;
    }

    if (callback_func) callback_func(data);

    i--;
  }
  return 0;
}
}  // namespace rank
