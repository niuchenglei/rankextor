#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "sys/stat.h"
#include "pthread.h"
#include "Interfaces/Logger.h"
#include "Interfaces/resource.h"

namespace rank {

#define REGIST_HANDLER(file) EventFile::RegistHandler(file, UpdateAux, this);

#define REGIST_HANDLER_PTR(file, ptr) \
  EventFile::RegistHandler(file, UpdateAux, ptr);

#define UNREGIST_HANDLER(file) EventFile::UnRegistHandler(file, this);

#define UNREGIST_HANDLER_PTR(file, ptr) EventFile::UnRegistHandler(file, ptr);

typedef bool (*HandleFunc)(void*);

bool UpdateAux(void*);
class AutoUpdate {
 public:
  virtual bool Update() = 0;
  virtual const std::string& getTypeName() const = 0;
  virtual const std::string& getRegistName() const = 0;
  virtual const bool getRegistFlag() const = 0;
};

class EventHandler {
 public:
  EventHandler(const std::string& file, HandleFunc handle, void* ptr);
  std::string file_;
  HandleFunc handle_;
  void* ptr_;
  time_t mTime_;
};

class EventFile {
 public:
  static EventFile* instance();
  static void destroy();
  static void kill_thread();
  static void RegistHandler(const std::string& file, HandleFunc handle,
                            void* ptr);
  static void UnRegistHandler(const std::string& file, void* ptr);

 private:
  static EventFile* spInstance;
  static bool mDestroyed;
  std::vector<EventHandler> pool_;
  pthread_mutex_t lock;
  static bool now_to_stop, stop_over;

  EventFile();
  ~EventFile();

  static void* startMonitor(void*);
};

#define DECLEAR_RESOURCE(x)       \
  rank::Resource* create_##x(); \
  void destroy_##x(rank::Resource* ins);

#define DEFINE_RESOURCE(x)                         \
  rank::Resource* create_##x() { return new x; } \
  void destroy_##x(rank::Resource* ins) {        \
    x* _case = static_cast<x*>(ins);               \
    delete _case;                                  \
  }

#define REGISTE_RESOURCE(file, x, resource_manager)                    \
  {                                                                    \
    struct rank::ResourceInfo __resource_##x__ = {                   \
        file, "file", file, file + ".touch", create_##x, destroy_##x}; \
    if (0 != resource_manager->RegisterResource(__resource_##x__))     \
      FATAL(LOGNAME, "regist resource error[%s].", file.c_str());      \
  }

#define REGISTE_RESOURCE_TOUCH(file, touch_file, x, resource_manager) \
  {                                                                   \
    struct rank::ResourceInfo __resource_##x__ = {                  \
        file, "file", file, touch_file, create_##x, destroy_##x};     \
    \            
if(resource_manager->RegisterResource(__resource_##x__) != 0)         \
        FATAL(LOGNAME, "regist resource error[%s].", file.c_str());   \
  }

#define REGISTE_RESOURCE_INSTANCE_TOUCH(file, touch_file, x, resource_manager) \
  {                                                                            \
    Service<ConfigurationSettings> _pSetting;                                  \
    std::string _ss = _pSetting->getSetting("main/plugin_name") + "_" + file;  \
    REGISTE_RESOURCE_TOUCH(_ss, touch_file, x, resource_manager);              \
  }
/*
struct ResourceInfo {
  std::string resource_name; // required字段，资源名字 全局唯一性
  std::string resource_type; // required字段，资源类型 redis or file
  std::string resource_conf; // required字段，资源配置
  std::string resource_reload_flag; // 依据该字段判断数据是否发生变化,
当资源类型为redis时，该字段无意义
  CreateResourceFunc create_resource_func; // 该函数由算法同学提供,
当资源类型为redis时，该字段无意义
  DestroyResourceFunc destroy_resource_func;// 该函数由算法同学提供,
当资源类型为redis时，该字段无意义
};*/

}  // namespace rank

#endif
