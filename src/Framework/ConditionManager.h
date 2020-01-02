#ifndef CONDITION_MANAGER_H
#define CONDITION_MANAGER_H

#include <string>
#include <vector>
#include "Interfaces/ConditionBase.h"

namespace rank {
class ConditionManager {
 public:
  ConditionManager();
  ~ConditionManager();

  bool Initialize(const std::string& alias, ResourceManager* resource_manager);
  std::vector<std::string> Check(const RankAdInfo& adinfo,
                                 RankDataContext& ptd);
  ConditionBase* getCondition(const std::string& name);
  std::map<std::string, ConditionBase*>& getAllConditions() {
    return mpConditions;
  }

 private:
  std::map<std::string, ConditionBase*> mpConditions;
  std::string mAlias;
  std::string mGraphConfigFile;

  void clear();
};

}  // namespace rank
#endif
