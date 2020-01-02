#ifndef COMMON_TYPE_H
#define COMMON_TYPE_H

#include <boost/unordered_map.hpp>
#include <map>
#include <iostream>
//#include <sharelib/util/string_util.h>
#include "Interfaces/const.h"
#include "Interfaces/define.h"
#include "Interfaces/Config.h"

namespace rank {

#define FILTER "filter_reason"

/*
 * origin and transformed feature such like gbdt encode/nn encode.
 */
class Features {
public:
  uint64_t id;
  struct Item {
    char type;  // 0:integer
    float value;
    char svalue[256];
    char* svalue_ptr;
  };
  // <group_name, <feature_name, feature_value> >
  // 所有特征（基础特征、变换后特征）均存放在此处，特征分组名即为模型配置的输出
  boost::unordered_map<std::string, boost::unordered_map<int64_t, Features::Item> > features_group;
};

}  // namespace rank

#endif
