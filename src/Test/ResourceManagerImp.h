#ifndef RESOURCE_MANAGER_IMP_H
#define RESOURCE_MANAGER_IMP_H

#include "process/resource.h"
#include <map>

using namespace std;

class ResourceManagerImp : public ResourceManager {
 public:
  // 资源注册
  // 1. 在内部调用resource_type.create_resource_func()，得到resource指针
  // 2. 然后调用 resource->Update() 完成资源加载
  virtual int RegisterResource(const ResourceInfo& resource_info) {
    Resource* ins = resource_info.create_resource_func();
    int flag = ins->Load(resource_info.resource_conf);
    if (flag != 0) return flag;
    mData.insert(make_pair(resource_info.resource_name, ins));
    return 0;
  }
  // 取消资源注册, 在内部调用resource_type.destroy_resource_func()
  virtual int UnRegisterResource(const std::string& resource_name) { return 0; }
  // 通过资源的名字获取资源
  virtual int FetchResource(const std::string& resource_name,
                            Resource** resource, int worker_index) {
    if (mData.find(resource_name) == mData.end()) return -1;
    std::map<std::string, Resource*>::iterator iter = mData.find(resource_name);
    *resource = iter->second;
    return 0;
  }

  // 依据资源的唯一标识符和key更新对应的资源
  virtual int UpdateResource(const std::string& resource_name,
                             const std::string& key, const std::string& value) {
    return 0;
  }

  // 依据资源的唯一标识符和key查找对应的资源
  virtual int FetchResource(const std::string& resource_name,
                            const std::string& key, std::string* value) {
    return 0;
  }

  virtual int FetchResource(const std::string& resource_name,
                            std::map<std::string, std::string>* map) {
    return 0;
  }

 private:
  std::map<std::string, Resource*> mData;
};

#endif
