#ifndef XFEA_BISHENG_FEATURE_OP_S_BIAS_H_
#define XFEA_BISHENG_FEATURE_OP_S_BIAS_H_

#include "feature_op/single_slot_feature_op.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 根据单个字段的内容直接生产特征
class S_bias : public SingleSlotFeatureOp {
public:
    S_bias() {
        // Nothing to do
    }

    // 初始化及参数检查, 需要依赖的字段个数必须为1
    virtual ReturnCode init(const ExtractorConfig& extractor_config);

    // 根据单个字段的内容直接生产特征
    virtual ReturnCode generate_fea(const LogRecordInterface& record, FeaResultSet& fea_result_set, bool copy_value = true);

    // 资源回收
    virtual void finalize() {
        // Nothing to do
    }
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
