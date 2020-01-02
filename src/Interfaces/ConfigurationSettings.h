/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef CONFIGURATIONSETTINGS_H
#define CONFIGURATIONSETTINGS_H

#include <map>
#include <string>

namespace rank {

class ConfigurationSettings {
 public:
  /**
  * Gets the current value for the given setting.  The value
  * returned will be located in the following order: temporary setting,
  * per-user setting and default setting.
  *
  * @see setTemporarySetting(), setSetting()
  *
  * @param   key
  *          Name of the setting to be found.  This key
  *          can have '/' in it, in which case it will behave like
  *          DynamicObject::getAttributeByPath.
  *
  * @return  The setting in a DataVariant. If the setting was not found,
  *          an invalid DataVariant will be returned.
  */
  virtual std::string getSetting(const std::string& key) const = 0;

  /**
  * Loads the given settings from the file and return them as a
  * DynamicObject. The load settings are simply returned as a
  * dynamic object, existing setting values are left unchanged.
  *
  * @param pFilename
  *        the full path to the file containing the settings.
  *
  * @return A dynamic object containing the loaded settings or \c NULL
  *         if the settings could not be loaded.
  */
  virtual bool deserialize(const std::string& pFilename) = 0;

  virtual bool isInitialized() const = 0;

 protected:
  /**
  * This will be cleaned up during application close.
  */
  virtual ~ConfigurationSettings() {}
};

}  // namespace rank

#endif
