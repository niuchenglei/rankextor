#ifndef RANK_DATA_CONTEXT_H
#define RANK_DATA_CONTEXT_H

#include <time.h>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "json/json.h"
#include "Interfaces/CommonType.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/PredictContext.h"
#include "Interfaces/RedisManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/Util.h"
#include "Utilities/AdUtil.h"

#include "Interfaces/rank_interface.pb.h"

using std::make_pair;
using std::vector;

namespace rank {

enum StatsInfoType {
  kPreStrategy,
  kFeature,
  kExtractor,
  kModel,
  kPostStrategy
};

class RankDataContext {
 public:
  RankDataContext() {
    resource_manager = NULL;
  }
  /*bool init(const std::string& redis_group_config) {
    return redis_handler.Initialize(redis_group_config);
  }*/
  void set_resource(ResourceManager* _resource_manager, int _worker_id) {
    resource_manager = _resource_manager;
    worker_idx = _worker_id;
    
    //Service<ConfigurationSettings> pSetting;
    //string _name = pSetting->getSetting("main/plugin_name");
  }

  inline void resize(size_t size) {
    std::vector<Features>().swap(features);
    features.resize(size);

    std::vector<ad_item_pair_t>().swap(rank_order);
  }
  long reset(const RankRequest& fisher_request,
             std::vector<RankAdInfo>& ad_list, 
             std::vector<RankItemInfo>& item_list) {
    size_t ad_size = ad_list.size();
    size_t item_size = item_list.size();

    struct timespec start, end;
    long sum = 0;

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif
    resize(item_size);

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &end);
    sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    FATAL(LOGNAME, "reset resize: %ld us (%d,%d)", sum, ad_size, item_size);
#endif

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif
    error_code = 0;
    error_msg = "";

    // 初始化特征数据
    //boost::unordered_map<int64_t, Features::Item> empty_map;
    //boost::unordered_map<int64_t, Features::Item>().swap(empty_map);

    for (int i = 0; i < item_size; i++) {
      if (item_list[i].filter_strategy != 10000 && item_list[i].filter_strategy != 0)
        continue;
      features[i].id = item_list[i].item_id;
      boost::unordered_map<std::string, boost::unordered_map<int64_t, Features::Item> >().swap(features[i].features_group);
    
      boost::unordered_map<int64_t, Features::Item>& empty_map = features[i].features_group["begin"];
    }

    for (int i = 0; i < ad_size; i++) {
      ad_list[i].ctr = 0;
      ad_list[i].score = 0;
      ad_list[i].cost = ad_list[i].bid_price * 0.8;
      //if (ad_list[i].bid_price <= 0 && ad_list[i].is_garanteed_delivery == 0)
      //  FATAL(LOGNAME, "fatal error: adid=%ld, bid_price=%ld", ad_list[i].ad_id, ad_list[i].bid_price);

      if (ad_list[i].filter_strategy != 10000 && ad_list[i].filter_strategy != 0)
        continue;

      boost::unordered_map<int64_t, int>& ava_items = ad_list[i].available_items;
      for (boost::unordered_map<int64_t, int>::iterator iter=ava_items.begin(); iter!=ava_items.end(); iter++) {
        if (iter->second >= item_list.size()) continue;
        if (item_list[iter->second].filter_strategy != 10000 && 
            item_list[iter->second].filter_strategy != 0)
          continue;

        ad_item_pair_t _t; _t.ad_id = ad_list[i].ad_id; _t.ad_idx = i; _t.item_id = iter->first; _t.item_idx = iter->second; _t.score = 0; _t.filter_strategy = 10000; _t.cost = 0;
        rank_order.push_back(_t);
      }

      // 初始化adlist里面的extend内容，目前包括：filter、tuning
      //ad_list[i].extend.insert(make_pair("filter_reason", "0"));
      //ad_list[i].extend.insert(make_pair("tuning", ""));
      //v = boost::lexical_cast<string>(i);
      //ad_list[i].extend.insert(make_pair("pre_sort_index", v));
      //ad_list[i].extend.insert(make_pair("aft_sort_index", "0"));
    }
    
#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &end);
    sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    FATAL(LOGNAME, "reset adlist foreach: %ld us", sum);
#endif

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif
    /*// 获取redis数据
    if (!redis_handler.Check(resource_manager, this->worker_idx)) {
      FATAL(LOGNAME,
            "RedisHandler initialize error (fetch instance from resource "
            "manager with NULL)");
      return 0;
    }*/
    //redis_handler.Clear();
    //redis_handler.AddAdidRealTimeKey(ad_list);

    clock_gettime(CLOCK_MONOTONIC, &start);
    //bool ret = redis_handler.Fetch(fisher_request.rank_user_data);
    //redis_fail_data[0] = (ret == true) ? 0 : 1;
    clock_gettime(CLOCK_MONOTONIC, &end);
    this->timeuse = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &end);
    sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    FATAL(LOGNAME, "reset redis: %ld us", sum);
#endif

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif
    
    //fillUserFeatureFromRedis(fisher_request);
    //fillAdInfoFromRedis(ad_list);
#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &end);
    sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    FATAL(LOGNAME, "reset fill data: %ld us", sum);
#endif

    return this->timeuse;
  }

  bool SetRuningStrategy(const string& runing_strategy) {
    return reader.parse(runing_strategy, strategy_json);
  }

  int worker_idx, max_queue_length;
  UserInfo user;                              // 用户特征
  //std::vector<AdsFeatures> ads_trace_data;  // 广告特征
  //std::vector<RealtimeData> realtime_data;  // 广告的实时特征
  Features common_features;                   // 共享特征
  std::vector<Features> features;             // 特征（gbdt编码等）
  // RedisManager* redis_manager;
  //RedisHandler redis_handler;
  ResourceManager* resource_manager;
  long timeuse;
  int error_code;
  std::string error_msg; 
 
  std::vector<ad_item_pair_t> rank_order;

  Json::Value strategy_json;  // 上游模块下发的动态配置策略信息, json，配置了需要运行的模块名称
  Json::Reader reader;
};

}  // namespace rank

#endif
