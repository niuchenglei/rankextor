#ifndef STRATEGY_MANAGER_H
#define STRATEGY_MANAGER_H

#include <string>
#include <vector>
#include "Interfaces/ConditionBase.h"
#include "Interfaces/StrategyBase.h"

namespace rank {
class StrategyManager {
 public:
  StrategyManager();
  ~StrategyManager();

  bool Initialize(const std::string& alias,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool Filter(const RankRequest& fisher_request,
              std::vector<RankAdInfo>& ad_list_for_rank,
              std::vector<RankItemInfo>& item_list_for_rank,
              RankDataContext& ptd, int after_model = 1);
  std::vector<std::string> getStrategies();

 private:
  std::string mAlias;
  std::string mStrategyPath, mGraphConfigFile;
  std::vector<StrategyBase*> mpFilters;
  std::vector<std::string> mStrategies;

  void clear();
};

}  // namespace rank
#endif
