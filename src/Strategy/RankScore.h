#ifndef RANKSCORE_STRATEGY_H_
#define RANKSCORE_STRATEGY_H_

#include "Interfaces/StrategyBase.h"

namespace rank {
class RankScoreStrategy : public StrategyBase {
public:
  OBJECT_CLASS(RankScoreStrategy)

  bool Initialize(strategy_arg_t& arg,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool Filter(const RankRequest& fisher_request,
              std::vector<RankAdInfo>& ad_list_for_rank,
              std::vector<RankItemInfo>& item_list_for_rank,
              RankDataContext& ptd);
  bool Update();
  strategy_arg_t& getArgument() {
    return mArg;
  };

 private:
  strategy_arg_t mArg;
  std::string mFeatureDir, mStrategyDir, mConfigFile;

  std::string mOptimizeOption, mOptimizeMethod;
  float mPacingAlpha, mCpcBeta;
  //mCPMWeight, mCPEWeight, mOCPMWeight, mCPMQ, mCPEQ, mOCPMQ, mMaxCPEScore, mMaxCPECTR, mOptimizerFlag, mPrecedureFlag;
  //float ocpm_oscore_scale;
  //float cpm_ctr_thresh, cpe_ctr_thresh, ocpm_ctr_thresh, cpc_ctr_thresh;
};
}  // namespace rank

#endif
