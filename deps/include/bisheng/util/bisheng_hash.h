#ifndef XFEA_BISHENG_COMMON_BISHENG_HASH_H_
#define XFEA_BISHENG_COMMON_BISHENG_HASH_H_

#include <stdint.h>
#include <string.h>
#include "common/bisheng_common.h"

XFEA_BISHENG_NAMESPACE_GUARD_BEGIN

// 签名相关函数
class HashTool {
public:
    // 如果text为NULL，sig1和sig2置为UINT32_MAX;
    // 如果text为""，sig1和sig2置为0;
    static void create_sign64_beg(uint32_t& sig1, uint32_t& sig2, const char* text);

    static void create_sign64(uint32_t& sig1, uint32_t& sig2, const uint32_t type, const uint32_t data1, const uint32_t data2);

    static uint64_t hash64(const char* text, const int32_t slot);

    static uint64_t crc64(const char* buffer, const int32_t slot);

    static uint64_t hash64_with_int32_value(const int32_t value, const int32_t slot);
};

XFEA_BISHENG_NAMESPACE_GUARD_END

#endif
