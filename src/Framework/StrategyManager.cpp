#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
//#include "base/time_util/time_util.h"
#include "Framework/StrategyManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Logger.h"
#include "Framework/graph_parse.h"

namespace rank {

StrategyManager::StrategyManager() {}

StrategyManager::~StrategyManager() { clear(); }

bool StrategyManager::Initialize(
    const std::string& alias,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  DEBUG(LOGNAME, "strategy init begin");
  Service<ConfigurationSettings> pSettings;
  Service<ObjectFactory> pFactory;

  mAlias = alias;
  mStrategies.clear();

  mStrategyPath = pSettings->getSetting(alias + "/strategy_dir");
  mGraphConfigFile = pSettings->getSetting(alias + "/graph");
  INFO(LOGNAME, "graph config path:%s", mGraphConfigFile.c_str());

  if (!construct_strategy(mGraphConfigFile, mStrategyPath, mpFilters,
                          resource_manager, conditions)) {
    ERROR(LOGNAME, "load graph strategy failed, config path:%s",
          mGraphConfigFile.c_str());
    return false;
  }

  string strategy_list = "";
  for (int i = 0; i < mpFilters.size(); i++) {
    const strategy_arg_t& arg = mpFilters[i]->getArgument();
    mStrategies.push_back(arg.name);
    strategy_list += mStrategies[i];
    if (i != mpFilters.size() - 1) 
      strategy_list += ",";
  }
  DEBUG(LOGNAME, "find %d strategies, they are %s", mStrategies.size(), strategy_list.c_str());
  DEBUG(LOGNAME, "strategy init end");
  return true;
}

bool StrategyManager::Filter(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd, int after_model) {

  //StatsInfoType stats_info_type = kPreStrategy;
  Json::Value& rank_strategy = ptd.strategy_json["rank_module_list"]["value"]["pre_strategy"];
  if (after_model == 1) {
    rank_strategy = ptd.strategy_json["rank_module_list"]["value"]["post_strategy"];
    //stats_info_type = kPostStrategy;
  }

  bool flag = true;
  int current_index = (after_model < 0) ? -50 : 0, upper = (after_model < 0) ? 0 : 100;
  struct timespec start, end;
  long _ss = 0;
  
  //while (current_index <= upper) {
  for (Json::ArrayIndex j = 0; j < rank_strategy.size(); ++j) {
    bool run = false;
    std::string strategy_name = rank_strategy[j].asString();

    //int64_t begin_time = base::TimeUtil::CurrentTimeInMSec();
    vector<StrategyBase*>::const_iterator iter = mpFilters.begin();
    for (; iter != mpFilters.end(); iter++) {
      StrategyBase* sb = static_cast<StrategyBase*>(*iter);
      const strategy_arg_t& arg = sb->getArgument();
      if (arg.order >= 0 && after_model < 0) continue;
      if (arg.order < 0 && after_model > 0) continue;
      //if (arg.order != current_index)
      //    continue;

      if (strategy_name != arg.name) {
        continue;
      }

#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &start);
#endif

      run = true;
      NOTICE(LOGNAME, "runing strategy %s", strategy_name.c_str());
      bool _run_ok = sb->Filter(fisher_request, ad_list_for_rank, item_list_for_rank, ptd);

#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &end);
      long sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
      _ss += sum;
      FATAL(LOGNAME, "strategy time: %s: %ld us", arg.name.c_str(), sum);
#endif

      if (!_run_ok) {
        ERROR(LOGNAME, "strategy %s run failed", strategy_name.c_str());
        ptd.error_code = (after_model<0)?100:500 + 0;
        ptd.error_msg = "strategy " + strategy_name + " run failed";
        return false;
      }
    }  // for mpFilters

    if (!run) {
      ERROR(LOGNAME, "strategy %s not run", strategy_name.c_str());
      ptd.error_code = (after_model<0)?100:500 + 1;
      ptd.error_msg = "strategy " + strategy_name + " not run";
      return false;
    }
    //int64_t end_time = base::TimeUtil::CurrentTimeInMSec();
    //ptd.RecordStatsInfo(stats_info_type, strategy_name, end_time - begin_time, 0);
    //current_index += 1;
  }  // for rank_strategy
#ifdef PROF
  FATAL(LOGNAME, "total strategy time: %ld us", _ss);
#endif
  return true;
}

std::vector<std::string> StrategyManager::getStrategies() {
  return mStrategies;
}

void StrategyManager::clear() {
  Service<ObjectFactory> pFactory;
  int len = mpFilters.size();

  for (int k = 0; k < len; k++) {
    StrategyBase* obj = mpFilters[k];
    pFactory->destroyObject(obj, obj->getRegistName());
  }
  mpFilters.clear();
  mStrategies.clear();
}
}  // namespace rank
