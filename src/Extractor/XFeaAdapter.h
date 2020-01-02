#ifndef XFEA_ADAPTER_H
#define XFEA_ADAPTER_H

#include "Interfaces/FeatureExtractorBase.h"

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
#include <fstream>
#endif

#include "core/extractor.h"
#include "common/bisheng_common.h"

using xfea::bisheng::FeaResultSet;
using xfea::bisheng::fea_result_t;
using xfea::bisheng::ReturnCode;
using xfea::bisheng::RC_SUCCESS;
using xfea::bisheng::RC_ERROR;
using xfea::bisheng::RC_WARNING;

namespace rank {

class XFeaAdapter : public FeatureExtractorBase {
 public:
  OBJECT_CLASS(XFeaAdapter)

  XFeaAdapter();
  ~XFeaAdapter();

  bool Initialize(extractor_arg_t& arg,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool Transform(const RankRequest& fisher_request,
                 std::vector<RankAdInfo>& ad_list_for_rank,
                 std::vector<RankItemInfo>& item_list_for_rank,
                 RankDataContext& ptd);
  bool Update();
  const extractor_arg_t& getArgument() const {
    return mArg;
  };

private:
  extractor_arg_t mArg;
  std::string mFeaturePath, mModelPath, mConfig;
  // FeatureMapData* mpData;
  // FeatureGraph* mpFeatureGraph;
  xfea::bisheng::Extractor* extractor[10];
  std::map<int64_t, int> name_hash_map;

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
  std::ofstream _xfea_debug_output_ofs;
#endif
};
}  // namespace rank
#endif
