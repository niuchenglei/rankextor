#ifndef RANK_INTERFACE_MERGE_H
#define RANK_INTERFACE_MERGE_H

#include <stdint.h>
#include <map>
#include <vector>
#include <set>
#include <string>
//#include "json/json.h"
#include <string.h>
#include "Interfaces/define.h"
#include "Interfaces/rank_interface.pb.h"

namespace rank {

bool set_user(rank::interface::UserInfo* user_info, UserInfo& user) {
  int kv_size = user_info->extend_ss_size();
  for (int k=0; k<kv_size; k++) {
    const rank::interface::kv_ss& kv = user_info->extend_ss(k);

    //if (kv.key() == "__runing_strategy__")
    user.extend[kv.key()] = kv.value();
  }
  
  user.uid = user_info->uid(); 
  user.extend["uid"] = TO_STRING(user.uid); 
  user._ptr = user_info;

  return true;
}

bool set_request(rank::interface::RequestInfo* request_info, RankRequest& request) {
  request._ptr = request_info;

  int kv_size = request_info->extend_ss_size();
  for (int k=0; k<kv_size; k++) {
    const rank::interface::kv_ss& kv = request_info->extend_ss(k);

    //if (kv.key() == "__runing_strategy__")
    request.extend[kv.key()] = kv.value();
  }

  return set_user(request_info->mutable_user(), request.user_info);
}

bool set_ad(rank::interface::AdInfo* ad_info, RankAdInfo& ad) {
  ad.ad_id = ad_info->ad_id();
  ad.customer_id = ad_info->cust_id();
  ad.bid_type = ad_info->bid_type();

  if (ad.bid_type == rank::CPM || ad.bid_type == rank::OCPM) { 
    ad.bid_price = ad_info->bid_price() / 100000.0;
    ad.max_bid_price = ad_info->max_bid_price() / 100000.0;
  } else if (ad.bid_type == rank::CPC) {
    ad.bid_price = ad_info->bid_price() / 100.0;
    ad.max_bid_price = ad_info->max_bid_price() / 100.0;
  }

  ad.delivery_speed = ad_info->delivery_speed();
  ad.daily_quota = ad_info->daily_quota();
  ad.budget = ad_info->budget();

  string ava_str = "";
  int ava_size = ad_info->available_items_size();
  for (int i=0; i<ava_size; i++) {
    const rank::interface::kv_ii& _kv = ad_info->available_items(i);
    ad.available_items[_kv.key()] = _kv.value();
    ava_str += TO_STRING(_kv.key()) + ",";
  }
  ad.extend["_available_items"] = ava_str;

  int kv_size = ad_info->extend_ss_size();
  for (int k=0; k<kv_size; k++) {
    const rank::interface::kv_ss& kv = ad_info->extend_ss(k);
    ad.extend[kv.key()] = kv.value();
  }

  ad._ptr = ad_info;

  return true;
}

bool set_item(rank::interface::ItemInfo* item_info, RankItemInfo& item) {
  item.item_id = item_info->item_id();
  item.owner_id = item_info->owner_id();
  item.style_type = item_info->style_type();
  item.type = item_info->type();
  item.object_id = item_info->object_id();
  item.text = item_info->text();
  item.tag = item_info->tag();
  item.ctr = item_info->ctr();
  item.cvr = item_info->cvr();
  item.ctcvr = item_info->ctcvr();
  item.score = item_info->score();

  int kv_size = item_info->extend_ss_size();
  for (int k=0; k<kv_size; k++) {
    const rank::interface::kv_ss& kv = item_info->extend_ss(k);
    item.extend[kv.key()] = kv.value();
  }
  item.extend["item_id"] = TO_STRING(item.item_id);

  item._ptr = item_info;

  return true;
}

bool set_union(rank::interface::Union* _union, RankRequest& request, std::vector<RankAdInfo>& ad_list, std::vector<RankItemInfo>& item_list, int shrink_item_size) {
  rank::interface::RequestInfo* _request = _union->mutable_request();

  if (!set_request(_request, request))
    return false;

  int ll = _union->ads_size();
  ad_list.resize(ll);
  for (int i=0; i<ll; i++) {
    rank::interface::AdInfo* _ad = _union->mutable_ads(i);
    RankAdInfo& ad = ad_list[i];
        
    if (!set_ad(_ad, ad))
      return false;
  }

  ll = _union->items_size();
  if (shrink_item_size > 5) 
    ll = (ll>shrink_item_size)?shrink_item_size:ll;
  item_list.resize(ll);
  for (int i=0; i<ll; i++) {
    rank::interface::ItemInfo* _item = _union->mutable_items(i);
    RankItemInfo& item = item_list[i];
 
    if (!set_item(_item, item))
      return false;
  }

  return true;
}

bool set_response(rank::interface::ResponseInfo* _response, 
                  rank::interface::Union* _union, 
                  std::vector<RankAdInfo>& ad_list, 
                  std::vector<RankItemInfo>& item_list,
                  std::vector<ad_item_pair_t>& pair_list) {

  if (ad_list.size() != _union->ads_size())
    return false;
  if (item_list.size() != _union->items_size())
    return false;

  int _ad_size = ad_list.size(), _item_size = item_list.size(),
      u_ad_size = _union->ads_size(), u_item_size = _union->items_size();

  const string& res_type = _union->request().response_type();
  if (res_type.find("whole") != std::string::npos) {
    for (int k=0; k<u_ad_size; k++) {
      rank::interface::AdInfo* ad = _response->add_ads();
      const rank::interface::AdInfo& _ad = _union->ads(k);

      *ad = _ad;

      if (k >= _ad_size) continue;
      RankAdInfo& fisher_ad = ad_list[k];
      ad->set_ad_id(fisher_ad.ad_id);
      ad->set_cust_id(fisher_ad.customer_id);
      ad->set_bid_type(fisher_ad.bid_type);

      // 外部价格单位为 /100万 = 元，内部为万次点击/曝光=元
      if (fisher_ad.bid_type == rank::CPM || fisher_ad.bid_type == rank::OCPM) {
        ad->set_bid_price(fisher_ad.bid_price * 100000.0);
      } else if (fisher_ad.bid_type == rank::CPC) {
        ad->set_bid_price(fisher_ad.bid_price * 100.0);
      }

      ad->set_ecpm(fisher_ad.ecpm);
      ad->set_cost(fisher_ad.cost);
      ad->set_score(fisher_ad.score);
      ad->set_ctr(fisher_ad.ctr);
      ad->set_cvr(fisher_ad.cvr);
      ad->set_ctcvr(fisher_ad.ctcvr);
      ad->set_filter_strategy(fisher_ad.filter_strategy);
    }

    for (int k=0; k<u_item_size; k++) {
      rank::interface::ItemInfo* item = _response->add_items();
      const rank::interface::ItemInfo& _item = _union->items(k);

      *item = _item;

      if (k >= _item_size) continue;
      RankItemInfo& fisher_item = item_list[k];
      item->set_item_id(fisher_item.item_id);
      item->set_owner_id(fisher_item.owner_id);
      item->set_score(fisher_item.score);
      item->set_ctr(fisher_item.ctr);
      item->set_cvr(fisher_item.cvr);
      item->set_ctcvr(fisher_item.ctcvr);
      item->set_filter_strategy(fisher_item.filter_strategy);
    }
  }

  for (int k=0; k<pair_list.size(); k++) {
    rank::interface::AdItemPair* _pair = _response->add_pairs();
    ad_item_pair_t& _fisher_pair = pair_list[k];

    RankAdInfo& fisher_ad = ad_list[_fisher_pair.ad_idx];
    RankItemInfo& fisher_item = item_list[_fisher_pair.item_idx];

    _pair->set_ad_id(_fisher_pair.ad_id);
    _pair->set_ad_idx(_fisher_pair.ad_idx);
    _pair->set_item_id(_fisher_pair.item_id);
    _pair->set_item_idx(_fisher_pair.item_idx);

    if (fisher_ad.bid_type == rank::CPM || fisher_ad.bid_type == rank::OCPM) {
      _pair->set_bid_price(_fisher_pair.bid_price * 100000.0);
    } else if (fisher_ad.bid_type == rank::CPC) {
      _pair->set_bid_price(_fisher_pair.bid_price * 100.0);
    }
    //_pair->set_bid_price(_fisher_pair.bid_price);
    _pair->set_cost(_fisher_pair.cost);
    _pair->set_ctr(fisher_item.ctr);
    _pair->set_cvr(fisher_item.cvr);
    _pair->set_ctcvr(fisher_item.ctcvr);

    _pair->set_filter_strategy(0);
    if (_fisher_pair.filter_strategy != 10000 && _fisher_pair.filter_strategy != 0)
      _pair->set_filter_strategy(_fisher_pair.filter_strategy);
    else if (fisher_ad.filter_strategy != 10000 && fisher_ad.filter_strategy != 0)
      _pair->set_filter_strategy(fisher_ad.filter_strategy);
    else if (fisher_item.filter_strategy != 10000 && fisher_item.filter_strategy != 0)
      _pair->set_filter_strategy(fisher_item.filter_strategy);
    if (_pair->filter_strategy() == 10000)
      _pair->set_filter_strategy(0);
  }

  return true;
}

}
#endif
