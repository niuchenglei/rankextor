#include <iostream>
#include "omp.h"
#include "Extractor/XFeaAdapter.h"

using namespace boost;

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
#include <stdlib.h>
#endif

#define XFEA_COPY_VALUE false

namespace rank {

REGISTE_CLASS("xfea", XFeaAdapter)

XFeaAdapter::XFeaAdapter() {
  for (int i=0; i<1; i++) 
    extractor[i] = new xfea::bisheng::Extractor(); 
}

XFeaAdapter::~XFeaAdapter() {
  for (int i=0; i<1; i++) {
    extractor[i]->finalize();
    delete extractor[i];
    extractor[i] = NULL;
  }
}

bool XFeaAdapter::Initialize(
    extractor_arg_t& arg,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  FeatureExtractorBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mFeaturePath = pSetting->getSetting(alias + "/feature_dir");
  mModelPath = pSetting->getSetting(alias + "/model_dir");
  mArg = arg;

  mConfig = arg.config;
  if (mConfig[0] != '/') mConfig = mModelPath + "/" + mConfig;

  for (int i=0; i<1; i++) {
    if (extractor[i]->init(mConfig) != RC_SUCCESS) {
      const char* msg = xfea::bisheng::xfea_error_msg();
      FATAL(LOGNAME, "init bisheng error(%s), detail:%s", mConfig.c_str(), msg);
      return false;
    }
  }
  // NOTICE(LOGNAME,"mConfig file %s", mConfig.c_str());
  REGIST_HANDLER(mConfig);

  name_hash_map.clear();
  const std::map<std::string, int>& schema = extractor[0]->get_input_schema_key_index_map();
  std::map<std::string, int>::const_iterator iter = schema.begin();
  for (; iter != schema.end(); iter++) {
    int64_t key = __xhash__(iter->first.c_str());
    if (name_hash_map.find(key) != name_hash_map.end()) {
      FATAL(LOGNAME, "bisheng schema error(%s)!", iter->first.c_str());
      return false;
    }
    name_hash_map.insert(make_pair(key, iter->second));
  }

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
  std::string xfea_debug_output_path =
      mFeaturePath + "/xfea_debug.txt";
  _xfea_debug_output_ofs.open(xfea_debug_output_path.c_str(),
                              std::ios_base::out);
#endif

  return Update();
}

bool XFeaAdapter::Transform(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  std::vector<RankItemInfo>& item_list = item_list_for_rank;
  uint size = item_list.size();

  DEBUG(LOGNAME, "begin xfea transform");

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
  bool is_xfea_output = false;
  float rnd = ((double)rand()) / RAND_MAX;
  if (rnd > 0.9) {
    is_xfea_output = true;
  }
#endif

  vector<string> bottoms = mArg.bottom;
  int btm_size = bottoms.size(), index, pid;

  int _cc = 0;
  for (int k = 0; k < btm_size; k++) {
    string bottom_name = bottoms[k];

    // 查找用户侧共享特征
    if (ptd.common_features.features_group.find(bottom_name) != ptd.common_features.features_group.end()) {
      boost::unordered_map<int64_t, Features::Item>& _map2 = ptd.common_features.features_group[bottom_name];
      boost::unordered_map<int64_t, Features::Item>::iterator iter2 = _map2.begin();
      for (; iter2 != _map2.end(); iter2++) {
        map<int64_t, int>::iterator _it = name_hash_map.find(iter2->first);
        if (_it == name_hash_map.end()) continue;

        //NOTICE(LOGNAME, "%d'th ind=%d, val=%s", _cc, _it->second, iter2->second.svalue); 
        _cc += 1;
        ReturnCode ret = extractor[0]->add_field_value(iter2->second.svalue, _it->second);
        if (ret != RC_SUCCESS)
          ERROR(LOGNAME, "extractor->add_field_value error");
      }
    }
  }
  DEBUG(LOGNAME, "add %d common fields of %d bottoms into xfea", _cc, btm_size);

// 目前extractor效率不高，开启omp效率依然不高
//#ifdef OMP
//#pragma omp parallel for num_threads(10) shared(ptd, adlist, fisher_request, extractor) private(index, pid)
//#endif
  for (index=0; index<size; index++) {
    if (item_list[index].filter_strategy != 10000) continue;
    //if (!CheckCondition(adlist[index], ptd)) continue;

    pid = 0; //omp_get_thread_num();

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
    if (is_xfea_output && 0 == ptd.worker_idx) {
      _xfea_debug_output_ofs << fisher_request.pvid << "\t";
    }
#endif

    //extractor[pid]->one_round_reset();
    //extractor[pid]->just_result_reset();

    ReturnCode ret;
    int cc = 0;
    for (int k = 0; k < btm_size; k++) {
      string bottom_name = bottoms[k];
      boost::unordered_map<int64_t, Features::Item>& _map = ptd.features[index].features_group[bottom_name];
      boost::unordered_map<int64_t, Features::Item>::iterator iter = _map.begin();
      for (; iter != _map.end(); iter++) {
        map<int64_t, int>::iterator _it = name_hash_map.find(iter->first);
        if (_it == name_hash_map.end()) continue;

        //NOTICE(LOGNAME, "%d'th ind=%d, val=%s", cc, _it->second, iter->second.svalue); 
        cc += 1;
        ret = extractor[pid]->add_field_value(iter->second.svalue, _it->second, XFEA_COPY_VALUE);
        if (ret != RC_SUCCESS)
          ERROR(LOGNAME, "extractor->add_field_value error");
      }

      /*// 查找用户侧共享特征
      if (ptd.common_features.features_group.find(bottom_name) != ptd.common_features.features_group.end()) {
        boost::unordered_map<int64_t, Features::Item>& _map2 = ptd.common_features.features_group[bottom_name];
        boost::unordered_map<int64_t, Features::Item>::iterator iter2 = _map2.begin();
        for (; iter2 != _map2.end(); iter2++) {
          map<int64_t, int>::iterator _it = name_hash_map.find(iter2->first);
          if (_it == name_hash_map.end()) continue;

          DEBUG(LOGNAME, "extract_features_from_record di %d values is %s index is %d", cc, iter2->second.svalue, _it->second);
          cc += 1;
          ret = extractor[pid]->add_field_value(iter2->second.svalue, _it->second, XFEA_COPY_VALUE);
          if (ret != RC_SUCCESS)
            ERROR(LOGNAME, "extractor->add_field_value error");
        }
      }*/
    }
    DEBUG(LOGNAME, "add %d fields of %d bottoms into xfea", cc, btm_size);

    ret = extractor[pid]->extract_features_from_record(XFEA_COPY_VALUE);
    if (ret != RC_SUCCESS) {
      FATAL(LOGNAME, "extractor->extract_features_from_record error");
      // continue;
    }

    const FeaResultSet& fea_result_set = extractor[pid]->get_fea_result_set();

    boost::unordered_map<int64_t, Features::Item>& _map = ptd.features[index].features_group[mArg.top];
    Features::Item item;
    item.type = 'n';
    item.value = 1.0;

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
    if (is_xfea_output && 0 == ptd.worker_idx) {
      _xfea_debug_output_ofs << fisher_request.pvid << "\t";
    }
#endif

    uint32_t fea_result_num = fea_result_set.size();
    for (uint32_t i = 0; i < fea_result_num; ++i) {
      const fea_result_t* fea_result = fea_result_set.get_fea_result(i);
      if (NULL == fea_result) continue;
      if (!fea_result->is_valid) continue;

      _map[fea_result->final_fea_sign] = item;

      DEBUG(LOGNAME, "xfea slot:%ld, text:%s, origin_sign:%ld, sign:%ld", fea_result->fea_slot, fea_result->fea_text, fea_result->origin_fea_sign, fea_result->final_fea_sign);
#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
      if (is_xfea_output && 0 == ptd.worker_idx) {
        _xfea_debug_output_ofs << fea_result->final_fea_sign << ":"
                               << fea_result->origin_fea_sign << ":"
                               << fea_result->fea_slot << ":"
                               << fea_result->fea_text << "\t";
      }
#endif
    }
#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
    if (is_xfea_output && 0 == ptd.worker_idx) {
      _xfea_debug_output_ofs << std::endl;
    }
#endif
    DEBUG(LOGNAME, "xfea generate %u feature sign", fea_result_num);
  }
  DEBUG(LOGNAME, "end xfea transform");
  return true;
}

bool XFeaAdapter::Update() {
  /*
  extractor dynamic update not support now cause it's a array now
  xfea::bisheng::Extractor* _ext = new xfea::bisheng::Extractor();
  if (_ext->init(mConfig) != RC_SUCCESS) {
    FATAL(LOGNAME, "init bisheng error(%s)", mConfig.c_str());
    return false;
  }
  std::map<int64_t, int> _name_hash_map;
  const std::map<std::string, int>& schema = extractor->get_input_schema_key_index_map();
  std::map<std::string, int>::const_iterator iter = schema.begin();
  for (; iter != schema.end(); iter++) {
    int64_t key = __xhash__(iter->first.c_str());
    if (_name_hash_map.find(key) != _name_hash_map.end()) {
      FATAL(LOGNAME, "bisheng schema error(%s)!", iter->first.c_str());
      return false;
    }
    _name_hash_map.insert(make_pair(key, iter->second));
  }

#ifdef __DEBUG_XFEA_FEA_OUTPUT_ONLINE_
  std::string xfea_debug_output_path =
      mFeaturePath + "/xfea_debug_zl_xxxxx.123.txt";
  _xfea_debug_output_ofs.open(xfea_debug_output_path.c_str(),
                              std::ios_base::out);
#endif

  xfea::bisheng::Extractor* _old = extractor;

  extractor = _ext;
  name_hash_map = _name_hash_map;

  _old->finalize();
  delete _old;
  _old = NULL;
*/
  return true;
}

}  // namespace rank
