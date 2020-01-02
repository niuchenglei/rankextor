#include <math.h>
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Strategy/Calibrate.h"

namespace rank {

REGISTE_CLASS("calibrate", CalibrateStrategy)

bool CalibrateStrategy::Initialize(
    strategy_arg_t& arg,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  StrategyBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mStrategyDir = pSetting->getSetting(alias + "/strategy_dir");

  mArg = arg;
  if (mArg.config != "" && mArg.config != " ")
    mConfigFile = mStrategyDir + "/" + mArg.config;
  else
    mConfigFile = mStrategyDir + "/" + mArg.name + ".txt";
  REGIST_HANDLER(mConfigFile);

  return Update();
}

bool CalibrateStrategy::Filter(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  DEBUG(LOGNAME, "calibrate run begin");
  // ------------- strategy begin -------------

  std::vector<RankAdInfo>& ad_list = ad_list_for_rank;
  std::vector<RankItemInfo>& item_list = item_list_for_rank;
  size_t ad_size = ad_list.size(), item_size = item_list.size();

  for (size_t i = 0; i < item_size; i++) {
    if (item_list[i].filter_strategy != 10000) 
      continue;

    uint64_t item_id = item_list[i].item_id;
    float pctr = item_list[i].ctr, pcvr = item_list[i].cvr, pctcvr = item_list[i].ctcvr;

    //uint32_t index = atoi(ad_list[i].extend["pre_sort_index"].c_str());
    //boost::unordered_map<int64_t, Features::Item>& features = ptd.features[index].features_group["begin"];
    //float hie_ctr = features[-1].value / 100000.0;  // features["hierarchy_smooth_ctr"].value/100000.0;

    // 1. pctr校正
    pctr *= 1.5f;
    item_list[i].ctr = pctr;

    // 2. 过滤掉此商品
    item_list[i].filter_strategy = getReason("ctr_too_low");
    item_list[i].filter_strategy = 123;

    // 3. 排序分数奖励
    item_list[i].score *= 1.4f;
    item_list[i].score = item_list[i].score * f1() + f2();
  }

  // ------------- strategy end -------------
  DEBUG(LOGNAME, "calibrate run end");

  return true;
}

bool CalibrateStrategy::Update() {
  PairReader reader(mConfigFile);
  if (!reader.isValid()) {
    DEBUG(LOGNAME, "calibrate strategy conf parse error [%s]", mConfigFile.c_str());
    return false;
  }

  ctr_thresh = -1.0; cvr_thresh = -1.0; ctcvr_thresh = -1.0; 
  reader.getValue("ctr_thresh", ctr_thresh);
  reader.getValue("cvr_thresh", cvr_thresh);
  reader.getValue("ctcvr_thresh", ctcvr_thresh);

  return true;
}

}  // namespace rank
