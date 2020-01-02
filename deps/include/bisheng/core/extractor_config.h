#ifndef XFEA_BISHENG_CORE_EXTRACTOR_CONFIG_H_
#define XFEA_BISHENG_CORE_EXTRACTOR_CONFIG_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/algorithm/string/trim.hpp>

#include "common/bisheng_common.h"
#include "util/bisheng_INIReader.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 单个特征类的配置
struct fea_op_config_t {
    fea_op_config_t(): name(""), fea_slot(0), fea_op_name(""), 
            arg_str(""), is_need_hash(false), hash_range_min(0), hash_range_max(0) {
        // Nothing to do
    }

    // 讨论：使用拷贝构造函数和复制预算符目前没有问题；extractor_config.cpp中vecotr push_back使用；

    std::string to_string() const;

    std::string name;                                // 特征类的标识名称
    int32_t fea_slot;                                // 特征类的标识ID
    std::string fea_op_name;                         // 特征类对应的提取类名称
    std::vector<std::string> depend_col_name_vec;    // 特征类依赖的字段名称
    std::vector<int> depend_col_index_vec;           // 特征类依赖的字段index
    std::string arg_str;                             // 特征类依赖的参数
    bool is_need_hash;                               // 是否对特征明文签名
    uint64_t hash_range_min;                         // 特征签名变换的最小值（包含）
    uint64_t hash_range_max;                         // 特征签名变换的最大值（不包含）
};

// 存储特征提取引擎所有配置的类
class ExtractorConfig {
public:
    ExtractorConfig(): _config_reader(), _tag_name(""), _input_schema_size(0), _feature_global_is_hash(false),
            _feature_global_hash_range_min(0), _feature_global_hash_range_max(0) {
        // Nothing to do
    }

    ReturnCode init(const std::string& config_file_path, const std::string& feature_list_conf_file_path);

    std::string get_tag_name() const {
        return _tag_name;
    }

    int key_to_index(const std::string& field_schema_name) const {
        //std::string _a(field_schema_name);// = boost::trim(field_schema_name);
        //boost::trim(_a);
        std::map<std::string, int>::const_iterator iter = _input_schema_key_index_map.find(field_schema_name); //field_schema_name);
        if (iter != _input_schema_key_index_map.end()) {
            return iter->second;
        } else {
            return -1;
        }
    }

    uint32_t get_input_field_num() const {
        return _input_schema_size;
    }

    const std::vector<fea_op_config_t>& get_feature_op_config_vec() const {
        return _feature_op_config_vec;
    }

    const INIReader& get_config_reader() const {
        return _config_reader;
    }

    int get_config_str(const std::string& name, std::string& value) const {
        return _config_reader.Get("", name, value);
    }

    const std::map<std::string, int>& get_input_schema_key_index_map() const {
        return _input_schema_key_index_map;
    }

    const std::vector<std::string>& get_output_add_field_name_vec() const {
        return _output_add_field_name_vec;
    }

    std::string get_label_field() const {
        return _label_field;
    }

    std::string to_string() const;

    void finalize() {
        // Nothing to do
    }

private:
    ReturnCode parse_output_add_field_name(const std::string& str_buf); 

    ReturnCode parse_input_schema(const std::string& input_schema_str_buf);

    ReturnCode parse_feature_conf_line(std::string& line_input, std::set<int32_t>& exist_slot_set, fea_op_config_t& fea_op_config);
 
    ReturnCode load_feature_list(const std::string& feature_list_file_path);

    ReturnCode load_config(const std::string& config_file_path, const std::string& feature_lsit_conf_file_path);

private:
    // 存储原始配置
    INIReader _config_reader;

    // 全局配置
    std::string _tag_name;
    std::string _label_field;

    // 输入schema相关的配置
    uint32_t _input_schema_size;
    // 讨论：使用hashmap更好，但考虑只是初始化阶段使用，可以使用map
    std::map<std::string, int> _input_schema_key_index_map;

    // 输出相关的配置
    std::vector<std::string> _output_add_field_name_vec;

    // 预处理相关的配置
    std::vector<std::string> _preprocess_op_name_vec;

    // 特征相关的配置
    bool _feature_global_is_hash; 
    uint64_t _feature_global_hash_range_min;
    uint64_t _feature_global_hash_range_max;
    std::vector<fea_op_config_t> _feature_op_config_vec;    

    ExtractorConfig(const ExtractorConfig&);
    void operator=(const ExtractorConfig&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
