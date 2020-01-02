#include <sys/time.h>
#include "Interfaces/RedisManager.h"
#include "Interfaces/Logger.h"
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/Util.h"

using namespace boost;

namespace rank {

bool RedisManager::Initialize(const std::string& config_file,
                              const std::string& redis_group_config) {
  if (redis_db_.Init(config_file)) {
    mRedisGroupConfig.clear();
    mRedisGroupConfigFile = redis_group_config;
    REGIST_HANDLER(mRedisGroupConfigFile);
    return Update();
  }

  const string& errstr = redis_db_.ErrorStr();
  ERROR(LOGNAME, "error caused by %s", redis_db_.ErrorStr().c_str());
  return false;
}

void RedisManager::Clear() { redis_db_.Clear(); }

void RedisManager::Fetch() {
  redis_db_.Fetch();

  const string& errstr = redis_db_.ErrorStr();
  if (strlen(errstr.c_str()) != 0)
    ERROR(LOGNAME, "redis fetch error: %s", errstr.c_str());
}

#define __EXPAND_KEY__(A, B, C) \
  (string(A) + string(":") + B + string(":") + string(C))
bool RedisManager::AddUserKey(const RankRequest& fisher_request,
                              UserInfo& user) {
  string tmp_flag = "limitupdateflag",
         uid = boost::lexical_cast<string>(fisher_request.uid),
         group_id = boost::lexical_cast<string>(fisher_request.psid);

  /*keys.push_back(__EXPAND_KEY__("user", uid, "com_ubi"));
  keys.push_back(__EXPAND_KEY__("user", uid, "feed_hgcpb"));
  keys.push_back(__EXPAND_KEY__("user", uid, "feed_ucrw"));
  keys.push_back(__EXPAND_KEY__("user", uid, "feed_stin"));
  //keys.push_back(__EXPAND_KEY__("user", uid, "feed_clustertimes"));

  keys.push_back(__EXPAND_KEY__("rts", uid, "feed_cust"));
  keys.push_back(__EXPAND_KEY__("rts", tmp_flag, "feed_upflag"));

  keys.push_back(__EXPAND_KEY__("group", group_id, "tag"));*/

  redis_db_.Add("user", uid, "com_ubi");
  redis_db_.Add("user", uid, "feed_hgcpb");
  redis_db_.Add("u2", uid, "feedtpb");
  redis_db_.Add("user", uid, "feed_ucrw");
  redis_db_.Add("user", uid, "feed_stin");
  redis_db_.Add("rts", uid, "feed_cust");
  redis_db_.Add("rts", tmp_flag, "sfst_upflag");
  redis_db_.Add("user", uid, "feed_clustertimes");
  redis_db_.Add("group", group_id, "tag");
  redis_db_.Add("user", uid, "feed_clkcatekunguang");
  redis_db_.Add("un", uid, "un");
  redis_db_.Add("user", uid, "feed_pv");

  return true;
}
#undef __EXPAND_KEY__

bool RedisManager::AddAdidRealTimeKey(
    const std::vector<RankAdInfo>& ad_list) {
  // std::vector<AdBaseInfo*>& ad_list = *algo_in->ad_list;
  // std::vector<AdInfo>& ad_list = *algo_in->ad_list;
/*
  size_t ad_size = ad_list.size();
  // vector<string>(ad_size).swap(m_adid_str_vec);
  string tmp_adid_str = "";
  string tmp_custid_str = "";
  string tmp_feedid_str = "";
  for (size_t i = 0; i < ad_size; i++) {
    tmp_adid_str = boost::lexical_cast<string>(ad_list[i].ad_id);

    if (getRedisGroupConfig("rts")) {
      //redis_db_.Add("rts", tmp_adid_str, "sfst_impbhv");
      //redis_db_.Add("rts", tmp_adid_str, "sfst_cimpbhv");

      //增加OCPX中当天实时数据的Key
      struct tm* currentTime;
      time_t t;
      t = time(NULL);
      currentTime = localtime(&t);
      int todayDate = (currentTime->tm_year + 1900) * 10000 +
                      (currentTime->tm_mon + 1) * 100 + currentTime->tm_mday;
      string ocpxKey = "ocpx_info:" + boost::lexical_cast<string>(todayDate);
      //redis_db_.Add("rts", tmp_adid_str, ocpxKey);
    }

    if (getRedisGroupConfig("deliver")) {
      //redis_db_.Add("ad", tmp_adid_str, "sfst_rtlimit");
    }

    if (getRedisGroupConfig("consume")) {
      // redis_db_.Add("ad", tmp_adid_str, "pv_second");
      // redis_db_.Add("rts", tmp_adid_str, "realtime_remain");
    }
*/
    /*tmp_custid_str = boost::lexical_cast<string>(ad_list[i].customer_id);
    tmp_feedid_str = boost::lexical_cast<string>(ad_list[i].feedid);
    if (getRedisGroupConfig("card")) {
        redis_db_.Add(tmp_feedid_str);
    }*/

    return true;
}

/*template<>
bool RedisManager::Get(const std::string& type, const std::string& id, const
std::string& property, boost::unordered_map<string, string>& value) {
    return redis_db_.Get(type, id, property, value);
}*/
/*template<>
bool RedisManager::Get(const std::string& type, const std::string& id, const
std::string& property, std::tr1::unordered_map<string, string>& value) {
    return redis_db_.Get(type, id, property, value);
}*/


/*template <>
bool RedisManager::Get(const std::string& id, long& value) {
  return redis_db_.Get(id, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property,
                       boost::unordered_map<string, string>& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property,
                       boost::unordered_map<string, int>& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, int& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, std::string& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, float& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, double& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, std::vector<int>& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, uint32_t& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, long& value) {
  return redis_db_.Get(type, id, property, value);
}
template <>
bool RedisManager::Get(const std::string& type, const std::string& id,
                       const std::string& property, uint64_t& value) {
  return redis_db_.Get(type, id, property, value);
}
*/


RedisManager::RedisManager() {}
RedisManager::~RedisManager() {}

bool RedisManager::Update() {
  ifstream in(mRedisGroupConfigFile.c_str());
  if (!in) {
    ERROR(LOGNAME, "redis_group_config_file can't find file=%s",
          mRedisGroupConfigFile.c_str());
    return false;
  }

  int FIELD_SIZE = 2;
  string line;
  bool ret;
  while (getline(in, line)) {
    vector<string> tmp;
    AdUtil::Split2(tmp, line, '=');
    if ((int)tmp.size() != FIELD_SIZE) {
      ERROR(LOGNAME, "update_aux size()=%d for line=%s", (int)tmp.size(),
            line.c_str());
      continue;
    }
    long value = 0;
    if (!isLong(tmp[1], &value)) {
      ERROR(LOGNAME, "update_aux str=%s is not valid in line=%s",
            tmp[1].c_str(), line.c_str());
      continue;
    }
    DEBUG(LOGNAME, "redis group config [%s=%d]", tmp[0].c_str(), value);

    if (mRedisGroupConfig.find(tmp[0]) != mRedisGroupConfig.end())
      mRedisGroupConfig[tmp[0]] = (value == 1) ? true : false;
    else
      mRedisGroupConfig.insert(make_pair(tmp[0], (value == 1) ? true : false));
  }
  in.close();

  // for (map<string, bool>::iterator it=mRedisGroupConfig.begin();
  // it!=mRedisGroupConfig.end(); it++)
  //    DEBUG(LOGNAME, "redis group config [%s=%d]", it->first.c_str(),
  // it->second);
  return true;
}

bool RedisManager::getRedisGroupConfig(const char* group) {
  if (mRedisGroupConfig.find(group) == mRedisGroupConfig.end()) return true;
  return mRedisGroupConfig[group];
}

}  // namespace rank
