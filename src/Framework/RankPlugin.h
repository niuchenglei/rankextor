#ifndef RANK_PLUGIN_H
#define RANK_PLUGIN_H

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <tr1/unordered_set>
#include <boost/unordered_map.hpp>
#include <boost/thread/thread.hpp>
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/RankDataContext.h"
#include "Framework/FeatureManager.h"
#include "Framework/ModelManager.h"
#include "Framework/StrategyManager.h"
#include "Framework/ConditionManager.h"
#include "Framework/ExtractorManager.h"

namespace rank {

void uninit_instance();

class RankPluginImp {
public:
  RankPluginImp();
  virtual ~RankPluginImp();

  /*
   * to initialize plugin.
   *
   * @param configure_file the file path of configuration file.
   */
  int Initialize(const std::string& conf); //, ResourceManager* resource_manager);

  /*
   * to release resources.
   */
  int UnInit();

  /*
   * to reload configuration file.
   *
   * @param configure_file the file path of configuration file.
   */
  int Reload();

  void GetPluginName(std::string* name);
  void GetPluginType(std::string* type);
  void GetPluginVersion(std::string* version);

  // fisher_request [输入参数] 请求参数
  // ad_list_for_rank [输入参数]
  // 输入rank的全量计划列表，不包含之前被过滤的计划列表
  // ad_list_dropped_by_filter [输入参数] 被用户体验过滤掉的计划列表
  // resource_manager [输入参数] 数据共享接口
  // post_ad_list_for_rank [输出参数] 胜出的广告计划
  // rank_match_info [输出参数] rank插件输出的记录信息
  // @return: 0(SUCCESS)
  virtual int Rank(const RankRequest& fisher_request,
                   const std::vector<RankAdInfo>& ad_list_dropped_by_filter,
                   std::vector<RankAdInfo>* ad_list_for_rank,
                   ResourceManager* resource_manager,
                   std::vector<RankAdInfo>* post_ad_list_for_rank,
                   std::map<std::string, std::string>* rank_match_info,
                   std::map<int64_t/*position*/, std::vector<int64_t>/*adid_list*/>* pos_adid_map,
                   int worker_id = -1);

  virtual char* Predict(const char* request, int bytes_request, int worker_id = -1, const char* runing_strategy = NULL, int* bytes_write = NULL);
 
  virtual char* Test(const char* request, int bytes_request, int* bytes_write);

private:
  RankPluginImp(const RankPluginImp& rhs);
  void operator=(const RankPluginImp& rhs);
  int DoRank(const RankRequest& fisher_request,
             const std::vector<RankAdInfo>& ad_list_dropped_by_filter,
             std::vector<RankAdInfo>* ad_list_for_rank,
             ResourceManager* resource_manager,
             std::vector<RankAdInfo>* post_ad_list_for_rank,
             std::map<std::string, std::string>* rank_match_info,
             std::map<int64_t/*position*/, std::vector<int64_t>/*adid_list*/>* pos_adid_map,
             int worker_id = -1);

  void record_match_info(const RankRequest& fisher_request, RankDataContext& ptd, 
                  std::vector<RankAdInfo>& ad_list,
                  std::vector<RankItemInfo>& item_list,
                  std::vector<ad_item_pair_t>& pair_list, void* _response);

  ConditionManager* mpConditionManager;
  FeatureManager* mpFeatureManager;
  ExtractorManager* mpExtractorManager;
  ModelManager* mpModelManager;
  StrategyManager* mpStrategyManager;

  RankDataContext mContext;

  std::string mPluginName;
  std::string mVersion;

  long timeuse[10], count[10], cnt[10];
  float ctr[10];
  map<int64_t, int64_t> filter_cnt, filter_cnt2;
  struct timespec pre_log_time;
  std::string _o_msg;
};
}  // namespace rank

#endif
