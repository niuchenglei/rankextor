#include <bitset>
#include <iostream>
#include <typeinfo>

#include "omp.h"
#include "Framework/RankPlugin.h"
#include "Interfaces/RedisManager.h"
#include "Interfaces/Service.h"
#include "Utilities/AdUtil.h"
#include "Utilities/ConfigurationSettingsImp.h"
#include "Utilities/ObjectFactoryImp.h"
#include "ResourceManagerHandler.h"

//#include "Interfaces/rank_interface.pb.h"
#include "Interfaces/rank_interface_merge.h"

using namespace boost;
using std::istringstream;
using std::map;
using std::ostringstream;

#define FAILED -3
#define SUCCESS 0

int OMP_THREAD = 1;
int NUM_THREAD = 1;

namespace rank {

extern char* get_config_file();
extern char* get_log4cpp_config();
RankPluginImp* create_instance() { return new (std::nothrow) RankPluginImp; }

void uninit_instance() {
  ResourceManagerHandler::kill_thread();
  ResourceManagerHandler::destroy();
  EventFile::kill_thread();
  EventFile::destroy();

  ConfigurationSettingsImp::destroy();
  ObjectFactoryImp::destroy();
}

static bool compAdInfo(const RankAdInfo& lhs, const RankAdInfo& rhs) {
  if (&lhs == &rhs) return false;
  if (lhs.bid_price == rhs.bid_price) {
    return (lhs.ad_id < rhs.ad_id);
  }

  return (lhs.bid_price < rhs.bid_price);
}

RankPluginImp::RankPluginImp()
    : mpFeatureManager(NULL),
      mpModelManager(NULL),
      mpStrategyManager(NULL),
      mpExtractorManager(NULL) { _o_msg = "";}
RankPluginImp::~RankPluginImp() { 
  UnInit(); 
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int RankPluginImp::Initialize(const std::string& conf) {
  if (conf.empty() || -1 == access(conf.c_str(), 0)) {
    fprintf(stderr, "RankPluginImp::initialize failed, file no access [%s]", conf.c_str());
    return FAILED;
  }

  pthread_mutex_lock(&mutex);
  char* config_path = get_config_file();
  strcpy(config_path, conf.c_str());
  if ((ObjectFactoryImp::instance() == NULL) ||
      (ConfigurationSettingsImp::instance() == NULL) ||
      (EventFile::instance() == NULL) ||
      (ResourceManagerHandler::instance() == NULL)) {
    //(FeatureRecord::instance() == NULL)) {
    fprintf(stderr,
          "initialize failed, ObjectFactoryImp, ConfigurationSettingsImp, "
          "EventFile or FeatureRecord can not be instance");
    return FAILED;
  }
  struct timespec start, end;
  long time[10];

  clock_gettime(CLOCK_MONOTONIC, &start);
  Service<ConfigurationSettings> pSettings;
  string log4cpp = pSettings->getSetting("log/log4cpp");
  char* log4cpp_config = get_log4cpp_config();
  strcpy(log4cpp_config, log4cpp.c_str());
  if (LoggerImp::getInstance() == NULL) { 
    fprintf(stderr, "LoggerImp::instance() == NULL");
    return FAILED;
  }
  pthread_mutex_unlock(&mutex);
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[0] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  ResourceManager* resource_manager = ResourceManagerHandler::instance()->resource_manager;
  mContext.resource_manager = resource_manager;
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[1] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  string _max_queue_length = pSettings->getSetting("main/max_queue_length");
  mContext.max_queue_length = STRING_TO_INT64(_max_queue_length);
  mContext.max_queue_length = max(5, mContext.max_queue_length);

  // 改多线程采用的临时方案，将来存储与计算分离后，会取消该逻辑
  // mContext.redis_manager = new RedisManager();

  Service<ObjectFactory> pFactory;
  string object_list = pFactory->getObjectList();
  DEBUG(LOGNAME, "object_list: %s", object_list.c_str());

  clock_gettime(CLOCK_MONOTONIC, &end);
  time[2] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // 使用配置组
  string alias = pSettings->getSetting("main/exp_id");

  mpConditionManager = new ConditionManager();
  if (mpConditionManager == NULL ||
      !mpConditionManager->Initialize(alias, resource_manager)) {
    ERROR(LOGNAME, "fail to create ConditionManager");
    return FAILED;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[3] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  std::map<std::string, ConditionBase*>& conditions = mpConditionManager->getAllConditions();
  mpFeatureManager = new FeatureManager();
  if (mpFeatureManager == NULL ||
      !mpFeatureManager->Initialize(alias, conditions, resource_manager)) {
    ERROR(LOGNAME, "fail to create FeatureManager");
    return FAILED;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[4] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  mpModelManager = new ModelManager();
  if (mpModelManager == NULL ||
      !mpModelManager->Initialize(alias, conditions, resource_manager)) {
    ERROR(LOGNAME, "fail to create ModelManager");
    return FAILED;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[5] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  mpStrategyManager = new StrategyManager();
  if (mpStrategyManager == NULL ||
      !mpStrategyManager->Initialize(alias, conditions, resource_manager)) {
    ERROR(LOGNAME, "fail to create StrategyManager");
    return FAILED;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[6] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  mpExtractorManager = new ExtractorManager();
  if (mpExtractorManager == NULL ||
      !mpExtractorManager->Initialize(alias, conditions, resource_manager)) {
    ERROR(LOGNAME, "fail to create ExtractorManager");
    return FAILED;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  time[7] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  clock_gettime(CLOCK_MONOTONIC, &start);

  mPluginName = pSettings->getSetting("main/plugin_name");

  string _num_thread_str = pSettings->getSetting("main/omp_thread");
  int _num_thread = atoi(_num_thread_str.c_str());
  if (_num_thread > 0)
    NUM_THREAD = _num_thread;

  OMP_THREAD = omp_get_num_procs();
  NUM_THREAD = (OMP_THREAD > MAX_THREADS) ? MAX_THREADS : OMP_THREAD;
  NOTICE(LOGNAME, "OMP_THREAD=%d, NUM_THREAD=%d", OMP_THREAD, NUM_THREAD);

  // 记录取redis时间和计数
  for (int k = 0; k < 10; k++) {
    timeuse[k] = 0;
    count[k] = 0;
    ctr[k] = 0;
    cnt[k] = 0;
  }

  FATAL(LOGNAME, "rank_plugin time statis, static:%d, resource:%d, factory:%d, condition:%d, feature:%d, model:%d, strategy:%d, extractor:%d", time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7]);
  return 0;
}

int RankPluginImp::UnInit() {
  DEBUG(LOGNAME, "uninit");

  if (mpFeatureManager != NULL) {
  delete mpFeatureManager;
  mpFeatureManager = NULL;
  }

  if (mpModelManager != NULL) {
  delete mpModelManager;
  mpModelManager = NULL;
  }

  if (mpStrategyManager != NULL) {
  delete mpStrategyManager;
  mpStrategyManager = NULL;
  }

  if (mpExtractorManager != NULL) {
  delete mpExtractorManager;
  mpExtractorManager = NULL;
  }
  // delete mContext.redis_manager;
  // mContext.redis_manager = NULL;

  return 0;
}

int RankPluginImp::Reload() { return 0; }

void RankPluginImp::GetPluginName(std::string* name) { *name = mPluginName; }

void RankPluginImp::GetPluginType(std::string* type) {
  *type = "ranking_plugin::RankPluginImp";

}

void RankPluginImp::GetPluginVersion(std::string* version) {
  *version = __VERSION_NUMBER;
}

char* RankPluginImp::Test(const char* request, int bytes_request, int* bytes_write) {
  rank::interface::Union _union;
  int error_code = 0;
  string error_msg = "";
  bool ret = true;
  if (!_union.ParseFromArray(request, bytes_request)) {
    error_code = 1;
    error_msg = "ParseFromArray error";
    ret = false;
  } 

  rank::interface::ResponseInfo _response;
  if (ret) {
    NOTICE(LOGNAME, "now to set response");
    //std::vector<ad_item_pair_t> pair;
    //set_response(&_response, &_union, _ad_list, _item_list, pair);
  }
  _response.set_code(error_code);
  _response.set_msg(error_msg);

  size_t size = _response.ByteSize();
  char* buffer = (char*)malloc(size);
  if (bytes_write != NULL)
    *bytes_write = size;
  _response.SerializeToArray(buffer, size);

  return buffer;
}

char* RankPluginImp::Predict(const char* request, int bytes_request, int worker_id, const char* runing_strategy, int* bytes_write) {
  //mContext.request_str = string(request);
  ////////////////////
  struct timespec start, end;

  // try-catch rank exception
  //string response = "";
  char* buffer = NULL;
  try {
    bool ret = false;
    int64_t begin_time = 0, end_time = 0, tu = 0;
    size_t ad_size = 0, item_size = 0;
    boost::unordered_map<std::string, std::string>::const_iterator it;
    RankRequest fisher_request;
    std::vector<RankAdInfo> _ad_list;
    std::vector<RankItemInfo> _item_list;

    mContext.worker_idx = worker_id;

    // 1. 从protobuf消息中构建请求结构体
    rank::interface::Union _union;
    if (!_union.ParseFromArray(request, bytes_request)) {
      mContext.error_code = 1;
      mContext.error_msg = "protobuf request parse error";
      goto EXIT_SEP;
    }

    if (_union.ads_size() < 1 || _union.items_size() < 1) {
      mContext.error_code = 1;
      mContext.error_msg = "request ads or item size invalid";
      goto EXIT_SEP;
    }

    // 1.1 使用protobuf数据，构建内部数据结构
    set_union(&_union, fisher_request, _ad_list, _item_list, mContext.max_queue_length);
    NOTICE(LOGNAME, "union(%d,%d) -> (%d,%d)", _union.ads_size(), _union.items_size(), _ad_list.size(), _item_list.size());

    ad_size = _ad_list.size();
    item_size = _item_list.size();
    if (ad_size == 0 || item_size == 0) {
      mContext.error_code = 1;
      mContext.error_msg = "ad list empty";
      goto EXIT_SEP;
    }
    for (int k = 0; k < ad_size; k++) {
      _ad_list[k].filter_strategy = 10000;
    }
    for (int k = 0; k < item_size; k++) {
      _item_list[k].filter_strategy = 10000;
    }

    // 2. 构造需运行模块策略
    it = fisher_request.extend.find("__runing_strategy__");
    if (it == fisher_request.extend.end()) {
      if (runing_strategy == NULL) {
        mContext.error_code = 2;
        mContext.error_msg = "parse runing strategy failed NULL.";
        ERROR(LOGNAME, "parse runing strategy failed NULL.");
        goto EXIT_SEP;
      }
      string _str = string(runing_strategy);
      if (!mContext.SetRuningStrategy(_str)) {
        mContext.error_code = 2;
        mContext.error_msg = "parse runing strategy failed.";
        ERROR(LOGNAME, "parse runing strategy failed.");
        goto EXIT_SEP;
      }
    } else {
      if (!mContext.SetRuningStrategy(it->second)) {
        mContext.error_code = 2;
        mContext.error_msg = "parse runing strategy failed.";
        ERROR(LOGNAME, "parse runing strategy failed.");
        goto EXIT_SEP;
      }
    }

    // 执行前置策略
    // （初始化adinfo的feature、取redis等也在策略中做）
    ret = mpStrategyManager->Filter(fisher_request, _ad_list, _item_list, mContext, -1);
    if (!ret) 
      goto EXIT_SEP;

    // 获得context初始化耗时
    tu = mContext.timeuse;

    // 对广告队列进行特征、模型、策略
    ret = mpFeatureManager->FetchFeature(fisher_request, _ad_list, _item_list, mContext);
    if (!ret)
      goto EXIT_SEP;

    ret = mpExtractorManager->Transform(fisher_request, _ad_list, _item_list, mContext);
    if (!ret)
      goto EXIT_SEP;

    ret = mpModelManager->Predict(fisher_request, _ad_list, _item_list, mContext);
    if (!ret)
      goto EXIT_SEP;

    ret = mpStrategyManager->Filter(fisher_request, _ad_list, _item_list, mContext);
    if (!ret)
      goto EXIT_SEP;

EXIT_SEP:
    // 6. 构造输出
    rank::interface::ResponseInfo _response;
    // 输出是否需要排序（排序经过sort策略）
    if (ret) {
      NOTICE(LOGNAME, "now to set response");
      set_response(&_response, &_union, _ad_list, _item_list, mContext.rank_order);
      //record_match_info(fisher_request, mContext, _ad_list, _item_list, mContext.rank_order, &_response);
    }
    _response.set_code(mContext.error_code);
    _response.set_msg(mContext.error_msg);

    size_t size = _response.ByteSize(); 
    buffer = (char*)malloc(size);
    if (bytes_write != NULL)
      *bytes_write = size;
    _response.SerializeToArray(buffer, size);
 
    _response.Clear();
    //_response.SerializeToString(&response);
    /*for (int k=0; k<ad_size; k++) {
      response += TO_STRING(_ad_list[k].ad_id) + ":" + TO_STRING(_ad_list[k].ctr) + ";";
    }*/
  } catch (std::exception& e) {
    string err_msg = string(typeid(e).name()) + ":" + string(e.what());
    FATAL(LOGNAME, "rank fatal error, detail: %s", err_msg.c_str());
    return NULL;
  }

  return buffer;
}

void RankPluginImp::record_match_info(const RankRequest& fisher_request, RankDataContext& ptd,
                  std::vector<RankAdInfo>& ad_list,
                  std::vector<RankItemInfo>& item_list,
                  std::vector<ad_item_pair_t>& pair_list, void* __response) {
  int ad_size = ad_list.size();
  int item_size = item_list.size();
  int pair_size = pair_list.size();

  rank::interface::ResponseInfo* res_pb = (rank::interface::ResponseInfo*)__response;
  char buf[1024];

  string host = "", ip = "";
  gethostname(buf, sizeof(buf));
  host = string(buf);

  struct hostent* _host = gethostbyname(buf);
  const char* ret = inet_ntop(_host->h_addrtype, _host->h_addr_list[0], buf, sizeof(buf));
  ip = string(buf);

  for (int i=0; i<pair_size; i++) {
    ad_item_pair_t& pair = pair_list[i];
    RankAdInfo& ad = ad_list[pair.ad_idx];
    RankItemInfo& item = item_list[pair.item_idx];
    ::rank::interface::AdItemPair* pair_pb = res_pb->mutable_pairs(i);

    boost::unordered_map<int, string> map_info;

    //boost::unordered_map<int64_t, Features::Item>& features = ptd.features[pair.item_idx].features_group["begin"];
  
    {
      sprintf(buf, "pd:%.5f,%.5f,%.5f", item.ctr, item.cvr, item.ctcvr);
      map_info[0] = string(buf);
    }

    {
      sprintf(buf, "bd:%d,%.0f", pair.bid_price, pair.score);
      map_info[1] = string(buf);
    }

    {
      sprintf(buf, "ft:%d", pair.filter_strategy);
      map_info[2] = string(buf);
    }

    {
      sprintf(buf, "ip:%s", ip.c_str());
      map_info[3] = string(buf);
    }

    {
      float pid = ad.extend_sf["bid_pid"], v = 0.0f, s = 0.0f;
      if (ad.extend_sf.find("__pid_actual_value") != ad.extend_sf.end())
        v = ad.extend_sf["__pid_actual_value"];
      if (ad.extend_sf.find("__pid_speed") != ad.extend_sf.end())
        s = ad.extend_sf["__pid_speed"];
      sprintf(buf, "ox:%.0f,%.3f,%.3f", pid, v, s);
      map_info[4] = string(buf);
    }

    string match_info = "";
    for (int mm=0; mm<100; mm++) {
      boost::unordered_map<int, string>::iterator iter = map_info.find(mm);
      if (iter == map_info.end()) 
        continue;
      match_info += iter->second + ";";
    }

    ::rank::interface::kv_ss* kv = pair_pb->add_extend_ss();
    kv->set_key("rank_match_info");
    kv->set_value(match_info);

    NOTICE(LOGNAME, "rank_match_info adid=%ld, itemid=%ld, match_info=%s", ad.ad_id, item.item_id, match_info.c_str());
  }

  return;
}

}  // namespace rank
