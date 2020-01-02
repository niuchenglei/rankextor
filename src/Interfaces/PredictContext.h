#ifndef PREDICTCONTEXT_H
#define PREDICTCONTEXT_H

#include <string>
#include <boost/unordered_map.hpp>
#include "Interfaces/EventHandler.h"
#include <pthread.h>
#include <queue>

namespace rank {

class PredictContext : public Resource {
public:
  PredictContext();
  virtual int Load(const std::string& config_file) { return 0; }
  virtual int GetValue(const std::string& key, std::string* value) { return 0; }
  virtual int GetValue(std::map<std::string, std::string>* key_value_map) {
    return 0;
  }

  // 100000*(20+6+10+6+3+2)*4/1024/1024 = 15MB
  typedef struct {
    // slot
    float value[20];  // 5-avg,5-var,5-avg41,5-avg43
    volatile int idx, cnt_slot, cnt, cnt41, cnt43, cnt_0;
    // pool    avg, var, avg41, avg43为汇总slot后回填数据
    volatile float sum, avg, var, var_sum, sum41, sum43, avg41, avg43, min, max,
        sum_0;
    // volatile float bin_sum[16], bin_cnt[16];
    volatile int cnt_left, cnt_right, cnt_left_41, cnt_right_41, cnt_left_43,
        cnt_right_43;
    volatile float median, median41, median43;
    int64_t timestamp;
  } pred_ctx_t;
  boost::unordered_map<uint64_t, pred_ctx_t> mAdInfoPredCtx;
};
extern "C" rank::Resource* create_PredictContext();
extern "C" void destroy_PredictContext(rank::Resource* ins);

}  // namespace rank

#endif
