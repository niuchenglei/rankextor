#ifndef XFEA_BISHENG_FEATURE_OP_S_DIRECTP_H_
#define XFEA_BISHENG_FEATURE_OP_S_DIRECTP_H_

#include "feature_op/single_slot_feature_op.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 根据单个字段的内容直接生产特征
class S_direct_p : public SingleSlotFeatureOp {
public:
    S_direct_p(): _output_ratio(0.0) {
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

private:
    double _output_ratio;
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
