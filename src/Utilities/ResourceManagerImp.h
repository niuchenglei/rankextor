#ifndef RANK_RESOURCE_MANAGER_IMP_H_
#define RANK_RESOURCE_MANAGER_IMP_H_

#include "Interfaces/resource.h"
#include <boost/unordered_map.hpp>

namespace rank {

class ResourceManagerImp : public ResourceManager {
 public:
  // 资源注册
  // 1. 在内部调用resource_type.create_resource_func()，得到resource指针
  // 2. 然后调用 resource->Update() 完成资源加载
  virtual int RegisterResource(const ResourceInfo& resource_info);
  // 取消资源注册, 在内部调用resource_type.destroy_resource_func()
  virtual int UnRegisterResource(const std::string& resource_name);
  // 通过资源的名字获取资源
  virtual int FetchResource(const std::string& resource_name,
                            Resource** resource, int worker_idx = -1);

  // 依据资源的唯一标识符和key更新对应的资源
  virtual int UpdateResource(const std::string& resource_name,
                             const std::string& key, const std::string& value);

  // 依据资源的唯一标识符和key查找对应的资源
  virtual int FetchResource(const std::string& resource_name,
                            const std::string& key, std::string* value);

  virtual int FetchResource(const std::string& resource_name,
                            std::map<std::string, std::string>* map);

  int Reload();

  ResourceManagerImp();
  ~ResourceManagerImp();

private:
  struct ResourceMeta {
    ResourceInfo res_info;
    int res_index;
    int64_t last_update_time;
    std::vector<Resource*> res_vec;
  };

  boost::unordered_map<std::string, ResourceMeta> res_list_;
};
}
#endif
