#include "Interfaces/RedisManager.h"
#include "Utilities/AdUtil.h"
#include "Interfaces/Util.h"

using namespace boost;

namespace rank {

bool RedisHandler::Initialize(const std::string& redis_group_config) {
  handler_instance = NULL;

  mRedisGroupConfig.clear();
  mRedisGroupConfigFile = redis_group_config;
  REGIST_HANDLER(mRedisGroupConfigFile);
  return Update();
}

bool RedisHandler::Check(ResourceManager* resource_manager, int worker_id) {
  if (handler_instance) return true;
  handler_instance = NULL;

  if (0 == resource_manager->FetchResource("main_warehouse_redis_instance",
                                           &handler_instance, worker_id) &&
      handler_instance != NULL) {
    return true;
  }
  return false;
}

void RedisHandler::Clear() {
  std::map<std::string, std::string>().swap(datas);
  std::vector<std::string>().swap(keys);
}

bool RedisHandler::Fetch(
    const std::map<std::string, std::string>& rank_user_data) {
  for (size_t i = 0; i < keys.size(); i++) datas.insert(make_pair(keys[i], ""));

  string v = "";
  int ret = handler_instance->GetValue(&datas);
  /*for (size_t i=0; i<keys.size(); i++)
      datas.clear();
      datas.insert(make_pair(keys[i], ""));
      ret = handler_instance->GetValue(&datas)
  }*/

  // must be behind "handler_instance->GetValue(&datas)"
  datas.insert(rank_user_data.begin(), rank_user_data.end());

  // for(int i=0;i<keys.size(); i++) {
  //    std::map<std::string, std::string>::iterator iter = datas.find(keys[i]);
  //    if (iter != datas.end()) {
  //        v += keys[i]+"="+iter->second+",";
  //    }
  //}

  if (ret == 0) {
    NOTICE(LOGNAME, "Resource.GetValue(redis, %s)", v.c_str());
  } else {
    ERROR(LOGNAME, "error while Resource.GetValue(redis, %s)", v.c_str());
  }

  keys.clear();
  return (ret == 0) ? true : false;
}

#define __EXPAND_KEY__(A, B, C) \
  (string(A) + string(":") + B + string(":") + string(C))
bool RedisHandler::AddUserKey(const RankRequest& fisher_request,
                              UserInfo& user) {
  // string tmp_flag = "limitupdateflag",
  // uid = boost::lexical_cast<string>(fisher_request.uid),
  // group_id = boost::lexical_cast<string>(fisher_request.psid);

  // keys.push_back(__EXPAND_KEY__("user", uid, "com_ubi"));
  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_hgcpb"));
  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_ucrw"));
  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_stin"));
  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_clustertimes"));

  // keys.push_back(__EXPAND_KEY__("rts", uid, "feed_cust"));
  // keys.push_back(__EXPAND_KEY__("rts", tmp_flag, "sfst_upflag"));

  // keys.push_back(__EXPAND_KEY__("group", group_id, "tag"));

  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_clkcatekunguang"));
  // keys.push_back(__EXPAND_KEY__("un", uid, "un"));
  // keys.push_back(__EXPAND_KEY__("user", uid, "feed_pv"));
  // keys.push_back(__EXPAND_KEY__("u2", uid, "feedtpb"));
  // keys.push_back(__EXPAND_KEY__("u2", uid, "applist"));

  return true;
}

bool RedisHandler::AddAdidRealTimeKey(
    const std::vector<RankAdInfo>& ad_list) {
  size_t ad_size = ad_list.size();
  DEBUG(LOGNAME, "add adid redis key");
  if (!getRedisGroupConfig("ad_queue")) return true;

  string tmp_adid_str = "";
  string tmp_custid_str = "";
  string tmp_feedid_str = "";
  for (size_t i = 0; i < ad_size; i++) {
    if (ad_list[i].filter_strategy != 10000 && ad_list[i].filter_strategy != 0)
      continue;
    try {
      tmp_adid_str = boost::lexical_cast<string>(ad_list[i].ad_id);
      tmp_custid_str = boost::lexical_cast<string>(ad_list[i].customer_id);
      // tmp_feedid_str =
      // boost::lexical_cast<string>(ad_list[i].creative_list[0].creative_id);

      if (getRedisGroupConfig("rts")) {
        DEBUG(LOGNAME, "add rts redis key");
        keys.push_back(__EXPAND_KEY__("rts", tmp_adid_str, "sfst_impbhv"));
        keys.push_back(__EXPAND_KEY__("rts", tmp_adid_str, "sfst_cimpbhv"));

        // keys.push_back(__EXPAND_KEY__("rts", tmp_adid_str, "ocpx_info"));

        //增加OCPX中当天实时数据的Key
        struct tm* currentTime;
        time_t t;
        t = time(NULL);
        currentTime = localtime(&t);
        int todayDate = (currentTime->tm_year + 1900) * 10000 +
                        (currentTime->tm_mon + 1) * 100 + currentTime->tm_mday;
        string ocpxKey = "ocpx_info:" + boost::lexical_cast<string>(todayDate);
        keys.push_back(__EXPAND_KEY__("rts", tmp_adid_str, ocpxKey));
      }
      if (getRedisGroupConfig("ad")) {
        DEBUG(LOGNAME, "add ad redis key");
        keys.push_back(__EXPAND_KEY__("ad", tmp_adid_str, "sfst_rtlimit"));
        // keys.push_back(__EXPAND_KEY__("ad", tmp_adid_str, "sc"));
      }

      // keys.push_back(__EXPAND_KEY__("rts", tmp_adid_str, "realtime_remain"));
    }
    catch (...) {
      FATAL(LOGNAME, "error string type, adid: %ld, custid: %ld",
            ad_list[i].ad_id, ad_list[i].customer_id);
    }
  }
  keys.push_back("brand_smooth_all_hot");//获取品速广告的紧迫度数据
  return true;
}
#undef __EXPAND_KEY__

#define __GET_VALUE_FROM_MAP__(KEY)         \
  std::string redis_value;                  \
  if (!Get(KEY, redis_value)) return false; \
  if (redis_value == "") return false;

template <>
bool RedisHandler::Get(const std::string& key, std::string& value) {
  if (datas.find(key) == datas.end()) return false;
  value = datas[key];
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, long& value) {
  __GET_VALUE_FROM_MAP__(id);
  value = strtol(redis_value.c_str(), NULL, 10);
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, double& value) {
  __GET_VALUE_FROM_MAP__(id);
  value = strtod(redis_value.c_str(), NULL);
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, std::vector<std::string>& value) {
  __GET_VALUE_FROM_MAP__(id);
  std::istringstream is(redis_value);
  std::string element;
  while (getline(is, element, ',')) value.push_back(element);
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, std::vector<int32_t>& value) {
  __GET_VALUE_FROM_MAP__(id);
  std::istringstream is(redis_value);
  std::string element;
  while (getline(is, element, ','))
    value.push_back(strtol(element.c_str(), NULL, 10));
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, int& value) {
  __GET_VALUE_FROM_MAP__(id);
  value = strtol(redis_value.c_str(), NULL, 10);
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, unsigned int& value) {
  __GET_VALUE_FROM_MAP__(id);
  value = strtol(redis_value.c_str(), NULL, 10);
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id, std::vector<double>& value) {
  __GET_VALUE_FROM_MAP__(id);
  std::istringstream is(redis_value);
  std::string element;
  while (getline(is, element, ','))
    value.push_back(strtod(element.c_str(), NULL));
  return true;
}
template <>
bool RedisHandler::Get(const std::string& id,
                       boost::unordered_map<std::string, std::string>& value) {
  __GET_VALUE_FROM_MAP__(id);
  std::istringstream is(redis_value);
  std::string mkv;
  size_t pos;
  while (getline(is, mkv, ',')) {
    if ((pos = mkv.find(':')) == std::string::npos) continue;
    value[mkv.substr(0, pos)] = mkv.substr(pos + 1);
  }
  return true;
}

#undef __GET_VALUE_FROM_MAP__
bool RedisHandler::Update() {
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

bool RedisHandler::getRedisGroupConfig(const char* group) {
  if (mRedisGroupConfig.find(group) == mRedisGroupConfig.end()) return true;
  return mRedisGroupConfig[group];
}

}  // namespace rank
