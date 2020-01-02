#ifndef CONFIGURATIONSETTINGSIMP_H
#define CONFIGURATIONSETTINGSIMP_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces/ConfigurationSettings.h"
#include "Utilities/INIReader.h"

namespace rank {

class ConfigurationSettingsImp : public ConfigurationSettings {
 public:
  static ConfigurationSettingsImp* instance();
  static void destroy();

  std::string getSetting(const std::string& key) const;
  bool deserialize(const std::string& pFilename);
  bool isInitialized() const;

 protected:
  static ConfigurationSettingsImp* spInstance;
  static bool mDestroyed;

  ConfigurationSettingsImp();
  virtual ~ConfigurationSettingsImp();

  bool mIsInitialized;
  INIReader* mpReader;
  std::map<std::string, std::string> mHashTable;
};
}  // namespace rank
#endif
