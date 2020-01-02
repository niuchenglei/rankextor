#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <string>
#include <vector>
#include "Interfaces/FeatureBase.h"
#include "Interfaces/ConditionBase.h"

namespace rank {
class FeatureManager {
 public:
  FeatureManager();
  ~FeatureManager();

  bool Initialize(const std::string& alias,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);

  bool FetchFeature(RankRequest& fisher_request,
                    std::vector<RankAdInfo>& ad_list_for_rank,
                    std::vector<RankItemInfo>& item_list_for_rank,
                    RankDataContext& ptd);

  bool Destroy();
  void clear();

 private:
  std::vector<FeatureBase*> mpFetcher;
  std::string mFeaturePath, mGraphConfigFile;
  std::string mAlias;
};
}  // namespace rank

#endif
