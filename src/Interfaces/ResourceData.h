#ifndef RESOURCE_DATA_H
#define RESOURCE_DATA_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/ConditionBase.h"
#include "Interfaces/resouce.h"

namespace rank {

class ResourceData : public rank::Resource {
 public:
  virtual int Load(const std::string& config_file);
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  boost::unordered_map<int, float> data1;
  boost::unordered_map<string, string> data2;
  boost::unordered_map<string, float> data3;
};

extern "C" fisher::Resource* create_ResourceData();
extern "C" void destroy_ResourceData(fisher::Resource* ins);

}  // namespace rank
