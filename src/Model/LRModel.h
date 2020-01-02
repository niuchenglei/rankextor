#ifndef LRMODEL_H
#define LRMODEL_H

#include <sstream>
#include <time.h>
//#include <sharelib/util/string_util.h>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/ModelBase.h"

namespace rank {
class lr_coeff_t : public Resource {
 public:
  virtual int Load(const std::string& config_file);
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  boost::unordered_map<int64_t, float> mModelWeights;  // m_featureIdToWeightMap;
  vector<int64_t> mFeatureKeys;
  float mDefaultWeight;

 private:
  std::string mModelFile;
  bool loadModel();
};
DEFINE_RESOURCE(lr_coeff_t);

/////////////////////////////////////////////////

class LRModel : public ModelBase {
 public:
  OBJECT_CLASS(LRModel)

  LRModel();
  ~LRModel();

  bool Initialize(model_arg_t& arg,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool Transform(const RankRequest& fisher_request,
                 std::vector<RankAdInfo>& ad_list_for_rank,
                 std::vector<RankItemInfo>& item_list_for_rank,
                 RankDataContext& ptd);
  bool Predict(const RankRequest& fisher_request,
               std::vector<RankAdInfo>& ad_list_for_rank,
               std::vector<RankItemInfo>& item_list_for_rank,
               RankDataContext& ptd);
  bool Update();
  const model_arg_t& getArgument() const {
    return mArg;
  };

 private:
  model_arg_t mArg;
  lr_coeff_t* mpCoeff;
  std::string mModelDir, mResourceKey, mPluginName;

  float wTx(const vector<long>& feature, lr_coeff_t& model);
};

}  // namespace rank
#endif
