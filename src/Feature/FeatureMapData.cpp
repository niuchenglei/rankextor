#include <iostream>
#include "Interfaces/FeatureBase.h"

using namespace boost;

namespace rank {

DEFINE_RESOURCE(FeatureMapData);

int FeatureMapData::Load(const std::string& config_file) {
  // FATAL(LOGNAME, "basic_info load from %s", config_file.c_str());
  mFeatureMapFile = config_file;

  if (!loadFeatureMapping(mFeatureMapFile)) return 1;

  return 0;
}
bool FeatureMapData::loadFeatureMapping(const std::string& config_file) {
  if (mFeatureMapFile == "") {
    mHasFeatureMappingFlag = false;
    return true;
  }
  ifstream ifs(mFeatureMapFile.c_str());
  if (!ifs) {
    ERROR(LOGNAME, "can'f find file: %s", mFeatureMapFile.c_str());
    return false;
  }
  unordered_map<string, vector<string> >().swap(mFeatureMappingKey);
  unordered_map<string, unordered_map<string, int64_t> >().swap(
      mFeatureMappingValue);  //释放内存
  string line;
  int cnt = 0;
  // flowgroup_feature_discreate_feature_table    smooth_adid_ctr_smooth  2   1
  // -0.455510
  while (getline(ifs, line)) {
    vector<string> tmp;
    // 映射文件由三部分组成，[特征名,特征值,映射后特征名]
    AdUtil::Split2(tmp, line, ',');
    if (tmp.empty() || tmp.size() < 3) {
      AdUtil::Split2(tmp, line, '\t');
      if (tmp.empty() || tmp.size() < 3) {
        tmp.clear();
        AdUtil::Split2(tmp, line, ' ');
        if (tmp.empty() || tmp.size() < 3) {
          tmp.clear();
          AdUtil::Split2(tmp, line, '^A');
        }
      }
    }
    string feature_name = "";
    string feature_value = "";
    int64_t mapping_value = 0, feature_value_int = -1;
    bool int_flag = false;
    if (tmp.size() >= 3) {
      feature_name = tmp[0];
      feature_value = tmp[1];

      // custid,adid,age,gender,platform,phone_brand,location,network_type,pv_hour,bid_type

      int_flag = isLong(tmp[1], &feature_value_int);

      if (!isLong(tmp[2], &mapping_value)) continue;
      if (cnt <= 5)
        DEBUG(LOGNAME, "%s,%s --> %ld", feature_name.c_str(),
              feature_value.c_str(), mapping_value);
      if (mFeatureMappingKey.find(feature_name) != mFeatureMappingKey.end()) {
        mFeatureMappingKey[feature_name].push_back(feature_value);
        mFeatureMappingValue[feature_name][feature_value] = mapping_value;
        if (int_flag) {
          mFeatureMappingKeyInt[feature_name].push_back(feature_value_int);
          mFeatureMappingValueInt[feature_name][feature_value_int] =
              mapping_value;
        }
      } else {
        vector<string> value_all;
        vector<int64_t> value_all_int;
        value_all.push_back(feature_value);
        if (int_flag) value_all_int.push_back(feature_value_int);

        mFeatureMappingKey[feature_name] = value_all;
        mFeatureMappingKeyInt[feature_name] = value_all_int;

        unordered_map<string, int64_t> value_mapping;
        unordered_map<int64_t, int64_t> value_mapping_int;
        value_mapping[feature_value] = mapping_value;
        if (int_flag) value_mapping_int[feature_value_int] = mapping_value;

        mFeatureMappingValue[feature_name] = value_mapping;
        mFeatureMappingValueInt[feature_name] = value_mapping_int;
      }
    } else {
      ERROR(LOGNAME, "error line=%s", line.c_str());
      continue;
    }
    cnt++;
  }
  DEBUG(LOGNAME, "feature mapping size=%d", mFeatureMappingKey.size());
  mHasFeatureMappingFlag = true;
  return true;
}

int64_t FeatureMapData::featureMapping(const string& feature_name,
                                       const int64_t& feature_value, int type) {
  if (!mHasFeatureMappingFlag) {
    // 如果当前模型影射文件不存在，则直接返回特征值
    return -1;
  }
  //二次查找
  unordered_map<string, unordered_map<int64_t, int64_t> >::iterator
      feature_mapping_iter = mFeatureMappingValueInt.find(feature_name);
  if (feature_mapping_iter != mFeatureMappingValueInt.end()) {
    unordered_map<int64_t, int64_t>::iterator feature_value_mapping_iter =
        feature_mapping_iter->second.find(feature_value);
    if (feature_value_mapping_iter != feature_mapping_iter->second.end()) {
      return feature_value_mapping_iter->second;
    }
    DEBUG(LOGNAME,
          "getweight, find approximate for feature_name=%s, feature_value=%ld",
          feature_name.c_str(), feature_value);
    if (type != 0) {
      //寻找编过号的最接近的特征值
      unordered_map<string, vector<int64_t> >::iterator field_value_it =
          mFeatureMappingKeyInt.find(feature_name);
      if (field_value_it == mFeatureMappingKeyInt.end())  //如果找不到该field
        return -1;
      int64_t approximate_value =
          findApproximateValue(feature_value, field_value_it->second);
      unordered_map<int64_t, int64_t>::iterator appro_it =
          feature_mapping_iter->second.find(approximate_value);
      if (appro_it != feature_mapping_iter->second.end())
        return appro_it->second;
      else
        return -1;
    }
  } else {
    // WARN(LOGNAME, "In m_featureValueToIdMap: can't find feature_name = %ld",
    // feature_name);
    return -1;
  }
  return -1;
}

int64_t FeatureMapData::featureMapping(const string& feature_name,
                                       const string& feature_value) {
  if (!mHasFeatureMappingFlag) {
    // 如果当前模型影射文件不存在，则直接返回特征值
    return -1;
  }
  //二次查找
  unordered_map<string, unordered_map<string, int64_t> >::iterator
      feature_mapping_iter = mFeatureMappingValue.find(feature_name);
  if (feature_mapping_iter != mFeatureMappingValue.end()) {
    unordered_map<string, int64_t>::iterator feature_value_mapping_iter =
        feature_mapping_iter->second.find(feature_value);
    if (feature_value_mapping_iter != feature_mapping_iter->second.end()) {
      return feature_value_mapping_iter->second;
    }
    DEBUG(LOGNAME,
          "getweight, find approximate for feature_name=%s, feature_value=%s",
          feature_name.c_str(), feature_value.c_str());
  } else {
    // WARN(LOGNAME, "In m_featureValueToIdMap: can't find feature_name = %ld",
    // feature_name);
    return -1;
  }
  return -1;
}

int64_t FeatureMapData::findApproximateValue(
    int64_t value, const vector<int64_t>& indexed_value_vec) {
  int total_len = (int)indexed_value_vec.size();
  if (total_len == 0) {
    return -1;
  }
  int start = 0;
  int end = total_len - 1;
  int mid = (start + end) / 2;
  while (start < end) {
    if (indexed_value_vec[mid] < value)
      start = mid + 1;
    else
      end = mid - 1;
    mid = (start + end) / 2;
  }
  DEBUG(LOGNAME, "mid=%d", mid);
  //和mid位置左右的值比较哪个更接近value
  int64_t appromiate_value = indexed_value_vec[mid];
  if (mid > 0) {
    if (abs(indexed_value_vec[mid - 1] - value) < abs(appromiate_value - value))
      appromiate_value = indexed_value_vec[mid - 1];
  }
  if (mid < total_len - 1) {
    if (abs(indexed_value_vec[mid + 1] - value) < abs(appromiate_value - value))
      appromiate_value = indexed_value_vec[mid + 1];
  }
  DEBUG(LOGNAME, "find the approximate value=%ld,for value=%ld",
        appromiate_value, value);
  return appromiate_value;
}

}  // namespace rank

