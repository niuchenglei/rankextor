#ifndef XFEA_BISHENG_CORE_FEA_RESULT_SET_H_
#define XFEA_BISHENG_CORE_FEA_RESULT_SET_H_

#include "common/bisheng_common.h"
#include <boost/unordered_map.hpp>
#include <vector>

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 单个特征结果的表示
class fea_result_t {
public:
    fea_result_t(): fea_slot(0), is_hash(false), origin_fea_sign(0), final_fea_sign(0) {
        fea_text[0] = '\0';
    }

    void inline reset() {
        fea_text[0] = '\0';
        fea_slot = 0;
        is_hash = false;
        origin_fea_sign = 0;
        final_fea_sign = 0; 
        is_valid = false;
    }

    char fea_text[GlobalParameter::kMaxFeaTextResultSize];         // 特征明文内容
    int32_t fea_slot;                                              // 所属特征类ID
    bool is_hash, is_valid;                                        // 是否需要hash，是否有效
    uint64_t origin_fea_sign;                                      // 原始特征签名
    uint64_t final_fea_sign;                                       // 变换后的特征签名

private:
    fea_result_t(const fea_result_t&);
    void operator=(const fea_result_t&);
};

// 单个样本的特征提取结果: 对fea_result_t数组的封装
class FeaResultSet {
public:
    FeaResultSet(): _fea_result_num(0) {
        // Nothing to do
    }

    // 初始化：清空样本
    ReturnCode init() {
        reset();
        return RC_SUCCESS;
    }

    // 清空样本
    void reset() {
        /*_fea_result_num = (_fea_result_num==0)?GlobalParameter::kMaxFeaResultNum:_fea_result_num;
        for (uint32_t i = 0; i < _fea_result_num; i++) {
            _fea_result_array[i].reset();
        }*/
        _fea_result_num = 0;
        _fea_name_idx.clear();
        _fea_name_num.clear();
    }

    // 获取样本中的首个特征及样本中特征的个数
    const fea_result_t* get_fea_result_array(uint32_t& fea_result_num) const {
        fea_result_num = _fea_result_num;
        return _fea_result_array;
    }

    // 获取样本中指定index的特征
    const fea_result_t* get_fea_result(const uint32_t index) const {
        if (index >= _fea_result_num) {
            XFEA_BISHENG_WARN_LOG("index [%u] of fea_result_array exeeds bound [%u]", index, _fea_result_num);
            return NULL;
        } else {
            return _fea_result_array + index;
        }
    }

    // 返回样本中特征的个数
    uint32_t size() const {
        return _fea_result_num;
    }

    // 填充一个特征明文为char*的特征提取结果
    ReturnCode fill_fea_result(const std::string& name, const int idx, const char* value, const int32_t fea_slot,
                               const bool is_hash = false, const uint64_t origin_fea_sign = 0, const uint64_t final_fea_sign = 0, bool copy_value = true);

    // 资源回收
    void finalize() {
        // Nothing to do
    }

private:
    fea_result_t _fea_result_array[GlobalParameter::kMaxFeaResultNum];  // 样本（特征的数组）
    uint32_t _fea_result_num;                                           // 样本中特征的个数
    boost::unordered_map<std::string, std::vector<int> > _fea_name_idx;
    boost::unordered_map<std::string, int > _fea_name_num;

    FeaResultSet(const FeaResultSet&);
    void operator=(const FeaResultSet&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
