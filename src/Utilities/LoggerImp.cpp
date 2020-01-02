#include "LoggerImp.h"

namespace rank {

static char __config_file_path[4096];
char* get_log4cpp_config() { return __config_file_path; }
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

LoggerImp* LoggerImp::_plog = NULL;

LoggerImp::LoggerImp() {
  pthread_mutex_lock(&mutex);
  try {
    log4cpp::PropertyConfigurator::configure(__config_file_path);
  } catch (log4cpp::ConfigureFailure& f) {
    fprintf(stderr, "%s\n", f.what());  //cout<<f.what()<<endl;
  }
  rootCategory = &log4cpp::Category::getRoot().getInstance("queen");
  if (rootCategory == NULL) {
    fprintf(stderr, "rootCategory == NULL");
  } else {
    fprintf(stderr, "log4cpp[%s] going\n", __config_file_path);
    rootCategory->fatal("log4cpp[%s] going", __config_file_path);
  }
  pthread_mutex_unlock(&mutex);
}

LoggerImp::~LoggerImp() {
  rootCategory->shutdown(); 
  rootCategory = NULL; 
}

void LoggerImp::destroy() {
  delete _plog;
}

LoggerImp* LoggerImp::getInstance() {
  if(NULL==_plog){
    _plog = new LoggerImp();
  }
  return _plog;
}

}

