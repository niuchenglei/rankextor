#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Framework/ExtractorManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/Logger.h"
#include "Framework/graph_parse.h"

namespace rank {

ExtractorManager::ExtractorManager() {}
ExtractorManager::~ExtractorManager() { clear(); }

bool ExtractorManager::Initialize(
    const std::string& alias,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  Service<ConfigurationSettings> pSettings;
  Service<ObjectFactory> pFactory;

  mAlias = alias;
  mModelPath = pSettings->getSetting(alias + "/model_dir");
  INFO(LOGNAME, "model path:%s", mModelPath.c_str());

  mGraphConfigFile = pSettings->getSetting(alias + "/graph");
  INFO(LOGNAME, "graph config path:%s", mGraphConfigFile.c_str());

  if (!construct_extractor(mGraphConfigFile, mModelPath, mpExtractors,
                           resource_manager, conditions)) {
    ERROR(LOGNAME, "load graph model failed, config path:%s", mGraphConfigFile.c_str());
    return false;
  }
  int len = mpExtractors.size();

  mExtractorNames = "";
  for (int i = 0; i < len; i++) {
    FeatureExtractorBase* obj = mpExtractors[i];
    mExtractorNames += obj->getArgument().name + ",";
    mExtractorNameTops.insert(
        make_pair(obj->getArgument().top, obj->getArgument().name));
  }
  INFO(LOGNAME, "extractor list: %s", mExtractorNames.c_str());

  return true;
}

bool ExtractorManager::Transform(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  DEBUG(LOGNAME, "transform ads begin");

  struct timespec start, end;
  long _ss = 0;
  //{"pre_strategy":["flowsplit","discard","discard_class","optimized_reserved_price","recall","reset","creative_opt","filter"],"feature":["hierarchy_smooth_ctr","gender_plat_cust_feed_ctr","cust60_smooth_ctr","feedid_cate_info","basic_adausr_info","adfea","online_feature"],"extractor":["xfea_1","xfea_2"],"model":["mainbody","ocpx_ctr_realimp","ocpx_cvr_test","cpl_ctr_hour","cpl_cvr_hour","ocpm_convert","addfea_guide_ctr_ftrl","addfea_social_ctr_ftrl","addfea_read_ctr_ftrl","fusion-ftrl-paramter","ctr_ftrl_iv","xfea_igctr_ftrl"],"post_strategy":["calibratecpe","optimizer2","auto_bid","ocpmbid","ocpx_realimp","ocpx_cost_optimize","precedure","cpl","cpl_cost_optimize","rankscore","search","realtime_optimizer","vcgcost","optimized_cost","reward","discard","discard_class","optimized_reserved_price","recall","custguaranteed","deliver","repeatcust","garanteed","real_read","pred_ctx","flowmerge"]}
  
  const Json::Value& rank_extractor = ptd.strategy_json["rank_module_list"]["value"]["extractor"];
  for (Json::ArrayIndex j = 0; j < rank_extractor.size(); ++j) {
    bool run = false;
    std::string extractor_name = rank_extractor[j].asString();
    vector<FeatureExtractorBase*>::const_iterator iter = mpExtractors.begin();
    for (; iter != mpExtractors.end(); iter++) {
      FeatureExtractorBase* fb = static_cast<FeatureExtractorBase*>(*iter);
      if (fb->getArgument().name != extractor_name) {
        continue;
      }
      
#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &start);
#endif

      run = true;
      bool _run_ok = fb->Transform(fisher_request, ad_list_for_rank, item_list_for_rank, ptd);
      if (!_run_ok) {
        ERROR(LOGNAME, "extractor %s run failed", extractor_name.c_str());
        ptd.error_code = 300;
        ptd.error_msg = "extractor " + extractor_name + " run failed";
        return false;
      }

#ifdef PROF
      clock_gettime(CLOCK_MONOTONIC, &end);
      long sum = (end.tv_sec - start.tv_sec) * 1000000 +
                 (end.tv_nsec - start.tv_nsec) / 1000;
      _ss += sum;
      FATAL(LOGNAME, "feature factory time: %s: %ld us", fb->getArgument().name.c_str(), sum);
#endif
    }
    if (!run) {
      ERROR(LOGNAME, "feature extractor %s not run", extractor_name.c_str());
      ptd.error_code = 301;
      ptd.error_msg = "extractor " + extractor_name + " not run";
      return false;
    }
    //int64_t end_time = base::TimeUtil::CurrentTimeInMSec();
    //ptd.RecordStatsInfo(kExtractor, extractor_name, end_time - begin_time, 0);
  }

#ifdef PROF
  FATAL(LOGNAME, "total feature factory time: %ld us", _ss);
#endif

  DEBUG(LOGNAME, "transform ads end");
  return true;
}

void ExtractorManager::clear() {
  Service<ObjectFactory> pFactory;
  int len = mpExtractors.size();

  for (int i = 0; i < len; i++) {
    FeatureExtractorBase* obj = mpExtractors[i];
    pFactory->destroyObject(obj, obj->getRegistName());
  }
  std::vector<FeatureExtractorBase*>().swap(mpExtractors);  // mpModels.clear();
  mExtractorNames = "";
  std::map<string, string>().swap(mExtractorNameTops);
}
}  // namespace rank
