#ifndef MODEL_BASE_H
#define MODEL_BASE_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/ConditionBase.h"

namespace rank {

/*
 * 模型参数结构体，从protobuffer解析而来，包含了模型依赖的输入，以及输出，计算ctr时的权重等
 */
typedef struct {
  std::string name;  // 模型名称
  std::string type;  // 模型类型，必须是已经注册到工厂中的，如:lr/gbdt等
  std::string top;  // 模型的输出，一般与模型名称保持一致
  std::vector<std::string> bottom;  // 模型依赖的输入，可以有多个
  std::string model_file;           // 模型文件路径
  std::string map_file;             // 编号文件路径
  float weight;
  uint32_t
      offset;  // 模型做特征变换时，变换后特征的起始编号（用于多个模型输出给一个模型，如：使用gbdt和nn做特征编号，输出给lr计算ctr）
  std::string role;
  std::string condition;
  std::string features;
  std::string formula;
  float sample_ratio;
} model_arg_t;

class ModelBase : public AutoUpdate {
 public:
  virtual bool Initialize(
      model_arg_t& arg, const std::map<std::string, ConditionBase*>& conditions,
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
      NOTICE(LOGNAME, "model:%s -> %s [%d,%x]", arg.name.c_str(),
             arg.condition.c_str(), mpConditions.size(), &mpConditions);
    } else
      NOTICE(LOGNAME, "model:%s -> all", arg.name.c_str());
    return true;
  }

  // 特征变换，只做特征变换，不计算ctr
  virtual bool Transform(
      const RankRequest& fisher_request,
      std::vector<RankAdInfo>& ad_list_for_rank,
      std::vector<RankItemInfo>& item_list_for_rank,
      RankDataContext& ptd) = 0;
  // 计算ctr，只计算ctr
  virtual bool Predict(
      const RankRequest& fisher_request,
      std::vector<RankAdInfo>& ad_list_for_rank,
      std::vector<RankItemInfo>& item_list_for_rank,
      RankDataContext& ptd) = 0;
  virtual const model_arg_t& getArgument() const = 0;

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
