#ifndef XFEA_BISHENG_COMMON_BISHENG_STRING_TOOL_H_
#define XFEA_BISHENG_COMMON_BISHENG_STRING_TOOL_H_

#include <vector>
#include <string>

#include "common/bisheng_common.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

class StringTool {
public:
    static std::string& trim(std::string &s);

    static int get_var_value(const std::string& src_str, const std::string& var_name, std::string& value);

    static void Split2(std::vector<std::string>& vs, const std::string& line, char dmt);

    static int tokenize(const std::string& src, const std::string& tok, std::vector<std::string>& tokens);

    static int StringToUint64(const std::string& input_str, uint64_t& return_value);

    static int StringToInt32(const char* input_str, int32_t& return_value);

    static int StringToInt32(const std::string& input_str, int& return_value);

    static int StringToDouble(const std::string& input_str, double& return_value);

    static int StringToBool(const std::string& input_str, bool& return_value);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
