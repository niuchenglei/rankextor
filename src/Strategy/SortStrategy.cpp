#include "Strategy/SortStrategy.h"
#include "Interfaces/Service.h"
#include "Interfaces/ConfigurationSettings.h"

using namespace boost;

namespace rank {
REGISTE_CLASS("sort", SortStrategy)

static bool sortBySimpleAd(const ad_item_pair_t& lhs, const ad_item_pair_t& rhs) {
  if (&lhs == &rhs) return false;
  if (lhs.score == rhs.score) {
    return (lhs.ad_id < rhs.ad_id);
  }
  return (lhs.score < rhs.score);
}

SortStrategy::~SortStrategy() {
  Service<ResourceManager> resource_manager;
  resource_manager->UnRegisterResource(price_ctx_key);
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool SortStrategy::Initialize(
    strategy_arg_t& arg,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  StrategyBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mStrategyDir = pSetting->getSetting(alias + "/strategy_dir");

  mArg = arg;
  if (mArg.config != "" && mArg.config != " ")
    mConfigFile = mStrategyDir + "/" + mArg.config;
  else
    mConfigFile = mStrategyDir + "/" + mArg.name + ".txt";
  REGIST_HANDLER(mConfigFile);


  // 一下内容与 Ocpx.cpp 一致
  string _name = pSetting->getSetting("main/plugin_name");
  price_ctx_key = _name + "_price_context";

  // 因为多线程可能同时初始化同一份key的资源
  pthread_mutex_lock(&mutex);
  REGISTE_RESOURCE_TOUCH(price_ctx_key, "", PriceContext, resource_manager);
  pthread_mutex_unlock(&mutex);

  mpPriceCtx = NULL;
  Resource* res = NULL;
  if (resource_manager->FetchResource(price_ctx_key, &res) == 0 && res != NULL) {
    mpPriceCtx = static_cast<PriceContext*>(res);
  } else {
    FATAL(LOGNAME, "resource_manager fetch %s failed", price_ctx_key.c_str());
    return false;
  }
  DEBUG(LOGNAME, "price_ctx:%x", mpPriceCtx);

  return Update();
}

bool SortStrategy::Filter(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  DEBUG(LOGNAME, "sortstrategy begin");

  stable_sort(ptd.rank_order.begin(), ptd.rank_order.end(), sortBySimpleAd);
  reverse(ptd.rank_order.begin(), ptd.rank_order.end());


  // 为了把排序后的广告出价，记录到上下文中，把这部分逻辑放到排序策略中（也可以独立策略，逻辑较少所以放到排序策略中）
  // 1. 升序获取一个ad下最高出价的pair
  boost::unordered_map<int64_t, int> ad_ind;
  for (int i=ptd.rank_order.size()-1; i>=0; i--) {
    ad_item_pair_t& pair = ptd.rank_order[i];
    
    ad_ind[pair.ad_id] = i;
  }

  // 2. 把出价历史累加到PriceContext中
  for (boost::unordered_map<int64_t, int>::iterator _iter=ad_ind.begin(); _iter!=ad_ind.end(); _iter++) {
    ad_item_pair_t& pair = ptd.rank_order[_iter->second];
    //RankAdInfo& ad = ad_list[pair.ad_idx];
   // RankAdInfo& ad = ad_list_for_rank[pair.ad_idx]
    //int available_items_num = ad.available_items.size();
    
    RankItemInfo& item = item_list_for_rank[pair.item_idx];
    
    float bid = pair.bid_price;
    //if (bid < 0.0001) continue;

    float bid_cpc = 0.0f;
    if (item.extend_sf["bid_lc"] > 0.00001){ 
       bid_cpc = pair.bid_price / (item.extend_sf["bid_lc"] + 0.0000001);
    }

    boost::unordered_map<uint64_t, PriceContext::price_ctx_t>::iterator iter = mpPriceCtx->mAdInfoPriceCtx.find(pair.ad_id);
    if (iter != mpPriceCtx->mAdInfoPriceCtx.end()) {
      PriceContext::price_ctx_t& ctx = iter->second;
      ctx.bid_acc += bid; 
      ctx.bid_acc_cnt += 1;
      ctx.bid_cpc_acc += bid_cpc; 
      ctx.bid_cpc_acc_cnt += 1;
      if (item.extend_sf["bid_lc"] < 0.00001 || bid > 9999) {
        FATAL(LOGNAME, "adid:%ld,item_id:%ld,bid:%.5f,bid_cpc:%.5f,ctx:%x,item_ctr:%.8f(%d),bid_cpc_acc:%.5f,bid_cpc_acc_cnt:%d", pair.ad_id, pair.item_id, bid, bid_cpc, &ctx, item.extend_sf["bid_lc"], (item.extend_sf.find("bid_lc")!=item.extend_sf.end())?1:0,  ctx.bid_cpc_acc, ctx.bid_cpc_acc_cnt);
      }
      NOTICE(LOGNAME, "adid:%ld, item_id:%ld, bid:%.5f, bid_cpc:%.5f, ctx:%x, item_ctr:%.8f, bid_cpc_acc:%.5f, bid_cpc_acc_cnt:%d", pair.ad_id, pair.item_id, bid, bid_cpc, &ctx, item.extend_sf["bid_lc"],  ctx.bid_cpc_acc, ctx.bid_cpc_acc_cnt);
    }
    DEBUG(LOGNAME, "add[%d] price ctx:[%.4f, %.4f] to adid:%ld", (iter != mpPriceCtx->mAdInfoPriceCtx.end())?1:0, bid, bid_cpc, pair.ad_id);
  }
  
  DEBUG(LOGNAME, "sortstrategy end");
  return true;
}

bool SortStrategy::Update() {
  return true;
  PairReader reader(mConfigFile);
  if (!reader.isValid()) {
    DEBUG(LOGNAME, "strategy conf parse error [%s]", mConfigFile.c_str());
    return false;
  }

  //cpm_reorder_byO = 0.0f;
  //reader.getValue("cpm_reorder_byO", cpm_reorder_byO);

  return true;
}

}  // namespace rank
