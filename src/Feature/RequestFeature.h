#ifndef REQUEST_FEATURE_H
#define REQUEST_FEATURE_H

#include "Interfaces/FeatureBase.h"

namespace rank {

class RequestFeature : public FeatureBase {
 public:
  OBJECT_CLASS(RequestFeature)

  RequestFeature();
  ~RequestFeature();

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
  std::string mFeaturePath, mResourceKey, mRedisSchemaFile;

  int mIndex;
  std::map<std::string, std::vector<std::string> > mUserRedisSchema[2];
  std::vector<std::string> mRedisFields[2];

  bool parse_user_data(const std::string& redis_user_data, UserInfo& user_info);
  bool parse_item_data(const std::string& redis_item_data, RankItemInfo& item_info);
  string empty_feature;
};
}  // namespace rank
#endif
