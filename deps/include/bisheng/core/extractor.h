#ifndef XFEA_BISHENG_CORE_EXTRACTOR_H
#define XFEA_BISHENG_CORE_EXTRACTOR_H

#include "core/log_record_array_impl.h"
#include "core/fea_result_set.h"
#include "core/feature_op_manager.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 特征提取引擎对外接口; 非多线程安全;
// 使用方法：
//   1. 创建对象
//   2. 调用init加载配置并初始化
//   3. 单条日志的特征提取过程如下，抽取多条可以循环调用该过程
//      1) 调用one_round_reset清空相关存储资源
//      2) 调用add_field_value相关接口填充相关字段 
//      3) 调用extract_features_from_record进行具体的特征提取
//      4) 调用get_fea_result_set获取特征提取结果
//   4. 调用finalize回收相关资源，整个过程结束

class Extractor {
public:
    // 打印bisheng版本和编译信息
    static void print_version_info();

public:
    Extractor(): _extractor_config(NULL), _record(NULL),
            _fea_result_set(NULL), _feature_op_manager(NULL) {
        // Nothing to do
    }

    // 加载配置并初始化各个成员变量（只调用一次）
    ReturnCode init(const std::string& config_file_path, const std::string& feature_list_conf_file_path = "");

    // 清空存储，在再次抽取特征之前调用
    void one_round_reset() {
        _record->reset(); 
        _fea_result_set->reset();
    }
    // 只清空结果，输入特征不进行处理，在线上使用更快
    void just_result_reset() {
        _fea_result_set->reset();
    }
 
    // 获取封住输入内容的Record
    const LogRecordInterface& get_record() const {
        return *_record;
    }

    // 填充字段值,离线使用
    ReturnCode add_field_value(const std::string& value, const int field_index, bool copy_value = true) {
        return _record->fill_value(value.c_str(), field_index, copy_value);
    }

    // 填充字段值,离线使用
    ReturnCode add_field_value(const char* value, const int field_index, bool copy_value = true) {
        return _record->fill_value(value, field_index, copy_value);
    }

    // 填充字段值,在线使用
    ReturnCode add_field_value(const std::string& value, const std::string& field_name, bool copy_value = true) {
        return _record->fill_value(value.c_str(), field_name, copy_value);
    }

    // 填充字段值,在线使用
    ReturnCode add_field_value(const char* value, const std::string& field_name, bool copy_value = true) {
        return _record->fill_value(value, field_name, copy_value);
    }

    // 进行特征抽取（单条样本）
    // 注意：进行特征签名时，暂时未处理特征签名冲突的情况
    ReturnCode extract_features_from_record(bool copy_value = true); 

    // 获取特征抽取的结果（单条样本）
    const FeaResultSet& get_fea_result_set() const {
        return *_fea_result_set;
    }

    // 返回输入的schema及其index
    const std::map<std::string, int>& get_input_schema_key_index_map() const {
        return _extractor_config->get_input_schema_key_index_map();
    }

    // 返回需要在样本中追加的原始字段schema，离线
    const std::vector<std::string>& get_output_add_field_name_vec() const {
        return _extractor_config->get_output_add_field_name_vec();
    }

    std::string get_label_field() const {
        return _extractor_config->get_label_field();
    }

    // 回收资源（只调用一次）
    void finalize();

private:
    ExtractorConfig* _extractor_config;        // 存储特征提取引擎的所有配置
    LogRecordInterface* _record;               // 封装输入字段
    FeaResultSet* _fea_result_set;             // 封装特征提取结果（单条样本）
    FeatureOpManager* _feature_op_manager;     // 特征管理类，调用管理的特征类进行具体的特征提取

    Extractor(const Extractor&);
    void operator=(const Extractor&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
