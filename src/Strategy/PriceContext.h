#ifndef PRICE_CONTEXT_H
#define PRICE_CONTEXT_H

#include <sys/time.h>
#include <string>
#include <boost/unordered_map.hpp>
#include "Interfaces/EventHandler.h"

namespace rank {

class PriceContext : public Resource {
public:
  PriceContext();
  virtual int Load(const std::string& config_file) { return 0; }
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  // 100000*(45)*4/1024/1024 = 17MB
  // bid_acc 记录5分钟累计CPM出价, bid_acc_cnt记录次数
  struct price_ctx_t {
    float bid[12], bid_cpc[12], speed[12], bid_advise;
    float imp[2], consume[2]; // 保存2次5分钟曝光和消耗
    struct timeval timestamp;
    int index, num;

    // 记录历史统计出价
    volatile float bid_acc, bid_cpc_acc;
    volatile int bid_acc_cnt, bid_cpc_acc_cnt;

    price_ctx_t() {
      index = 0; num = 0; bid_advise = 0; bid_acc = 0; bid_cpc_acc = 0; bid_acc_cnt = 0; bid_cpc_acc_cnt = 0;
      for (int nn=0; nn<12; nn++) {
        bid[nn] = 0;
        bid_cpc[nn] = 0;
        speed[nn] = 0;
      }
    }
  };
  boost::unordered_map<uint64_t, price_ctx_t> mAdInfoPriceCtx;
};
extern "C" rank::Resource* create_PriceContext();
extern "C" void destroy_PriceContext(rank::Resource* ins);

class PIDContext : public Resource {
public:
  PIDContext();
  virtual int Load(const std::string& config_file) { return 0; }
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  typedef struct {
    volatile float Ek;                       //当前误差
    volatile float Ek1;                      //前一次误差 e(k-1)
    volatile float Ek2;                      //再前一次误差 e(k-2)
    struct timeval timestamp;
    
    float bid_advise;
    float imp[2], clk[2], cvt[2], consume[2];

    float mu_consume_last_windows;
    float mu_click_last_windows;
  } pid_def_t;
  boost::unordered_map<uint64_t, pid_def_t> mAdInfoPIDCtx;
};
extern "C" rank::Resource* create_PIDContext();
extern "C" void destroy_PIDContext(rank::Resource* ins);

}

#endif
