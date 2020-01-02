#ifndef XFEA_BISHENG_FEATURE_OP_S_CTR_BIN_H_
#define XFEA_BISHENG_FEATURE_OP_S_CTR_BIN_H_

#include "feature_op/single_slot_feature_op.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 
class S_ctr_bin : public SingleSlotFeatureOp {
public:
    const static std::string kSepeartor;
    const static int32_t kDefaultCtrBin = -1;

public:
    S_ctr_bin(): _config_item_name(""), _ctr_bin_name(""), _ctr_filename("") {
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
    int parse_ctr_mapping_line(const std::string& line, const std::string& ctr_bin_name, std::vector<std::string>& field_vec);

    int load_ctr_mapping(const std::string& ctr_filename, const std::string& ctr_bin_name);

    int32_t find_approximate_value(const std::vector<int32_t>& target_vec, const int32_t value);

    int32_t find_ctr_map(const char* fea_text);

private:
    std::string _config_item_name;
    std::string _ctr_bin_name;
    std::string _ctr_filename;
    std::vector<int32_t> _ctr_value_vec;
    std::map<int32_t, int32_t> _ctr_value_and_bin_map;
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
