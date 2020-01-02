#ifndef EXTRACTOR_MANAGER_H
#define EXTRACTOR_MANAGER_H

#include <string>
#include <vector>
#include "Interfaces/ModelBase.h"
#include "Interfaces/ConditionBase.h"
#include "Interfaces/FeatureExtractorBase.h"

namespace rank {
class ExtractorManager {
 public:
  ExtractorManager();
  ~ExtractorManager();

  bool Initialize(const std::string& alias,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);

  bool Transform(const RankRequest& fisher_request,
                 std::vector<RankAdInfo>& ad_list_for_rank,
                 std::vector<RankItemInfo>& item_list_for_rank,
                 RankDataContext& ptd);

 private:
  std::vector<FeatureExtractorBase*> mpExtractors;
  std::string mAlias, mGraphConfigFile, mModelPath;
  std::string mExtractorNames;
  std::map<string, string> mExtractorNameTops;

  void clear();
};

}  // namespace rank
#endif
