// Read an INI file into easy-to-access name/value pairs.

// Better the class below to integrate into the lib better
//
// http://code.google.com/p/inih/

#ifndef XFEA_BISHENG_UTIL_INIREADER_H_
#define XFEA_BISHENG_UTIL_INIREADER_H_

#include <map>
#include <string>

#include "common/bisheng_common.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// Modified by the google ini lib
class INIReader {
public:
    INIReader() {
        // Nothing to do
    }

    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, or -1 on file open error.
    int Init(const std::string& filename);

    // 找到配置项返回0并设置return_value为配置项的值，未找到配置项返回1
    int Get(const std::string& section, const std::string& name,
                    std::string& return_value) const;
    // 找到配置项返回0并设置return_value为配置项的值，未找到配置项返回1, 会将return_value设置为default_value
    int Get(const std::string& section, const std::string& name,
            std::string& return_value, const std::string& default_value) const;

    // 找到配置项且转换正确，返回0并设置return_value为转换后的配置项的值
    // 找到配置项但转换不正，返回-1，return_value不做任何处理
    // 未找到配置项，返回1
    int GetUInt64(const std::string& section, const std::string& name, uint64_t& return_value) const;
    // 找到配置项且转换正确，返回0并设置return_value为转换后的配置项的值
    // 找到配置项但转换不正，返回-1，return_value不做任何处理
    // 未找到配置项，返回0, 设置return_value为default_value
    int GetUInt64(const std::string& section, const std::string& name, uint64_t& return_value, const uint64_t& default_value) const;

    // 同GetUInt64
    int GetReal(const std::string& section, const std::string& name, double& return_value) const;
    int GetReal(const std::string& section, const std::string& name, double& return_value, const double& default_value) const;

    // 同GetUInt64
    int GetBoolean(const std::string& section, const std::string& name, bool& return_value) const;
    int GetBoolean(const std::string& section, const std::string& name, bool& return_value, const bool& default_value) const;

private:
    static std::string MakeKey(const std::string& section, const std::string& name);
    static int ValueHandler(void* user, const char* section, const char* name,
                            const char* value);

private:
    std::map<std::string, std::string> _values;         // 存储原始配置项内容

    INIReader(const INIReader&);
    void operator=(const INIReader&);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
