#ifndef BASIC_INFO_FEATURE_H
#define BASIC_INFO_FEATURE_H

#include "Interfaces/FeatureBase.h"

namespace rank {

class BasicFeature : public FeatureBase {
 public:
  OBJECT_CLASS(BasicFeature)

  BasicFeature();
  ~BasicFeature();

  bool Initialize(feature_arg_t& arg,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool FetchFeature(RankRequest& fisher_request,
                    std::vector<RankAdInfo>& ad_list_for_rank,
                    std::vector<RankItemInfo>& item_list_for_rank,
                    RankDataContext& ptd);
  bool Update();
  const feature_arg_t& getArgument() const {
    return mArg;
  };

 private:
  feature_arg_t mArg;
  std::string mFeatureDir, mModelDir;
  std::string mFeaturePath, mResourceKey, mFeatureMapFile;
  FeatureMapData* mpData;
};
}  // namespace rank
#endif
