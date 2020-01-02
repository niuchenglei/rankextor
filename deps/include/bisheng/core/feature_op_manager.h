#ifndef XFEA_BISHENG_CORE_FEATURE_OP_MANAGER_H_
#define XFEA_BISHENG_CORE_FEATURE_OP_MANAGER_H_

#include <vector>

#include "common/bisheng_common.h"
#include "core/extractor_config.h"
#include "feature_op/feature_op_base.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 特征类对象管理类
class FeatureOpManager {
public:
    FeatureOpManager(): _extractor_config(NULL) {
        // Nothing to do
    }

    // 初始化：创建特征配置列表配置的特征类对象，并初始化这些对象
    virtual ReturnCode init(ExtractorConfig* extractor_config);

    // 调用所有配置的特征类提取特征，并将结果存入FeaResultSet
    // 注意：进行特征签名时，暂时未处理特征签名冲突的情况
    virtual ReturnCode extract_features(const LogRecordInterface& record, FeaResultSet& fea_result_set, bool copy_value = true);

    // 回收资源：delete创建的特征类对象
    virtual void finalize();

    virtual ~FeatureOpManager() {
        // Nothing to do
    }

private:
    ExtractorConfig* _extractor_config;           // 存储特征提取引擎的所有配置的对象指针（外部传递赋值）
    std::vector<FeatureOpBase*> _fea_op_vec;      // 存储创建好的特征类对象

    FeatureOpManager(const FeatureOpManager&);
    void operator=(const FeatureOpManager&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
