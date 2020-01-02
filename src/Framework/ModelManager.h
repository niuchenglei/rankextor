#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "Interfaces/ModelBase.h"
#include "Interfaces/ConditionBase.h"

namespace rank {
class ModelManager {
 public:
  ModelManager();
  ~ModelManager();

  bool Initialize(const std::string& alias,
                  const std::map<std::string, ConditionBase*>& conditions,
                  ResourceManager* resource_manager);
  bool Predict(const RankRequest& fisher_request,
               std::vector<RankAdInfo>& ad_list_for_rank,
               std::vector<RankItemInfo>& item_list_for_rank,
               RankDataContext& ptd);

 private:
  std::vector<ModelBase*> mpModels;
  std::string mModelPath, mGraphConfigFile;
  std::string mAlias;
  std::string mModelNames;
  // std::map<string, std::set<string> > mModelNameTops;
  std::set<std::string> mModelNamesSet;
  std::multimap<string, string> mModelNameTops;

  void clear();
};

}  // namespace rank
#endif
