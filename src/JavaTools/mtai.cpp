#include "mtai.h"
#include "Framework/RankPlugin.h"

static pthread_mutex_t __mutex[40] = { PTHREAD_MUTEX_INITIALIZER };
static ::rank::RankPluginImp* __instance[40] = { NULL };
static int __worker_num;

int mtai_init(const char* config_file, int worker_num) {
  if (__instance[0] != NULL) return 0;

  string config = string(config_file);
  int r = 0;
  for (int k=0; k<worker_num; k++) {
    __instance[k] = (::rank::RankPluginImp*)new rank::RankPluginImp();
    r = __instance[k]->Initialize(config);
    if (r != 0) break;
  }
  
  __worker_num = worker_num;
  return r;
}

int mtai_uninit() {
  for (int k=0; k<__worker_num; k++) {
    pthread_mutex_lock(&(__mutex[k]));
    ::rank::RankPluginImp* _ins = __instance[k];
    __instance[k] = NULL;
    sleep(3);
    _ins->UnInit();
    delete _ins;
    _ins = NULL;
    pthread_mutex_unlock(&(__mutex[k]));
  }

  rank::uninit_instance();
  return 0;
}

char* mtai_predict(const char* request, int bytes_request, int worker_idx, int* bytes_write) {
  int idx = worker_idx%__worker_num;

  //{"pre_strategy":["flowsplit","discard","discard_class","optimized_reserved_price","recall","reset","creative_opt","filter"],"feature":["hierarchy_smooth_ctr","gender_plat_cust_feed_ctr","cust60_smooth_ctr","feedid_cate_info","basic_adausr_info","adfea","online_feature"],"extractor":["xfea_1","xfea_2"],"model":["mainbody","ocpx_ctr_realimp","ocpx_cvr_test","cpl_ctr_hour","cpl_cvr_hour","ocpm_convert","addfea_guide_ctr_ftrl","addfea_social_ctr_ftrl","addfea_read_ctr_ftrl","fusion-ftrl-paramter","ctr_ftrl_iv","xfea_igctr_ftrl"],"post_strategy":["calibratecpe","optimizer2","auto_bid","ocpmbid","ocpx_realimp","ocpx_cost_optimize","precedure","cpl","cpl_cost_optimize","rankscore","search","realtime_optimizer","vcgcost","optimized_cost","reward","discard","discard_class","optimized_reserved_price","recall","custguaranteed","deliver","repeatcust","garanteed","real_read","pred_ctx","flowmerge"]}


  string runing_strategy = "{\"rank_module_list\":{\"value\":{\"pre_strategy\":[\"reset\"],\"feature\":[\"request_feature\"],\"extractor\":[\"xfea1\"],\"model\":[\"gds_ctcvr1\"],\"post_strategy\":[\"calibrate\",\"ocpx\",\"rankscore\",\"sort\"]}}}";

  //string runing_strategy = "{\"rank_module_list\":{\"value\":{\"pre_strategy\":[\"reset\"],\"feature\":[\"request_feature\"],\"extractor\":[\"xfea1\"],\"model\":[\"\"],\"post_strategy\":[\"calibrate\",\"ocpx\",\"rankscore\",\"sort\"]}}}";

  char* res = NULL;
  pthread_mutex_lock(&(__mutex[idx]));
  if (__instance[idx] == NULL) {
    *bytes_write = 0;
  } else {
    *bytes_write = 0;
    res = __instance[idx]->Predict(request, bytes_request, idx, runing_strategy.c_str(), bytes_write);
  }
  pthread_mutex_unlock(&(__mutex[idx]));
 
  return res; //strndup(res.c_str(), res.size()); 
}

/*char* mtai_ocpx(const char* request, int worker_idx) {
  int idx = worker_idx%__worker_num;
  return NULL;
}*/

void mtai_free(char* ptr) {
  if (ptr != NULL)
    free(ptr);
}

int mtai_test() {
  return 0;
}

char* mtai_test_pb(const char* request, int bytes_request, int worker_idx, int* bytes_write) {
  int idx = worker_idx%__worker_num;
  pthread_mutex_lock(&(__mutex[idx]));
  char* res = __instance[idx]->Test(request, bytes_request, bytes_write);
  pthread_mutex_unlock(&(__mutex[idx]));
  return res;
}

int mtai_test_broken() {
  std::string ss = "1232";
  printf("broken now %s\n", ss.c_str());
  return 0;
}
int mtai_test_oom() {
  std::vector<int> v;
  v.push_back(1); 
  v.push_back(2);
  int a = v[500];
  return a;
}
int mtai_test_malloc() {
  char* v = (char*)malloc(1024*1024*1024);
  return 0;
}

