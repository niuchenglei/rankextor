#ifndef XFEA_BISHENG_FEATURE_OP_SINGLE_SLOT_FEATURE_OP_H_
#define XFEA_BISHENG_FEATURE_OP_SINGLE_SLOT_FEATURE_OP_H_

#include "feature_op/feature_op_base.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 所有生成单个slot下特征实现类的基类（可以产生单个slot下的多个特征）
class SingleSlotFeatureOp : public FeatureOpBase {
public:
    SingleSlotFeatureOp() {
        // Nothing to do
    }

    // 初始化及参数检查
    virtual ReturnCode init(const ExtractorConfig& extractor_config) = 0;

    // 具体的产生特征的实现
    virtual inline ReturnCode generate_fea(const LogRecordInterface& record, FeaResultSet& fea_result_set, bool copy_value = true) = 0;

    // 资源回收
    virtual void finalize() {
        // Nothing to do
    }

protected:
    // 将提取的特征明文进行签名变换等操作，并将相关结果存入fea_result_set
    virtual ReturnCode emit_feature(const std::string& name, const int idx, const char* fea_text, FeaResultSet& fea_result_set, bool copy_value = true);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
