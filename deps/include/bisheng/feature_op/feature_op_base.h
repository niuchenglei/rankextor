#ifndef XFEA_BISHENG_FEATURE_OP_FEATURE_OP_BASE_H_
#define XFEA_BISHENG_FEATURE_OP_FEATURE_OP_BASE_H_

#include "common/bisheng_common.h"
#include "core/extractor_config.h"
#include "core/log_record_interface.h"
#include "core/fea_result_set.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 所有特征实现类的基类
class FeatureOpBase {
public:
    FeatureOpBase(): _name(""), _slot(0), _fea_op_name(""), _arg_str(""),
            _is_need_hash(false), _hash_range_min(0), _hash_range_max(0) {
        // Nothing to do
    }

    // 初始化及参数检查
    virtual ReturnCode init(const ExtractorConfig& extractor_config) = 0;

    // 具体的产生特征的实现
    // 讨论:第一个参数可以考虑更加直观的方式,而不是使用LogRecordInterface
    virtual ReturnCode generate_fea(const LogRecordInterface& record, FeaResultSet& fea_result_set, bool copy_value = true) = 0;

    // 资源回收
    virtual void finalize() {
        // Nothing to do
    }

    virtual ~FeatureOpBase() {
        // Nothing to do
    }

    // not virtual
    // 实际上成员变量可用fea_op_config_t标识，将二者分离主要考虑将来可能不一致
    void set_info(const fea_op_config_t& fea_op_config) {
        _name = fea_op_config.name;
        _slot = fea_op_config.fea_slot;
        _fea_op_name = fea_op_config.fea_op_name;
        _depend_col_name_vec = fea_op_config.depend_col_name_vec;
        _depend_col_index_vec = fea_op_config.depend_col_index_vec;
        _arg_str = fea_op_config.arg_str;
        _is_need_hash = fea_op_config.is_need_hash;
        _hash_range_min = fea_op_config.hash_range_min;
        _hash_range_max = fea_op_config.hash_range_max;
        if (_hash_range_min > _hash_range_max)
          _hash_range_min = _hash_range_max;
        _hash_range = _hash_range_max - _hash_range_min;
    }

    // not virtual
    const std::string& get_name_str() const {
        return _name;
    }

    const char* get_name_char_ptr() const {
        return _name.c_str();
    }

    int32_t get_slot() const {
        return _slot;
    }

    void set_slot(const int32_t slot) {
        _slot = slot;
    }

protected:
    // 将提取的特征明文进行签名变换等操作，并将相关结果存入fea_result_set
    virtual ReturnCode emit_feature(const std::string& name, const int idx, const char* fea_text, FeaResultSet& fea_result_set, bool copy_value = true) = 0;

protected:
    // 讨论：目前变量定义成protected，方便子类操作，更好的方式是提供get接口
    std::string _name;                                 // 特征类的标识名称
    int32_t _slot;                                     // 特征类的标识ID
    std::string _fea_op_name;                          // 特征类对应的提取类名称
    std::vector<std::string> _depend_col_name_vec;     // 特征类依赖的字段名称
    std::vector<int> _depend_col_index_vec;            // 特征类依赖的字段index
    std::string _arg_str;                             // 特征类依赖的参数
    bool _is_need_hash;                                // 是否对特征明文签名
    uint64_t _hash_range_min;                          // 特征签名变换的最小值（包含）
    uint64_t _hash_range_max;                          // 特征签名变换的最大值（不包含）
    uint64_t _hash_range;
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
