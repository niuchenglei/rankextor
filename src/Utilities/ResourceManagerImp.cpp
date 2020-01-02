#include "Utilities/ResourceManagerImp.h"
#include <sys/stat.h>
#include "Interfaces/Logger.h"

namespace rank {
 

ResourceManagerImp::ResourceManagerImp() {


}

ResourceManagerImp::~ResourceManagerImp() {
  boost::unordered_map<std::string, ResourceMeta>::iterator iter = res_list_.begin();
  for (; iter!=res_list_.end(); iter++) {
    ResourceMeta& meta = iter->second;
    for (int i=0; i<meta.res_vec.size(); i++) {
      if (meta.res_vec[i] != NULL) {
        meta.res_info.destroy_resource_func(meta.res_vec[i]);
        meta.res_vec[i] = NULL;
      }
    }
    meta.res_vec.clear();
  }
  res_list_.clear();
}
 
// 资源注册
// 1. 在内部调用resource_type.create_resource_func()，得到resource指针
// 2. 然后调用 resource->Update() 完成资源加载
int ResourceManagerImp::RegisterResource(const ResourceInfo& resource_info) {
  std::string resource_name = resource_info.resource_name;
  INFO(LOGNAME, "resource manager register resource begin, name=%s, reload_flag=%s, conf=%s, type=%s",
      resource_info.resource_name.c_str(),
      resource_info.resource_reload_flag.c_str(),
      resource_info.resource_conf.c_str(), resource_info.resource_type.c_str());
  boost::unordered_map<std::string, ResourceMeta>::const_iterator iter =
      res_list_.find(resource_name);
  if (iter != res_list_.end()) {
    WARN(LOGNAME, "resource manager duplicated register resource , resource=%s",
                resource_name.c_str());
    return 0;
  }
  ResourceMeta meta;
  meta.res_info = resource_info;
  meta.last_update_time = 0;
  if ((resource_info.resource_type == "file") &&
      !meta.res_info.resource_reload_flag.empty()) {
    struct stat file_stat;
    stat(meta.res_info.resource_reload_flag.c_str(), &file_stat);
    meta.last_update_time = file_stat.st_mtime;
  }
  if (resource_info.resource_type == "redis") {
    /*if (resource_info.cache_enable) {
      meta.cache = NewLRUCache(1024 * 1024 * 1024);
      // meta.cache->SetName("dmp resource");
      // meta.cache->SetCacheExpireTime(3 * 60);
    }
    for (int i = 0; i < worker_num_; ++i) {
      meta.res_info = resource_info;
      Resource* res_ptr = meta.res_info.create_resource_func();
      if (!res_ptr) {
        FISHER_ERROR("resource manager create resource failed, resource=%s",
                     resource_name.c_str());
        return 1;
      }
      int ret = res_ptr->Load(resource_info.resource_conf);
      if (ret != 0) {
        FISHER_ERROR(
            "resource manager load resource failed, resource=%s, ret=%d",
            resource_name.c_str(), ret);
        return 2;
      }
      if (resource_info.cache_enable && meta.cache) {
        res_ptr->SetCache(meta.cache);
      }
      meta.res_vec.push_back(res_ptr);
    }*/
  } else {  // file && redis_special will be here
    for (int i = 0; i < 2; ++i) {
      meta.res_info = resource_info;
      Resource* res_ptr = meta.res_info.create_resource_func();
      if (!res_ptr) {
        ERROR(LOGNAME, "resource manager create resource failed, resource=%s",
                     resource_name.c_str());
        return 1;
      }
      int ret = res_ptr->Load(resource_info.resource_conf);
      if (ret != 0) {
        ERROR(LOGNAME, "resource manager load resource failed, resource=%s, ret=%d",
            resource_name.c_str(), ret);
        return 2;
      }
      meta.res_vec.push_back(res_ptr);
    }
    meta.res_index = 0;
  }
  res_list_[meta.res_info.resource_name] = meta;
  DEBUG(LOGNAME, "resource manager register resource end, resource=%s",
               resource_name.c_str());
  return 0;
}

// 取消资源注册, 在内部调用resource_type.destroy_resource_func()
int ResourceManagerImp::UnRegisterResource(const std::string& resource_name) {
  boost::unordered_map<std::string, ResourceMeta>::iterator iter = res_list_.find(resource_name);
  if (iter != res_list_.end()) {
    ResourceMeta& meta = iter->second;
    for (int i=0; i<meta.res_vec.size(); i++) {
      if (meta.res_vec[i] != NULL) {
        meta.res_info.destroy_resource_func(meta.res_vec[i]);
        meta.res_vec[i] = NULL;
      }
    }
    meta.res_vec.clear();
    res_list_.erase(resource_name);
  }
  return 0;
}

// 通过资源的名字获取资源
int ResourceManagerImp::FetchResource(const std::string& resource_name,
                                      Resource** resource, int worker_idx) {
  boost::unordered_map<std::string, ResourceMeta>::iterator iter =
      res_list_.find(resource_name);
  if (iter == res_list_.end()) {
    ERROR(LOGNAME, "resource manager fetch resource failed, resource=%s",
                 resource_name.c_str());
    return 1;
  }
  ResourceMeta& meta = iter->second;
  if (meta.res_info.resource_type == "redis") {
    *resource = meta.res_vec[worker_idx];
    DEBUG(LOGNAME, "resource manager fetch resource, resource=%s",
                 resource_name.c_str());
  } else {  // file && redis_special will be here
    *resource = meta.res_vec[meta.res_index];
    DEBUG(LOGNAME, "resource manager fetch resource, resource=%s",
                 resource_name.c_str());
  }
  return 0;
}

// 依据资源的唯一标识符和key更新对应的资源
int ResourceManagerImp::UpdateResource(const std::string& resource_name,
                                       const std::string& key,
                                       const std::string& value) {
  return 0;
}

// 依据资源的唯一标识符和key查找对应的资源
int ResourceManagerImp::FetchResource(const std::string& resource_name,
                                      const std::string& key,
                                      std::string* value) {
  return 0;
}

int ResourceManagerImp::FetchResource(const std::string& resource_name,
                                      std::map<std::string, std::string>* map) {
  return 0;
}

int ResourceManagerImp::Reload() {
  int ret = 0;
  boost::unordered_map<std::string, ResourceMeta>::iterator iter = res_list_.begin();
  for (; iter != res_list_.end(); ++iter) {
    ResourceMeta& meta = iter->second;
    INFO(LOGNAME, "resource manager reload resource begin, name=%s, reload_flag=%s, "
        "conf=%s, type=%s",
        meta.res_info.resource_name.c_str(),
        meta.res_info.resource_reload_flag.c_str(),
        meta.res_info.resource_conf.c_str(),
        meta.res_info.resource_type.c_str());

    if (meta.res_info.resource_type == "redis") {
      WARN(LOGNAME, 
          "resource manager reload resource type error name=%s, "
          "reload_flag=%s, conf=%s, type=%s",
          meta.res_info.resource_name.c_str(),
          meta.res_info.resource_reload_flag.c_str(),
          meta.res_info.resource_conf.c_str(),
          meta.res_info.resource_type.c_str());
      continue;
    }

    if (meta.res_info.resource_type == "file") {
      if (meta.res_info.resource_reload_flag.empty()) {
        WARN(LOGNAME, 
            "resource manager reload flag empty, name=%s, reload_flag=%s, "
            "conf=%s",
            meta.res_info.resource_name.c_str(),
            meta.res_info.resource_reload_flag.c_str(),
            meta.res_info.resource_conf.c_str());
        continue;
      }

      struct stat file_stat;
      ret = stat(meta.res_info.resource_reload_flag.c_str(), &file_stat);
      if (ret != 0) {
        ERROR(LOGNAME, 
            "resource manager stat file failed, name=%s, reload_flag=%s, "
            "conf=%s, ret=%d",
            meta.res_info.resource_name.c_str(),
            meta.res_info.resource_reload_flag.c_str(),
            meta.res_info.resource_conf.c_str(), ret);
        continue;
      }

      if (meta.last_update_time == file_stat.st_mtime) {
        continue;
      }
      meta.last_update_time = file_stat.st_mtime;
    }

    // file && redis_special will be here
    INFO(LOGNAME, 
        "resource manager reload, name=%s, reload_flag=%s, conf=%s, type=%s",
        meta.res_info.resource_name.c_str(),
        meta.res_info.resource_reload_flag.c_str(),
        meta.res_info.resource_conf.c_str(),
        meta.res_info.resource_type.c_str());
    int index = (meta.res_index + 1) % 2;
    Resource* res = meta.res_vec[index];
    ret = res->Load(meta.res_info.resource_conf);
    if (ret != 0) {
      ERROR(LOGNAME, 
          "resource manager reload failed, name=%s, reload_flag=%s, conf=%s, "
          "type=%s",
          meta.res_info.resource_name.c_str(),
          meta.res_info.resource_reload_flag.c_str(),
          meta.res_info.resource_conf.c_str(),
          meta.res_info.resource_type.c_str());
      continue;
    }
    INFO(LOGNAME,
        "resource manager reload resource end, name=%s, reload_flag=%s, "
        "conf=%s, type=%s",
        meta.res_info.resource_name.c_str(),
        meta.res_info.resource_reload_flag.c_str(),
        meta.res_info.resource_conf.c_str(),
        meta.res_info.resource_type.c_str());
    meta.res_index = index;
  }
  return 0;
}
}
