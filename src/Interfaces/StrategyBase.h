#ifndef STRATEGY_BASE_H
#define STRATEGY_BASE_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/PairReader.h"
#include "Interfaces/ConditionBase.h"

namespace rank {

#define TUNING "tuning"
/*
 * 策略参数结构体，从protobuffer解析而来
 */
typedef struct {
  std::string name;      // 策略名称
  std::vector<int> ids;  // 策略id，用于记录ad被策略奖励／惩罚情况
  std::map<std::string, int> desc;  // 策略说明
  int order;                        // 策略执行顺序
  std::string config;               // 策略配置文件
  std::string condition;            // 策略执行的条件
  std::string classtype;  // 若配置此项，则由此项对应实现类，否则由name字段对应
} strategy_arg_t;

class StrategyBase : public AutoUpdate {
 public:
  virtual bool Initialize(
      strategy_arg_t& arg,
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
      NOTICE(LOGNAME, "strategy:%s -> %s [%d]", arg.name.c_str(), arg.condition.c_str(), mpConditions.size());
    } else
      NOTICE(LOGNAME, "strategy:%s -> all", arg.name.c_str());
    return true;
  }
  virtual bool Filter(
      const RankRequest& fisher_request,
      std::vector<RankAdInfo>& ad_list_for_rank,
      std::vector<RankItemInfo>& item_list_for_rank,
      RankDataContext& ptd) = 0;

  virtual strategy_arg_t& getArgument() = 0;

  virtual int getReason(int idx) {
    const strategy_arg_t& arg = getArgument();
    if (idx >= arg.ids.size() || idx == 0) return 10000;
    int v = arg.ids[idx];
    if (arg.order > 0) v += 100000;
    return v;
  }
  
  virtual int getReason(std::string idx_name) {
    const strategy_arg_t& arg = getArgument();
    std::map<std::string, int>::const_iterator iter = arg.desc.find(idx_name);
    if (iter == arg.desc.end()) return 10000;
    int idx = iter->second;
    idx = arg.ids[idx];
    if (arg.order > 0) idx += 100000;
    // FATAL(LOGNAME, "%s -> %d,%d", idx_name.c_str(), idx, arg.ids.size());
    return idx;
  }

  bool CheckCondition(const RankAdInfo& adinfo, RankDataContext& ptd,
                      const RankRequest* fisher_request = NULL,
                      const std::string& condition_name = "") {
    if (condition_name != "" && !condition_name.empty()) {
      std::map<std::string, ConditionBase*>::const_iterator iter =
          mpConditionsDict->find(condition_name);
      if (iter == mpConditionsDict->end()) return false;
      return iter->second->Check(adinfo, ptd);
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
