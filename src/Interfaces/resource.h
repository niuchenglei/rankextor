#ifndef RANK_RESOURCE_H_
#define RANK_RESOURCE_H_

#include <string>
#include <vector>
#include <map>

namespace rank {

//class Cache;
// 资源抽象类
// 只提供一个update接口，来完成数据的加载
// 外界无需关心文件的格式，以及数据是如何被加载的
// 至于数据以什么方式被外界获取，需要由派生类提供访问接口
class Resource {
 public:
  virtual ~Resource() {};
  virtual int Load(const std::string& conf) = 0;
  virtual int GetValue(const std::string& key, std::string* value) = 0;
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) = 0;
  virtual int Find(const std::string& key) { return 0; }
  //virtual void SetCache(Cache* cache) {};
};

typedef Resource* (*CreateResourceFunc)();
typedef void (*DestroyResourceFunc)(Resource*);

struct ResourceInfo {
  std::string resource_name;  // required字段，资源名字 全局唯一性
  std::string resource_type;  // required字段，资源类型 redis or file or
                              // redis_special
  std::string resource_conf;  // required字段，资源配置
  std::string resource_reload_flag;  // 依据该字段判断数据是否发生变化,
  // 当资源类型为redis时，该字段无意义
  CreateResourceFunc create_resource_func;  // 该函数由算法同学提供,
  // 当资源类型为redis时，该字段无意义
  DestroyResourceFunc destroy_resource_func;  // 该函数由算法同学提供,
  // 当资源类型为redis时，该字段无意义
  bool cache_enable;  // 是否使用cache
};

// 所有的数据存储在ResourceManager里面
class ResourceManager {
 public:
  virtual ~ResourceManager() {};
  // 资源注册
  // 1. 在内部调用resource_type.create_resource_func()，得到resource指针
  // 2. 然后调用 resource->Update() 完成资源加载
  virtual int RegisterResource(const ResourceInfo& resource_info) = 0;

  // 取消资源注册, 在内部调用resource_type.destroy_resource_func()
  virtual int UnRegisterResource(const std::string& resource_name) = 0;

  // 通过资源的名字获取资源
  virtual int FetchResource(const std::string& resource_name,
                            Resource** resource, int worker_index = -1) = 0;

  // 依据资源的唯一标识符和key更新对应的资源
  virtual int UpdateResource(const std::string& resource_name,
                             const std::string& key,
                             const std::string& value) = 0;

  // 依据资源的唯一标识符和key查找对应的资源
  virtual int FetchResource(const std::string& resource_name,
                            const std::string& key, std::string* value) = 0;

  virtual int FetchResource(const std::string& resource_name,
                            std::map<std::string, std::string>* map) = 0;

  virtual int Reload() = 0;
};

}  // namespace fisher

#endif  // FISHER_RESOURCE_H_
