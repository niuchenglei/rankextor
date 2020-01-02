#ifndef RESET_STRATEGY_H_
#define RESET_STRATEGY_H_

#include "Interfaces/StrategyBase.h"

namespace rank {
class ResetStrategy : public StrategyBase {
 public:
  OBJECT_CLASS(ResetStrategy)

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
};

}  // namespace rank
#endif
