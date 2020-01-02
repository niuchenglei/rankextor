#include <stdexcept>
#include <list>
#include <vector>
#include <map>

#include "Utilities/ObjectFactoryImp.h"
#include "Interfaces/Config.h"
#include "Interfaces/Logger.h"

using std::map;
using std::string;

namespace rank {

static map<string, createObjectMethod> sCreateObjectMap;
static map<string, destroyObjectMethod> sDestroyObjectMap;

ObjectFactoryImp* ObjectFactoryImp::spInstance = NULL;
bool ObjectFactoryImp::mDestroyed = false;

ObjectFactoryImp* ObjectFactoryImp::instance() {
  if (spInstance == NULL) {
    spInstance = new ObjectFactoryImp;
  }

  return spInstance;
}

void ObjectFactoryImp::destroy() {
  if (mDestroyed) {
    return;
    throw std::logic_error(
        "Attempting to destroy ObjectFactory after "
        "destroying it.");
  }
  //sDestroyObjectMap.clear();
  //sCreateObjectMap.clear();

  delete spInstance;
  spInstance = NULL;
  mDestroyed = true;
}

bool ObjectFactoryImp::registObject(const std::string& className,
                                    createObjectMethod com,
                                    destroyObjectMethod dom) {
  sCreateObjectMap.insert(
      map<string, createObjectMethod>::value_type(className, com));
  sDestroyObjectMap.insert(
      map<string, destroyObjectMethod>::value_type(className, dom));

  printf("regist name:%s, create:%x, destroy:%x\n", className.c_str(), com, dom);  
  return true;
}

void* ObjectFactoryImp::createObject(const std::string& className) {
  map<string, void* (*)()>::iterator itr;

  // Fabricate the entry point name, using the class name argument
  itr = sCreateObjectMap.find(className);
  if (itr != sCreateObjectMap.end()) return ((*itr).second)();
  return NULL;
}

void ObjectFactoryImp::destroyObject(void* pObject,
                                     const std::string& className) {
  map<string, void (*)(void*)>::iterator itr;

  // Fabricate the entry point name, using the class name argument
  itr = sDestroyObjectMap.find(className);
  if (itr != sDestroyObjectMap.end()) {
    ((*itr).second)(pObject);
    return;
  }
}

std::string ObjectFactoryImp::getObjectList() {
  string v = "";
  map<string, void* (*)()>::iterator iter = sCreateObjectMap.begin();
  for (; iter != sCreateObjectMap.end(); iter++) {
    v += iter->first;
    v += ",";
  }
  return v;
}

}  // namespace rank
