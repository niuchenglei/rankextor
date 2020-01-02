#ifndef XFEA_BISHENG_COMMON_BISHENG_FACTORY_H_
#define XFEA_BISHENG_COMMON_BISHENG_FACTORY_H_

#include <string>
#include <map>

#include "common/bisheng_common.h"
#include "feature_op/feature_op_base.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 创建对象的工厂基类
class OpFactory {
public:
    virtual void* create_op_object() = 0;
};

template <typename T>
class TemplateOpFactory : public OpFactory {
public:
    virtual void* create_op_object() {
        return new(std::nothrow) T();
    }
};

// 工厂的管理类基类
class OpFactoryManager {
public:
    virtual const char* name() const = 0;
    virtual OpFactory* find_op_factory(const std::string& name) = 0;
    virtual void* create_op_object(const std::string& name) = 0;
    virtual bool register_op_factory(const std::string& name, OpFactory* op_factory) = 0;
};

template<int I>
class TemplateOpFactoryManager : public OpFactoryManager {
public:
    static TemplateOpFactoryManager<I>* instance() {
        if (NULL == TemplateOpFactoryManager<I>::_s_instance) {
            TemplateOpFactoryManager<I>::_s_instance = new(std::nothrow) TemplateOpFactoryManager<I>();
        }
        return TemplateOpFactoryManager<I>::_s_instance;
    }

public:
    static std::string _s_name;
    static TemplateOpFactoryManager<I>* _s_instance;

public:
    TemplateOpFactoryManager() {
        // Nothing to do
    }

    virtual const char* name() const {
       return _s_name.c_str();
    }

    virtual OpFactory* find_op_factory(const std::string& name) {
        std::map<std::string, OpFactory*>::iterator iter = _op_factory_map.find(name);
        if (iter != _op_factory_map.end()) {
             return iter->second;
        } else {
             return NULL;
        }
    }

    virtual void* create_op_object(const std::string& name) {
        OpFactory* op_factory = find_op_factory(name);
        if (NULL != op_factory) {
            return op_factory->create_op_object();
        } else {
            XFEA_BISHENG_FATAL_LOG("can not find op_factory[%s] in OpFactoryManager[%s] with [%lu] factory!",
                    name.c_str(), _s_name.c_str(), _op_factory_map.size());
            return NULL;
        }
    }

    virtual bool register_op_factory(const std::string& name, OpFactory* op_factory) {
        if (NULL == op_factory) {
            XFEA_BISHENG_FATAL_LOG("register_op_factory input param op_factory is NULL in OpFactoryManager[%s]!", _s_name.c_str());
            return false;
        }
        if (find_op_factory(name) != NULL) {
            XFEA_BISHENG_FATAL_LOG("OpFactory[%s] has already in OpFactoryManager[%s] with [%lu] facotry!",
                    name.c_str(), _s_name.c_str(), _op_factory_map.size());
            return false;
        }

        _op_factory_map.insert(std::pair<std::string, OpFactory*>(name, op_factory));
        XFEA_BISHENG_NOTICE_LOG("register_op_factory[%s] to OpFactoryManager[%s] successfully.", name.c_str(), _s_name.c_str());
        return true;
    }

private:
    std::map<std::string, OpFactory*> _op_factory_map;

    TemplateOpFactoryManager(const TemplateOpFactoryManager&);
    void operator=(const TemplateOpFactoryManager&);
};

class OpFactoryResiterT {
public:
    OpFactoryResiterT(const std::string& op_factory_name, OpFactory* op_factory, OpFactoryManager* op_factory_manager) {
        if (NULL == op_factory_manager) {
            XFEA_BISHENG_FATAL_LOG("OpFactoryResiterT input param op_factory_manager is NULL!");
        } else {
            bool ret = op_factory_manager->register_op_factory(op_factory_name, op_factory);
            if (ret) {
                XFEA_BISHENG_NOTICE_LOG("OpFactoryResiterT register_op_factory[%s] to OpFactoryManager successfully.", op_factory_name.c_str());
            } else {
                XFEA_BISHENG_FATAL_LOG("OpFactoryResiterT register_op_factory[%s] to OpFactoryManager failed!", op_factory_name.c_str());
            }
        }
    }
};

class ComponentManager {
public:
    static ComponentManager* instance() {
        if (NULL == _s_instance) {
            _s_instance = new(std::nothrow) ComponentManager();
        }
        return _s_instance;
    }

