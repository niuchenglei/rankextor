#ifndef REDIS_MANAGER_H
#define REDIS_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <errno.h>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include "Interfaces/resource.h"
#include "Interfaces/define.h"

#include "Utilities/Cluster.h"
#include "Interfaces/Logger.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/CommonType.h"

// using namespace kf::base;

namespace rank {

class RedisManager : public AutoUpdate {
 public:
  bool Initialize(const std::string& config_file,
                  const std::string& redis_group_config);
  void Clear();
  void Fetch();
  bool AddUserKey(const RankRequest& fisher_request, UserInfo& user);
  bool AddAdidRealTimeKey(const std::vector<RankAdInfo>& ad_list);
  template <typename T>
  bool Get(const std::string& type, const std::string& id,
           const std::string& property, T& value);
  template <typename T>
  bool Get(const std::string& id, T& value);

  virtual bool Update();
  virtual const std::string& getTypeName() const { return ""; }
  virtual const std::string& getRegistName() const { return ""; }
  virtual const bool getRegistFlag() const { return false; }

  RedisManager();
  virtual ~RedisManager();

 private:
  ad_algo::Cluster redis_db_;
  // std::vector<string> m_adid_str_vec;
  std::string mRedisGroupConfigFile;
  std::map<string, bool> mRedisGroupConfig;

  bool getRedisGroupConfig(const char* group);
};

class RedisHandler : public AutoUpdate {
 public:
  bool Initialize(const std::string& redis_group_config);
  bool Check(ResourceManager* resource_manager, int worker_id);
  void Clear();
  bool Fetch(const std::map<std::string, std::string>& rank_user_data);
  bool AddUserKey(const RankRequest& fisher_request, UserInfo& user);
  bool AddAdidRealTimeKey(const std::vector<RankAdInfo>& ad_list);
  template <typename T>
  bool Get(const std::string& type, const std::string& id,
           const std::string& property, T& value) {
    string key = type + ":" + id + ":" + property;
    return Get<T>(key, value);
  }
  template <typename T>
  bool Get(const std::string& id, T& value);

  virtual bool Update();
  virtual const std::string& getTypeName() const { return ""; }
  virtual const std::string& getRegistName() const { return ""; }
  virtual const bool getRegistFlag() const { return false; }

 private:
  rank::Resource* handler_instance;
  std::vector<std::string> keys;
  std::map<std::string, std::string> datas;

  std::string mRedisGroupConfigFile;
  std::map<string, bool> mRedisGroupConfig;
  bool getRedisGroupConfig(const char* group);
};

}  // namespace rank

#endif
