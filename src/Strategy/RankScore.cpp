#include <fstream>
#include <sstream>
#include "Strategy/RankScore.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/RedisManager.h"

using namespace boost;

namespace rank {
REGISTE_CLASS("rankscore", RankScoreStrategy)

#define MIN_VAL 0.0000001
#define DEFAULT_CTR 0.00001

bool RankScoreStrategy::Initialize(
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

  srand((unsigned)time(NULL));
  return Update();
}

bool RankScoreStrategy::Filter(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {

  std::vector<RankAdInfo>& ad_list = ad_list_for_rank;
  std::vector<RankItemInfo>& item_list = item_list_for_rank;
  size_t ad_size = ad_list.size(), item_size = item_list.size(), pair_size = ptd.rank_order.size();

  for (int k=0; k<pair_size; k++) {
    ad_item_pair_t& _pair = ptd.rank_order[k];
    RankAdInfo& ad = ad_list[_pair.ad_idx];
    RankItemInfo& item = item_list[_pair.item_idx];

    _pair.score = 0; 
    _pair.bid_price = 0;

    if (item.filter_strategy != 10000)
      continue;
    if (ad.filter_strategy != 10000)
      continue;

    _pair.score = max(min(bid, (float)ad.max_bid_price), 5.0f);

    NOTICE(LOGNAME, "ad_id=%ld, item_id=%ld, score=%f, bid_type=%ld, bid=%ld, bid_max=%ld", 
           ad.ad_id, item.item_id, _pair.score, ad.bid_type, _pair.bid_price, ad.max_bid_price);
  }

  return true;
}

bool RankScoreStrategy::Update() {
  PairReader reader(mConfigFile);
  if (!reader.isValid()) {
    DEBUG(LOGNAME, "config file parse error [%s]", mConfigFile.c_str());
    return false;
  }

  mOptimizeOption = "max";  // budget,uv,roi
  reader.getValue("opt_option", mOptimizeOption);
  mOptimizeMethod = "pid";
  reader.getValue("opt_method", mOptimizeMethod);

  mPacingAlpha = 0.5; mCpcBeta = 0.5;
  reader.getValue("pacing_alpha", mPacingAlpha);
  reader.getValue("cpc_beta", mCpcBeta);

  return true;
}

}  // namespace rank
