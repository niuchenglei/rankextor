#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>
#include <map>
//#include <unordered_map>
#include <vector>
#include <set>
#include <string>
//#include "json/json.h"
#include <string.h>

namespace rank {

// 内部使用的物料详细信息
struct RankItemInfo {
  int64_t item_id;        // 物料id
  int64_t owner_id;       // 拥有者id
  int64_t type;           // 计划类型

  int64_t style_type;     // 物料样式类型,枚举
  std::string object_id;  // 物料对象id
  std::string text;       // 物料描述
  std::string tag;        // 物料标签

  float ctr, cvr, ctcvr;  // 预估ctr
  float score;            // 打分
  int64_t bid_price;      // 物料侧出价

  boost::unordered_map<std::string, std::string> extend;  // 扩展字段
  boost::unordered_map<std::string, float> extend_sf;
  int64_t filter_strategy;
 
  void* _ptr;         // 指向protobuf数据内存

  RankItemInfo() { Reset(); }
  void Reset() {
    item_id = 0;
    owner_id = 0;
    type = 0;
    style_type = 0;
    ctr = 0.0;
    cvr = 0.0;
    ctcvr = 0.0;
    score = 0.0;
    extend.clear();
    _ptr = NULL;
    filter_strategy = 0;
  }

  bool operator==(const RankItemInfo& rhs) const { return item_id == rhs.item_id; }
};

// 内部使用的计划详细信息
struct RankAdInfo {
  int64_t ad_id;        // 计划id
  int64_t ad_type;      // 计划类型
  int64_t customer_id;  // 广告主id
  int64_t campaign_id;  // 组id
 
  int64_t bid_type;       // 竞价类型,枚举
  int64_t bid_price;      // 出价
  int64_t max_bid_price;  // 针对ocpm，广告主设置的最大出价
  int64_t delivery_speed;          // 加速投放

  int64_t daily_quota;                   // 日限额
  int64_t budget;                        // 预算

  int64_t ecpm;           // ecpm，单位：毫
  int64_t cost;           // 扣费价格，单位：毫
  float ctr, cvr, ctcvr;  // 预估ctr
  float score;       // 打分

  boost::unordered_map<std::string, std::string> extend;  // 扩展字段
  boost::unordered_map<std::string, float> extend_sf;
  int64_t filter_strategy;

  boost::unordered_map<int64_t, int> available_items;
 
  void* _ptr;         // 指向protobuf数据内存

  RankAdInfo() { Reset(); }
  void Reset() {
    ad_id = 0;
    ad_type = 0;
    customer_id = 0;
    campaign_id = 0;
    bid_type = 0;
    bid_price = 0;
    delivery_speed = 0;
    daily_quota = 0;
    budget = 0;
    ecpm = 0;
    cost = 0;
    ctr = 0.0;
    cvr = 0.0;
    score = 0.0;
    extend.clear();
    _ptr = NULL;
    filter_strategy = 0;
  }

  bool operator==(const RankAdInfo& rhs) const { return ad_id == rhs.ad_id; }
};

// 内部使用的用户详细信息
struct UserInfo {  
  int64_t uid;
  int64_t age;                  // 年龄
  int64_t gender;               // 性别
  int64_t live_location;        // 常驻地域
  int64_t resident_location;    // 实时地理信息
  std::vector<int64_t> category_interest;     // 兴趣
  std::vector<std::string> accurate_interest; // 类目

  int64_t device;               // 设备类型，PC端or安卓or苹果
  int64_t network_type;         // 网络类型
  int64_t os_version;           // 操作系统版本
  int64_t mobile_brand;         // 手机品牌
  std::string client_version;   // 客户端版本
  std::string attachment;

  boost::unordered_map<std::string, std::string> extend;// 扩展字段*/

  // 用户基本属性
  std::string age_code;
  std::string age_segment; 
  std::string gender_code; 

  void* _ptr;     // 指向protobuf用户数据结构内存
};

// 内部使用的请求体详细信息
struct RankRequest {
  std::string pvid;                            // 请求唯一标识符
  int64_t psid;  
  int64_t uid;                                 // uid
  int64_t age;                                 // 年龄
  int64_t gender;                              // 性别
  int64_t live_location;                       // 常驻地域
  int64_t resident_location;                   // 实时地理信息
  
  int64_t device;              // 设备类型，PC端or安卓or苹果
  int64_t network_type;        // 网络类型
  int64_t os_version;          // 操作系统版本
  int64_t mobile_brand;        // 手机品牌
  std::string client_version;  // 客户端版本
  std::string attachment;

  boost::unordered_map<std::string, std::string> extend;
  UserInfo user_info;

  void* _ptr;         // 指向protobuf数据内存

  RankRequest() { Reset(); }
  void Reset() {
    uid = 0;
    age = 0;
    gender = 0;
    live_location = 0;
    resident_location = 0;
    device = 0;
    network_type = 0;
    os_version = 0;
    mobile_brand = 0;
    client_version.clear();
    attachment.clear();
    psid = 0;
    pvid.clear();
    extend.clear();
  }
};

typedef struct {
  int64_t ad_id, ad_idx, item_id, item_idx;
  float score;
  int64_t bid_price, cost, filter_strategy;
} ad_item_pair_t;

}  // namespace

#endif
