#ifndef GRAPH_PARSE_H
#define GRAPH_PARSE_H

#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Interfaces/FeatureBase.h"
#include "Interfaces/ModelBase.h"
#include "Interfaces/StrategyBase.h"
#include "Interfaces/ConditionBase.h"
#include "Interfaces/FeatureExtractorBase.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Logger.h"

namespace rank {
bool construct_condition(const std::string& filename,
                         std::vector<ConditionBase*>& conditions,
                         ResourceManager* resource_manager);
bool construct_feature(const std::string& filename,
                       const std::string& feature_path,
                       std::vector<FeatureBase*>& features,
                       ResourceManager* resource_manager,
                       const std::map<std::string, ConditionBase*>& conditions);
bool construct_extractor(
    const std::string& filename, const std::string& feature_path,
    std::vector<FeatureExtractorBase*>& features,
    ResourceManager* resource_manager,
    const std::map<std::string, ConditionBase*>& conditions);
bool construct_model(const std::string& filename, const std::string& model_path,
                     std::vector<ModelBase*>& models,
                     ResourceManager* resource_manager,
                     const std::map<std::string, ConditionBase*>& conditions);
bool construct_strategy(
    const std::string& filename, const std::string& strategy_path,
    std::vector<StrategyBase*>& strategies, ResourceManager* resource_manager,
    const std::map<std::string, ConditionBase*>& conditions);

}  // namespace rank
#endif
