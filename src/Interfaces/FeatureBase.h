#ifndef FEATURE_BASE_H_
#define FEATURE_BASE_H_

#include <string>
#include <boost/unordered_map.hpp>
#include "Utilities/AdUtil.h"
#include "Interfaces/EventHandler.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/ConditionBase.h"

namespace rank {

static int64_t __xhash_str__(string str) {
  int64_t code = crc64(str.c_str());
  code = code & 0x7FFFFFFFFFFFFFFF;
  return code;
}
static int64_t __xhash__(const char* str) {
  int64_t code = crc64(str);
  code = code & 0x7FFFFFFFFFFFFFFF;
  return code;
}

#define FEATURE(x) mpList[x] = __xhash__(x);
#define __F(x) FEATURE(x)

#define __ADD_FEATURE__(ad_index, feature_name, feature_value, group_name,     \
                         _type)                                                \
  {                                                                            \
    boost::unordered_map<int64_t, Features::Item>& __map =                     \
        ptd.features[ad_index].features_group[group_name];                     \
    Features::Item& __item = __map[mpList[feature_name]];                      \
    __item.type = _type;                                                       \
    __item.value = 1.0;                                                        \
    /*string _v = TO_STRING(feature_value);*/                                  \
    if (strlen(feature_value.c_str()) < 1) {                                   \
      __item.svalue[0] = '-';  /*默认值*/                                      \
      __item.svalue[1] = '\0';                                                 \
    } else {                                                                   \
      strncpy(__item.svalue, feature_value.c_str(), 256);                      \
      __item.svalue[255] = '\0';                                               \
    }                                                                          \
    /*__map.insert(make_pair(mpList[feature_name], __item));*/                 \
  }

#define ADD_FEATURE(ad_index, feature_name, feature_value)                      \
  {                                                                          \
    const feature_arg_t& _arg = this->getArgument();                         \
    __ADD_FEATURE__(ad_index, feature_name, feature_value, _arg.group, 's')     \
  }

#define ADD_FEATURE_GROUP(ad_index, feature_name, feature_value, group_name) \
  __ADD_FEATURE__(ad_index, feature_name, feature_value, group_name, 's')



#define __ADD_COMMON_FEATURE__(feature_name, feature_value, group_name,        \
                         _type)                                                \
  {                                                                            \
    boost::unordered_map<int64_t, Features::Item>& __map =                     \
        ptd.common_features.features_group[group_name];                        \
    Features::Item& __item = __map[mpList[feature_name]];                      \
    __item.type = _type;                                                       \
    __item.value = 1.0;                                                        \
    /*string _v = TO_STRING(feature_value); */                                 \
    if (strlen(feature_value.c_str()) < 1) {                                   \
      __item.svalue[0] = '-';                                                  \
      __item.svalue[1] = '\0';                                                 \
    } else {                                                                   \
      strncpy(__item.svalue, feature_value.c_str(), 256);                      \
      __item.svalue[255] = '\0';                                               \
    }                                                                          \
    /*__map.insert(make_pair(mpList[feature_name], __item));*/                 \
  }

#define ADD_COMMON_FEATURE(feature_name, feature_value)                      \
  {                                                                          \
    const feature_arg_t& _arg = this->getArgument();                         \
    __ADD_COMMON_FEATURE__(feature_name, feature_value, _arg.group, 's')     \
  }

#define ADD_COMMON_FEATURE_GROUP(feature_name, feature_value, group_name) \
  __ADD_COMMON_FEATURE__(feature_name, feature_value, group_name, 's')

/*
 * 特征参数结构体，从protobuffer解析而来
 */
typedef struct {
  std::string name;                         // 特征名称
  std::map<std::string, std::string> data;  // 数据路径
  std::string type;                         // 特征类型，FILE/REDIS
  std::string condition;
  std::string group;
  std::string config;
  std::string comment;
} feature_arg_t;

class FeatureBase : public AutoUpdate {
 public:
  virtual bool Initialize(
      feature_arg_t& arg,
      const std::map<std::string, ConditionBase*>& conditions,
      ResourceManager* resource_manager) {
    mpConditionsDict = &conditions;
    if (arg.condition != "" && !arg.condition.empty()) {
      vector<string> tmp;
      AdUtil::Split3(tmp, arg.condition, ',');
      for (int i = 0; i < tmp.size(); i++) {
        if (mpConditionsDict->find(tmp[i]) == mpConditionsDict->end()) {
          ERROR(LOGNAME, "condition %s not exist in condition manager",
                tmp[i].c_str());
          return false;
        }
        mpConditions.push_back(tmp[i]);
      }
      NOTICE(LOGNAME, "feature:%s -> %s [%d]", arg.name.c_str(), arg.condition.c_str(), mpConditions.size());
    } else
      NOTICE(LOGNAME, "feature:%s -> all", arg.name.c_str());
    return true;
  }
  virtual bool FetchFeature(
      RankRequest& fisher_request,
      std::vector<RankAdInfo>& ad_list_for_rank,
      std::vector<RankItemInfo>& item_list_for_rank,
      RankDataContext& ptd) = 0;
  virtual const feature_arg_t& getArgument() const = 0;
  // virtual const std::map<std::string, int64_t>& getList() const = 0;

  bool CheckCondition(const RankAdInfo& adinfo, RankDataContext& ptd,
                      const RankRequest* fisher_request = NULL,
                      const std::string& condition_name = "") {
    if (condition_name != "" && !condition_name.empty()) {
      std::map<std::string, ConditionBase*>::const_iterator iter =
          mpConditionsDict->find(condition_name);
      if (iter == mpConditionsDict->end()) return false;
      return iter->second->Check(adinfo, ptd);
    }

    if (mpConditions.size() == 0) return true;

    for (int k = mpConditions.size() - 1; k >= 0; k--) {
      std::map<std::string, ConditionBase*>::const_iterator iter =
          mpConditionsDict->find(mpConditions[k]);
      if (iter == mpConditionsDict->end()) continue;
      ConditionBase* cb = iter->second;
      if (cb == NULL) continue;
      if (cb->Check(adinfo, ptd, fisher_request)) return true;
    }
    // 一个也没有匹配上条件，返回false
    return false;
  }

 protected:
  boost::unordered_map<std::string, int64_t> mpList;
  std::vector<std::string> mpConditions;
  const std::map<std::string, ConditionBase*>* mpConditionsDict;
};

// 特征映射文件资源类
class FeatureMapData : public Resource {
 public:
  bool mHasFeatureMappingFlag;
  std::string mFeatureMapFile;
  virtual int Load(const std::string& config_file);
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  boost::unordered_map<string, std::vector<string> > mFeatureMappingKey;
  boost::unordered_map<string, boost::unordered_map<string, int64_t> >
      mFeatureMappingValue;
  boost::unordered_map<string, std::vector<int64_t> > mFeatureMappingKeyInt;
  boost::unordered_map<string, boost::unordered_map<int64_t, int64_t> >
      mFeatureMappingValueInt;

  // 特征映射，原来属于模型功能，现在拿到特征层面来做
  // type=0 表示特征为枚举类型，type=1 表示特征为数值类型
  int64_t featureMapping(const string& feature_name,
                         const string& feature_value);
  int64_t featureMapping(const string& feature_name,
                         const int64_t& feature_value, int type = 0);

 private:
  bool loadFeatureMapping(const std::string& config_file);
  int64_t findApproximateValue(int64_t value,
                               const vector<int64_t>& indexed_value_vec);
};
rank::Resource* create_FeatureMapData();
void destroy_FeatureMapData(rank::Resource* ins);

}  // namespace rank

#endif
