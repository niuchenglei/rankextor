#include <iostream>
#include "Feature/RequestFeature.h"
#include "Interfaces/PairReader.h"
//#include "Interfaces/rank_interface.pb.h"

using namespace boost;

// custid,adid,feedid,label,age,gender,platform,phone_brand,location,network_type,pv_hour,bid_type

//#define FAKE_FEATURE

#define FAKE_COMMON(x, y) {string v=y; ADD_COMMON_FEATURE(x, v);}
#define FAKE_ITEM(m, x, y) {string v=y; ADD_FEATURE(m, x, v);}

namespace rank {

REGISTE_CLASS("request_feature", RequestFeature)

RequestFeature::RequestFeature() {}
RequestFeature::~RequestFeature() {}

bool RequestFeature::Initialize(
    feature_arg_t& arg, const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  FeatureBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mFeatureDir = pSetting->getSetting(alias + "/feature_dir");
  mModelDir = pSetting->getSetting(alias + "/model_dir");
  string _name = pSetting->getSetting("main/plugin_name");
  mArg = arg;
  srand((unsigned)time(NULL));

  if (mArg.type == "FILE") {
    mRedisSchemaFile = mFeatureDir + "/" + mArg.data["redis_user_schema"];

    REGIST_HANDLER(mRedisSchemaFile);
    //REGISTE_RESOURCE_TOUCH(mFeatureMapFile, mFeatureMapFile + ".touch", FeatureMapData, resource_manager);
  } else {
    ERROR(LOGNAME, "config from redis");
    return false;
  }

  string v = "impression_id,click_flag,solution_id,exchange_id,main_media_domain,allyes_client_type,allyes_client_info,ad_space_id,banner_id,width,height,size,bid_time,categ1_id,categ2_id,categ3_id,catelog4_id,brand_name,age,app_prefer,gender,is_cart_nocommit_0_15d,is_cart_nocommit_0_3d,is_cart_nocommit_0_7d,price_sensitive,purchase_power,work_day_prefer,work_time_prefer,brandcart_cnt_3d,brandorder_cnt_3d,brandsubmit_cnt_15d,brandsubmit_cnt_5d,itembrowse_cnt_15d,itembrowse_cnt_30d,itemiscart_cnt_15d,itemiscart_cnt_30d,itemiscollect_cnt_15d,itemiscollect_cnt_30d,itemissearch_cnt_15d,itemissearch_cnt_30d,itemissubmit_cnt_15d,itemissubmit_cnt_30d,categ3browse_cnt_15d,categ3cart_cnt_3d,categ3isbrowse_cnt_15d,categ3isbrowse_cnt_30d,categ3iscart_cnt_30d,categ3iscart_cnt_7d,categ3iscollect_cnt_15d,categ3iscollect_cnt_30d,categ3search_cnt_3d,categ3submit_cnt_5d,gds_cd,cate1_pv_click,cate2_pv_click,cate1_detail_pv_click,cate2_detail_pv_click,view_3,view_7,view_15,sale_3,sale_7,sale_15,collect_3,collect_7,collect_15,imp_3,imp_7,imp_15,clk_3,clk_7,clk_15,cvt_3,cvt_7,cvt_15,praiserate";

  vector<string> tmp;
  AdUtil::Split2(tmp, v, ',');
  for (int k=0; k<tmp.size(); k++) {
    __F(tmp[k].c_str());
  }

  __F("---------------------------");
  __F("put any thing here you want");
  __F("---------------------------");
  // mpList["custid,adid,feedid,label,age,gender,platform,phone_brand,location,network_type,pv_hour,bid_type
  //

  empty_feature = "-";
  mIndex = 0;
  return Update();
}

#define UF(x) { if(fisher_request.user_info.extend.find(x) != fisher_request.user_info.extend.end()) \
  { ADD_COMMON_FEATURE(x, fisher_request.user_info.extend[x]); } \
else \
  { ADD_COMMON_FEATURE(x, empty_feature); } \
}

#define UF2(x, y) { if (fisher_request.extend.find(y) != fisher_request.extend.end()) \
  { ADD_COMMON_FEATURE(x, fisher_request.extend[y]); } \
else \
  { ADD_COMMON_FEATURE(x, empty_feature); } \
}

#define RF(x) { if(fisher_request.extend.find(x) != fisher_request.extend.end()) \
  {ADD_COMMON_FEATURE(x, fisher_request.extend[x]);} \
else \
  {ADD_COMMON_FEATURE(x, empty_feature);} \
}

#define AF(x, y) { \
if (item.extend.find(y) != item.extend.end()) { \
  if (fisher_request.user_info.extend.find(x + item.extend[y]) != fisher_request.user_info.extend.end()) { \
    ADD_FEATURE(m, x, fisher_request.user_info.extend[x + item.extend[y]]); } \
  else { ADD_FEATURE(m, x, empty_feature); } \
} else { ADD_FEATURE(m, x, empty_feature); } \
}

bool RequestFeature::FetchFeature(RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {

  int size = item_list_for_rank.size();

  boost::unordered_map<string, string>::const_iterator iter = fisher_request.user_info.extend.find("user_feature");
  if (iter == fisher_request.user_info.extend.end()) {
    ERROR(LOGNAME, "can not find user feature 'user_feature' from fisher_request.user_info.extend");
  } else {
    parse_user_data(iter->second, fisher_request.user_info);
  }

  for (int i=0; i<size; i++) {
    iter = item_list_for_rank[i].extend.find("gds_property");
    if (iter == item_list_for_rank[i].extend.end()) {
      ERROR(LOGNAME, "can not find item 'gds_property' from tem_list_for_rank[i].extend");
      continue;
    }
    parse_item_data(iter->second, item_list_for_rank[i]);
  }

#ifdef FAKE_FEATURE
  FAKE_COMMON("age","-")

#else
  // 用户基本用户画像特征
  UF("uid")
  UF("age")
  UF("app_prefer")
#endif
  
  int omp_thread_num = NUM_THREAD;
  if (omp_thread_num*3 > size)
    omp_thread_num = float(size)/3.0;
  int m = 0;

  string default_value = "-";
#ifdef OMP
#pragma omp parallel for num_threads(omp_thread_num) shared(item_list_for_rank, ptd) private(m)
#endif
  for (m=0; m<size; m++) {
    RankItemInfo& item = item_list_for_rank[m];

#ifdef FAKE_FEATURE
    FAKE_ITEM(m, "gds_cd", "10000027224");
#else
    ADD_FEATURE(m, "gds_cd", item.extend["item_id"]);
#endif

  }
 
  DEBUG(LOGNAME, "common feature: %d, ad feature: %d*%d", ptd.common_features.features_group["begin"].size(), ptd.features.size(), ptd.features[0].features_group["begin"].size()); 
  // FATAL(LOGNAME, "total: %d, %d, %d", adsize, cnt[1], cnt[0]);
  return true;
}

#undef UF
#undef RF
#undef AF

bool RequestFeature::Update() { 
  
  return true; 
}

}  // namespace rank
