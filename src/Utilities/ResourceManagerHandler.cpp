#include "Interfaces/ResourceManagerHandler.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Utilities/ResourceManagerImp.h"

namespace rank {

static const std::string EVENT_FILE_SUFFIX = ".touch";
static const int MONITOR_INTERVAL_SECOND = 120;

ResourceManagerHandler* ResourceManagerHandler::spInstance = NULL;
bool ResourceManagerHandler::mDestroyed = false;
ResourceManager* ResourceManagerHandler::resource_manager = NULL;

bool ResourceManagerHandler::now_to_stop = false;
bool ResourceManagerHandler::stop_over = false;

#define THREAD_ON

ResourceManagerHandler* ResourceManagerHandler::instance() {
  if (spInstance && stop_over == true) {
    resource_manager = (ResourceManager*)new ResourceManagerImp();
#ifdef THREAD_ON    
    pthread_t tid;
    if (pthread_create(&tid, NULL, startMonitor, NULL) != 0) {
      ERROR(LOGNAME, "[ResourceManagerHandler] pthread_create failed.");
      exit(-1);
    }
#endif
    now_to_stop = false;
    stop_over = false;
  }

  if (spInstance == NULL) {
    spInstance = new ResourceManagerHandler;
  }

  return spInstance;
}

void ResourceManagerHandler::destroy() {
  /*if (mDestroyed) {
    return; throw std::logic_error(
        "Attempting to destroy ResourceManagerHandler after "
        "destroying it.");
  }*/

  delete spInstance;
  spInstance = NULL;
  mDestroyed = true;
}

void ResourceManagerHandler::kill_thread() {
#ifdef THREAD_ON
  now_to_stop = true;
  while (!stop_over) {
    sleep(1);
  }
#endif
}

ResourceManagerHandler::ResourceManagerHandler() {
  resource_manager = (ResourceManager*)new ResourceManagerImp();
#ifdef THREAD_ON
  pthread_t tid;
  pthread_mutex_init(&lock, NULL);
  if (pthread_create(&tid, NULL, startMonitor, NULL) != 0) {
    ERROR(LOGNAME, "[ResourceManagerHandler] pthread_create failed.");
    exit(-1);
  }
#endif
}
ResourceManagerHandler::~ResourceManagerHandler() {
  delete resource_manager;
  resource_manager = NULL;
};

void* ResourceManagerHandler::startMonitor(void*) {
  Service<ConfigurationSettings> pSetting;
  std::string time_s = pSetting->getSetting("main/monitor_interval_second");
  int sleep_time = MONITOR_INTERVAL_SECOND;
  if (!time_s.empty()) {
    sleep_time = atoi(time_s.c_str());
    if (sleep_time < 0) {
      sleep_time = MONITOR_INTERVAL_SECOND;
    }
  }
  //if (sleep_time < 120)
  //sleep_time = 10;
  
  while (true) {
    if (now_to_stop)
      break;
    DEBUG(LOGNAME, "event sleeping %d seconds", sleep_time);
    sleep(sleep_time);
    if (now_to_stop)
      break;

    pthread_mutex_lock(&spInstance->lock);
    
    resource_manager->Reload();

    pthread_mutex_unlock(&spInstance->lock);
  }
  stop_over = true;
  return NULL;
}

}  // namespace rank
