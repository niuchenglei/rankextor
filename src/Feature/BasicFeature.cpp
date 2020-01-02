#include <iostream>
#include "Feature/BasicFeature.h"
using namespace boost;

// custid,adid,feedid,label,age,gender,platform,phone_brand,location,network_type,pv_hour,bid_type

namespace rank {

REGISTE_CLASS("basic_info", BasicFeature)

BasicFeature::BasicFeature() {}
BasicFeature::~BasicFeature() {}

bool BasicFeature::Initialize(
    feature_arg_t& arg, const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  FeatureBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mFeatureDir = pSetting->getSetting(alias + "/feature_dir");
  mModelDir = pSetting->getSetting(alias + "/model_dir");
  string _name = pSetting->getSetting("main/plugin_name");
  mArg = arg;
  srand((unsigned)time(NULL));

  if (mArg.type == "FILE") {
    mFeatureMapFile = mFeatureDir + "/" + mArg.data["map_file"];

    // FATAL(LOGNAME, "registe resource %s, rely on %s.touch",
    // mFeatureMapFile.c_str(), mFeatureMapFile.c_str());
  } else {
    ERROR(LOGNAME, "config from redis");
    return false;
  }

  mpData = NULL;
  __F("uid");
  __F("custid");
  __F("adid");
  __F("feedid");
  __F("label");
  __F("age");
  __F("ages");
  __F("gender");
  __F("platform");
  __F("phone");
  __F("location");
  __F("network");
  __F("hour");
  __F("bidtype");
  __F("type");
  __F("style");
  __F("link");
  __F("show");
  __F("appid");
  __F("offset");
  __F("feedid");
  __F("userfreq");
  __F("psid");
  __F("promotion");
  __F("lifestat");
  __F("position");
  __F("mid");

  __F("---------------------------");
  __F("put any thing here you want");
  __F("---------------------------");
  // mpList["custid,adid,feedid,label,age,gender,platform,phone_brand,location,network_type,pv_hour,bid_type
  //

  return Update();
}

bool BasicFeature::FetchFeature(
    RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {

  ADD_COMMON_FEATURE("age", fisher_request.user_info.age);
  ADD_COMMON_FEATURE("location", fisher_request.user_info.location);
  ADD_COMMON_FEATURE("interest", redis["interest"]);
  ADD_COMMON_FEATURE("realtime_bhv", file["interest"]);

  int size = item_list_for_rank.size();
  int omp_thread_num = NUM_THREAD;
  if (omp_thread_num*3 > size)
    omp_thread_num = float(size)/3.0;
  int m = 0;

#ifdef OMP
#pragma omp parallel for num_threads(omp_thread_num) shared(item_list_for_rank, ptd) private(m)
#endif
  for (m=0; m<size; m++) {
    RankItemInfo& item = item_list_for_rank[m];

#ifdef FAKE_FEATURE
    FAKE_ITEM(m, "item_id", "10000027224");
    FAKE_ITEM(m, "item_category1", "-");
    FAKE_ITEM(m, "item_category2", "-");
    FAKE_ITEM(m, "item_category3", "-");
    FAKE_ITEM(m, "item_property", "-");
#else
    ADD_FEATURE(m, "item_id", item.item_id);

    // 物料特征
    ADD_FEATURE(m, "item_category1", item.extend["item_category1"]);
    ADD_FEATURE(m, "item_category2", item.extend["item_category2"]);
    ADD_FEATURE(m, "item_category3", item.extend["item_category3"]);
    ADD_FEATURE(m, "item_property", redis["category_4"]);
#endif
  }

  return true;
}

bool BasicFeature::Update() { return true; }

}  // namespace rank
