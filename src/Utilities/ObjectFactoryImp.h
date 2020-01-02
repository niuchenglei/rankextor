#ifndef OBJECTFACTORYIMP_H
#define OBJECTFACTORYIMP_H

#include "Interfaces/ObjectFactory.h"

#include <string>

namespace rank {

class ObjectFactoryImp : public ObjectFactory {
 public:
  static ObjectFactoryImp* instance();
  static void destroy();

  virtual bool registObject(const std::string& className,
                            createObjectMethod com, destroyObjectMethod dom);
  virtual void* createObject(const std::string& className);
  virtual void destroyObject(void* pObject, const std::string& className);
  virtual std::string getObjectList();

 protected:
  ObjectFactoryImp() {};
  virtual ~ObjectFactoryImp() {};

 private:
  static ObjectFactoryImp* spInstance;
  static bool mDestroyed;
};

}  // namespace rank

#endif
