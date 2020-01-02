#ifndef XFEA_BISHENG_CORE_LOG_RECORD_ARRAY_IMPL_H_
#define XFEA_BISHENG_CORE_LOG_RECORD_ARRAY_IMPL_H_

#include "core/log_record_interface.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 封装特征提取引擎字段输入的依托数组实现的子类
class LogRecordArrayImpl : public LogRecordInterface {
public:
    LogRecordArrayImpl(): _tag_name(""), _already_filled_field_num(0),
            _expected_field_num(0), _extractor_config(NULL) {
        // 构造函数不要调用虚函数，所有不掉reset
        for (uint32_t i = 0; i < GlobalParameter::kMaxRecordFieldNum; ++i) {
            _value_buf[i][0] = '\0';
            _value_buf_ptr[i] = NULL;
            _is_set[i] = false;
        }
    }

    // 初始化：从配置初始化成员变量
    virtual ReturnCode init(ExtractorConfig* extractor_config);

    // 填充字段值
    virtual ReturnCode fill_value(const char* value, const int field_index, bool copy_value = true);
    virtual ReturnCode fill_value(const char* value, const std::string& field_name, bool copy_value = true);

    // 清空存储的内容，供后续再次使用该类
    virtual void reset();

    // 检查是否可供特征提取使用（是否所有的字段都已填充值)
    virtual bool is_valid() const;
    virtual bool is_update(const int field_index) const {
        return _is_update[field_index];
    }
    virtual void set_update(bool flag);

    // 获取字段值
    virtual const char* get_value(const int field_index) const;
    virtual const char* get_value(const std::string& field_name) const;

    // 返回已填充的字段个数
    virtual uint32_t size() const {
        return _already_filled_field_num;
    }

    // 回收资源
    virtual void finalize();

private:
    std::string _tag_name;   // 区分不同的特征提取场景
    const char* _value_buf_ptr[GlobalParameter::kMaxRecordFieldNum];
    char _value_buf[GlobalParameter::kMaxRecordFieldNum][GlobalParameter::kMaxRecordFieldValueSize];  // 存储存储的多个字段
    bool _is_set[GlobalParameter::kMaxRecordFieldNum];                                                // 标识字段是否被填充
    bool _is_update[GlobalParameter::kMaxRecordFieldNum];                                             // 标识字段是否被更新
    uint32_t _already_filled_field_num;                                                               // 已经填充的字段总数
    uint32_t _expected_field_num;                                                                     // 期望填充的字段总数
    ExtractorConfig* _extractor_config;                                                               // 存储特征提取引擎的所有配置的对象指针（外部传递赋值）

    LogRecordArrayImpl(const LogRecordArrayImpl&);
    void operator=(const LogRecordArrayImpl&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
