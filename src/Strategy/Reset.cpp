#include "Strategy/Reset.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"

using namespace boost;

namespace rank {
REGISTE_CLASS("reset", ResetStrategy)

bool ResetStrategy::Initialize(
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
  // REGIST_HANDLER(mConfigFile);
  return Update();
}

bool ResetStrategy::Filter(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  DEBUG(LOGNAME, "reset begin");

  // 此策略负责初始化context上下文，读取redis等
  // 可加入截断竞价队列功能
  long tu = ptd.reset(fisher_request, ad_list_for_rank, item_list_for_rank);

  DEBUG(LOGNAME, "reset end");
  return true;
}

bool ResetStrategy::Update() { return true; }

}  // namespace rank
