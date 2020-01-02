#ifndef XFEA_BISHENG_CORE_LOG_RECORD_INTERFACE_H_
#define XFEA_BISHENG_CORE_LOG_RECORD_INTERFACE_H_

#include "common/bisheng_common.h"
#include "core/extractor_config.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 封装特征提取引擎字段输入的基类
class LogRecordInterface {
public:
    // 初始化
    virtual ReturnCode init(ExtractorConfig* extractor_config) = 0;

    // 填充字段值
    virtual ReturnCode fill_value(const char* value, const int field_index, bool copy_value = true) = 0;
    virtual ReturnCode fill_value(const char* value, const std::string& field_name, bool copy_value = true) = 0;
    
    // 更新字段值, 一般是预处理类使用
    //virtual ReturnCode update_value(const char* value, const int field_index) = 0;
    //virtual ReturnCode update_value(const char* value, const std::string& field_name) = 0;

    // 清空存储的内容，供后续再次使用该类
    virtual void reset() = 0;
    virtual void set_update(bool flag) = 0;

    // 检查是否可供特征提取使用（是否所有的字段都已填充值)
    virtual bool is_valid() const = 0;
    virtual bool is_update(const int field_index) const = 0;

    // 获取字段值
    virtual const char* get_value(const int field_index) const = 0;
    virtual const char* get_value(const std::string& field_name) const = 0;

    // 返回已填充的字段个数
    virtual uint32_t size() const = 0;

    // 回收资源
    virtual void finalize() {
        // Nothing to do
    }

    virtual ~LogRecordInterface() {
        // Nothing to do
    }
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
