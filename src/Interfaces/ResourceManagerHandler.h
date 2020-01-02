#ifndef RESOURCE_MANAGER_HANDLER_H
#define RESOURCE_MANAGER_HANDLER_H

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "sys/stat.h"
#include "pthread.h"
#include "Interfaces/Logger.h"
#include "Interfaces/resource.h"

namespace rank {

class ResourceManagerHandler {
public:
  static ResourceManagerHandler* instance();
  static void destroy();
  static void kill_thread();
  static ResourceManager* resource_manager;

private:
  static ResourceManagerHandler* spInstance;
  static bool mDestroyed;
  static bool now_to_stop, stop_over;
  pthread_mutex_t lock;

  ResourceManagerHandler();
  ~ResourceManagerHandler();

  static void* startMonitor(void*);
};


}  // namespace rank

#endif
