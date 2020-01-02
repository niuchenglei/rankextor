#ifndef FEATURE_EXTRACTOR_BASE_H
#define FEATURE_EXTRACTOR_BASE_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/ConditionBase.h"
#include "Interfaces/FeatureBase.h"

namespace rank {

typedef struct {
  std::string name;  // 特征名称
  std::string condition;
  std::string type;
  std::vector<std::string> bottom;
  std::string top;
  std::string config;
  std::string comment;
} extractor_arg_t;

class FeatureExtractorBase : public AutoUpdate {
 public:
  virtual bool Initialize(
      extractor_arg_t& arg,
      const std::map<std::string, ConditionBase*>& conditions,
      ResourceManager* resource_manager) {
    mpConditionsDict = &conditions;
    if (arg.condition != "" && !arg.condition.empty()) {
      vector<string> tmp;
      AdUtil::Split3(tmp, arg.condition, ',');
      for (int i = 0; i < tmp.size(); i++) {
        if (mpConditionsDict->find(tmp[i]) == mpConditionsDict->end()) {
          ERROR(LOGNAME, "condition %s not exist in condition manager",
                tmp[i].c_str());
          return false;
        }
        mpConditions.push_back(tmp[i]);
      }
      NOTICE(LOGNAME, "feature:%s -> %s [%d]", arg.name.c_str(),
             arg.condition.c_str(), mpConditions.size());
    } else
      NOTICE(LOGNAME, "feature:%s -> all", arg.name.c_str());
    return true;
  }
  virtual bool Transform(
      const RankRequest& fisher_request,
      std::vector<RankAdInfo>& ad_list_for_rank,
      std::vector<RankItemInfo>& item_list_for_rank,
      RankDataContext& ptd) = 0;
  virtual const extractor_arg_t& getArgument() const = 0;

  bool CheckCondition(const RankAdInfo& adinfo, RankDataContext& ptd,
                      const RankRequest* fisher_request = NULL,
                      const std::string& condition_name = "") {
    if (condition_name != "" && !condition_name.empty()) {
      std::map<std::string, ConditionBase*>::const_iterator iter =
          mpConditionsDict->find(condition_name);
      if (iter == mpConditionsDict->end()) return false;
      return iter->second->Check(adinfo, ptd, fisher_request);
    }

    if (mpConditions.size() == 0) return true;

    for (int k = mpConditions.size() - 1; k >= 0; k--) {
      std::map<std::string, ConditionBase*>::const_iterator iter =
          mpConditionsDict->find(mpConditions[k]);
      if (iter == mpConditionsDict->end()) continue;
      ConditionBase* cb = iter->second;
      if (cb == NULL) continue;
      if (cb->Check(adinfo, ptd, fisher_request)) return true;
    }
    // 一个也没有匹配上条件，返回false
    return false;
  }

 protected:
  std::vector<std::string> mpConditions;
  const std::map<std::string, ConditionBase*>* mpConditionsDict;
};

}  // namespace rank

#endif