    static FeatureOpBase* create_feature_op_object(const std::string& name) {
        ComponentManager* instance = ComponentManager::instance();
        if (NULL == instance) {
           XFEA_BISHENG_FATAL_LOG("ComponentManager::instance() returns NULL!");
           return NULL;
        }

        const std::string feature_manager_tag = "FEATURE";
        void* void_op_object = instance->create_op_object(feature_manager_tag, name);
        if (NULL == void_op_object) {
           XFEA_BISHENG_FATAL_LOG("create_op_object[%s] failed from OpFactoryManager[%s]!", name.c_str(), feature_manager_tag.c_str());
           return NULL;
        }
 
        // 讨论：配置错误时可能会有潜在问题
        return static_cast<FeatureOpBase*>(void_op_object);
    }

public:
    static ComponentManager* _s_instance;

public:
    ComponentManager() {
        // Nothing to do
    }

    OpFactoryManager* find_op_factory_manager(const std::string& name) {
        std::map<std::string, OpFactoryManager*>::iterator iter = _op_factory_manager_map.find(name);
        if (iter != _op_factory_manager_map.end()) {
            return iter->second;
        } else {
            return NULL;
        }
    }

    bool register_factory_manager(const std::string& name, OpFactoryManager* op_factory_manager) {
        if (NULL == op_factory_manager) {
            XFEA_BISHENG_FATAL_LOG("register_factory_manager input param op_factory_manager is NULL!");
            return false;
        }
        if (find_op_factory_manager(name) != NULL) {
            XFEA_BISHENG_FATAL_LOG("OpFactoryManager[%s] has already in the OpFactoryManager!", name.c_str());
            return false;
        }
       
        _op_factory_manager_map.insert(std::pair<std::string, OpFactoryManager*>(name, op_factory_manager));
        XFEA_BISHENG_NOTICE_LOG("register_factory_manager [%s] successfully.", name.c_str());
        return true;
    }

    void* create_op_object(const std::string& op_factory_manager_name, const std::string& op_factory_name) {
        OpFactoryManager* op_factory_manager = find_op_factory_manager(op_factory_manager_name);
        if (NULL == op_factory_manager) {
            XFEA_BISHENG_FATAL_LOG("can not find op_factory_manager[%s]!", op_factory_manager_name.c_str());
            return NULL;
        } else {
            return op_factory_manager->create_op_object(op_factory_name);
        }
    }

private:
    std::map<std::string, OpFactoryManager*> _op_factory_manager_map;

    ComponentManager(const ComponentManager&);
    void operator=(const ComponentManager&);
};

class OpFactoryManagerResiterT {
public:
    OpFactoryManagerResiterT(const std::string& op_factory_manager_name, OpFactoryManager* op_factory_manager) {
        // TODO 多线程？
        if (NULL == op_factory_manager) {
            XFEA_BISHENG_FATAL_LOG("OpFactoryManagerResiterT input param op_factory_manager is NULL!");
        } else {
            ComponentManager* component_manager = ComponentManager::instance();
            if (NULL == component_manager) {
                XFEA_BISHENG_FATAL_LOG("ComponentManager::instance() returns NULL!");
            } else {
                bool ret = component_manager->register_factory_manager(op_factory_manager_name, op_factory_manager);            
                if (ret) {
                    XFEA_BISHENG_NOTICE_LOG("OpFactoryManagerResiterT register_factory_manager[%s] successfully.", op_factory_manager_name.c_str());
                } else {
                    XFEA_BISHENG_FATAL_LOG("OpFactoryManagerResiterT register_factory_manager[%s] failed!", op_factory_manager_name.c_str());
                }
            }
        }
    }
};

#define XFEA_BISHNEG_DECLARE_FACTORY_MANAGER(name, id) \
    typedef TemplateOpFactoryManager<id> name##_factory_manager_t;

#define XFEA_BISHNEG_USING_FACTORY_MANAGER(name) \
    template<> name##_factory_manager_t* name##_factory_manager_t::_s_instance = NULL; \
    template<> std::string name##_factory_manager_t::_s_name = #name; \
    static OpFactoryManagerResiterT op_factory_manager_register(#name, name##_factory_manager_t::instance());


// 声明Feature OpFactoryManager
XFEA_BISHNEG_DECLARE_FACTORY_MANAGER(FEATURE, 6)

#define XFEA_BISHNEG_USING_FEATURE(feature) \
    typedef TemplateOpFactory<feature> feature##_feature_factory_t; \
    static feature##_feature_factory_t feature##_feature_factory; \
    static OpFactoryResiterT feature##_factory_register(#feature, &feature##_feature_factory, FEATURE_factory_manager_t::instance());


XFEA_BISHENG_NAMESPACE_GUARD_END

#endif

