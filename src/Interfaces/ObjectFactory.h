/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <string>

namespace rank {

typedef void* (*createObjectMethod)();
typedef void (*destroyObjectMethod)(void*);

#define OBJECT_CLASS(className)                                               \
  static const std::string _Name;                                             \
  static const std::string _RegistName;                                       \
  static const bool _RegistFlag;                                              \
  const std::string& getTypeName() const { return className::_Name; }         \
  const std::string& getRegistName() const { return className::_RegistName; } \
  const bool getRegistFlag() const { return className::_RegistFlag; }

#define REGISTE_CLASS(registName, className)                                  \
  const std::string className::_Name = #className;                            \
  const std::string className::_RegistName = registName;                      \
  static void* createObject() { return new className(); }                     \
  static void destroyObject(void* ptr) {                                      \
    className* pp = static_cast<className*>(ptr);                             \
    delete pp;                                                                \
  }                                                                           \
  const bool className::_RegistFlag = Service<ObjectFactory>()->registObject( \
      registName, createObject, destroyObject);

/**
 *  \ingroup ServiceModule
 *  Dynamic creation of arbitrary class objects
 *
 *  The ObjectFactory can create and destroy objects of arbitrary classes
 *  dynamically at runtime.
 */
class ObjectFactory {
 public:
  virtual bool registObject(const std::string& className,
                            createObjectMethod com,
                            destroyObjectMethod dom) = 0;

  /**
  *  Instantiate an object whose type is determined at runtime.
  *
  *  @param   className
  *           A string containing the class name of the object
  *           to be instantiated.
  *           The valid object types are:
  *           - AFDVideoYUV420P
  *           - AFDAudioU8
  *           .
  *
  *  @return  A pointer to the instantiated object if successful, or NULL.
  */
  virtual void* createObject(const std::string& className) = 0;

  /**
  *  Deallocate an object that was previously allocated by the ObjectFactory,
  *  including vectors.
  *
  *  @param   pObject
  *           A pointer to an object previously allocated by the ObjectFactory.
  *  @param   className
  *           A string containing the class name of the object to be
  *           deallocated.
  */
  virtual void destroyObject(void* pObject, const std::string& className) = 0;

  virtual std::string getObjectList() = 0;

 protected:
  /**
  * This will be cleaned up during application close.  Plug-ins do not
  * need to destroy it.
  */
  virtual ~ObjectFactory() {}
};

}  // namespace rank

#endif
