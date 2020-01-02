#ifndef FMMODEL_H
#define FMMODEL_H

#include <sstream>
#include <time.h>
//#include <sharelib/util/string_util.h>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/ModelBase.h"

namespace rank {
class fm_coeff_t : public rank::Resource {
 public:
  virtual int Load(const std::string& config_file);
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  std::map<uint64_t, float> mModelWeights;
  float mConstWeight;
  bool mHasConstWeightFlag;
  int mFactor;
  uint64_t mFeatureNum;

 private:
  std::string mModelFile;
  bool loadModel();
};
DEFINE_RESOURCE(fm_coeff_t);

class FMModel : public ModelBase {
 public:
  OBJECT_CLASS(FMModel)

  FMModel();
  ~FMModel();

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
  fm_coeff_t* mpCoeff;
  std::string mModelDir, mResourceKey;

  float wTx(const vector<uint64_t>& feature, uint64_t feature_size, fm_coeff_t& model); //float* v1, float* v2, std::string& msg);
  //float wTx(const uint64_t* feature, uint64_t feature_size, fm_coeff_t& model, float* v1, float* v2, std::string& msg);
};
}  // namespace rank
#endif
