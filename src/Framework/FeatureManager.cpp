#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Framework/FeatureManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Logger.h"
#include "Framework/graph_parse.h"

namespace rank {

//#define PROF
FeatureManager::FeatureManager() {}

FeatureManager::~FeatureManager() { clear(); }

bool FeatureManager::Initialize(
    const std::string& alias,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  Service<ConfigurationSettings> pSettings;
  Service<ObjectFactory> pFactory;

  mAlias = alias;
  mFeaturePath = pSettings->getSetting(alias + "/feature_dir");
  INFO(LOGNAME, "feature path:%s", mFeaturePath.c_str());

  mGraphConfigFile = pSettings->getSetting(alias + "/graph");
  INFO(LOGNAME, "graph config path:%s", mGraphConfigFile.c_str());

  if (!construct_feature(mGraphConfigFile, mFeaturePath, mpFetcher,
                         resource_manager, conditions)) {
    ERROR(LOGNAME, "load graph feature failed, config path:%s",
          mGraphConfigFile.c_str());
    return false;
  }

  return true;
}

bool FeatureManager::FetchFeature(
    RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  const Json::Value& rank_feature = ptd.strategy_json["rank_module_list"]["value"]["feature"];

  struct timespec start, end;
  long _ss = 0;

  for (Json::ArrayIndex j = 0; j < rank_feature.size(); ++j) {
    std::string feature_name = rank_feature[j].asString();
    vector<FeatureBase*>::const_iterator iter = mpFetcher.begin();
    bool run = false;

    for (; iter != mpFetcher.end(); iter++) {
      FeatureBase* fb = static_cast<FeatureBase*>(*iter);
#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &start);
#endif

      if (feature_name != fb->getArgument().name) {
        continue;
      }

      run = true;
      NOTICE(LOGNAME, "runing feature %s", feature_name.c_str());
      bool _run_ok = fb->FetchFeature(fisher_request, ad_list_for_rank, item_list_for_rank, ptd);

      if (!_run_ok) {
        ERROR(LOGNAME, "feature %s run failed", fb->getArgument().name.c_str());
        ptd.error_code = 200;
        ptd.error_msg = "feature " + feature_name + " run failed";
        return false;
      }
#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &end);
      long sum = (end.tv_sec - start.tv_sec) * 1000000 +
                 (end.tv_nsec - start.tv_nsec) / 1000;
      _ss += sum;
      FATAL(LOGNAME, "feature time: %s: %ld us", fb->getArgument().name.c_str(), sum);
#endif
    }  // for mpFetcher

    if (!run) {
      ERROR(LOGNAME, "feature %s not run", feature_name.c_str());
      ptd.error_code = 201;
      ptd.error_msg = "feature " + feature_name + " not run";
      return false;
    }
  }  // for rank_feature

#ifdef PROF
  FATAL(LOGNAME, "total feature time: %ld us", _ss);
#endif
  return true;
}

void FeatureManager::clear() {
  Service<ObjectFactory> pFactory;
  int len = mpFetcher.size();

  for (int k = 0; k < len; k++) {
    FeatureBase* obj = mpFetcher[k];
    pFactory->destroyObject(obj, obj->getRegistName());
  }
  mpFetcher.clear();
}

bool FeatureManager::Destroy() { }
}  // namespace rank
