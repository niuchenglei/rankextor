#include "Interfaces/Service.h"
#include "Utilities/ConfigurationSettingsImp.h"
#include "Utilities/ObjectFactoryImp.h"
#include "Interfaces/RedisManager.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ResourceManagerHandler.h"

namespace rank {

template <>
ObjectFactory* Service<ObjectFactory>::get() const {
  return ObjectFactoryImp::instance();
}

template <>
ConfigurationSettings* Service<ConfigurationSettings>::get() const {
  return ConfigurationSettingsImp::instance();
}

template <>
EventFile* Service<EventFile>::get() const {
  return EventFile::instance();
}

template <>
ResourceManager* Service<ResourceManager>::get() const {
  return ResourceManagerHandler::instance()->resource_manager;
}

}  // namespace rank
