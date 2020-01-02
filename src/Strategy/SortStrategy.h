#ifndef SORT_STRATEGY_H_
#define SORT_STRATEGY_H_

#include "Interfaces/StrategyBase.h"
#include "Strategy/PriceContext.h"

namespace rank {
class SortStrategy : public StrategyBase {
public:
  OBJECT_CLASS(SortStrategy)

  ~SortStrategy();
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
  std::string mFeatureDir, mStrategyDir, mConfigFile, price_ctx_key;
  PriceContext* mpPriceCtx;
};

}  // namespace rank
#endif
