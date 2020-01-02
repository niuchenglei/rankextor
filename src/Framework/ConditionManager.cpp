#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Framework/ConditionManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Logger.h"
#include "Framework/graph_parse.h"

namespace rank {

ConditionManager::ConditionManager() {}

ConditionManager::~ConditionManager() { clear(); }

bool ConditionManager::Initialize(const std::string& alias,
                                  ResourceManager* resource_manager) {
  Service<ConfigurationSettings> pSettings;
  Service<ObjectFactory> pFactory;

  mAlias = alias;
  mGraphConfigFile = pSettings->getSetting(alias + "/graph");
  INFO(LOGNAME, "graph config path:%s", mGraphConfigFile.c_str());

  std::vector<ConditionBase*> cds;
  if (!construct_condition(mGraphConfigFile, cds, resource_manager)) {
    ERROR(LOGNAME, "load graph condition failed, config path:%s",
          mGraphConfigFile.c_str());
    return false;
  }

  for (int i = 0; i < cds.size(); i++) {
    mpConditions.insert(make_pair(cds[i]->name(), cds[i]));
  }

  return true;
}

std::vector<std::string> ConditionManager::Check(const RankAdInfo& adinfo,
                                                 RankDataContext& ptd) {
  std::vector<std::string> v;
  return v;
}

ConditionBase* ConditionManager::getCondition(const std::string& name) {
  for (map<std::string, ConditionBase*>::iterator iter = mpConditions.begin();
       iter != mpConditions.end(); iter++) {
    if (iter->first == name) return iter->second;
  }
  return NULL;
}

void ConditionManager::clear() {
  Service<ObjectFactory> pFactory;
  int len = mpConditions.size();

  for (map<std::string, ConditionBase*>::iterator iter = mpConditions.begin();
       iter != mpConditions.end(); iter++) {
    ConditionBase* obj = iter->second;
    pFactory->destroyObject(obj, obj->getRegistName());
  }
  mpConditions.clear();
}
}  // namespace rank
