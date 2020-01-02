#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Framework/ModelManager.h"
#include "Interfaces/Service.h"
#include "Interfaces/ObjectFactory.h"
#include "Interfaces/ConfigurationSettings.h"
#include "Interfaces/Util.h"
#include "Interfaces/Logger.h"
#include "Framework/graph_parse.h"

#define faraday_model_mask

namespace rank {

ModelManager::ModelManager() {}
ModelManager::~ModelManager() { clear(); }

bool ModelManager::Initialize(
    const std::string& alias,
    const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  Service<ConfigurationSettings> pSettings;
  Service<ObjectFactory> pFactory;

  mAlias = alias;
  mModelPath = pSettings->getSetting(alias + "/model_dir");
  INFO(LOGNAME, "model path:%s", mModelPath.c_str());

  mGraphConfigFile = pSettings->getSetting(alias + "/graph");
  INFO(LOGNAME, "graph config path:%s", mGraphConfigFile.c_str());

  if (!construct_model(mGraphConfigFile, mModelPath, mpModels, resource_manager, conditions)) {
    ERROR(LOGNAME, "load graph model failed, config path:%s", mGraphConfigFile.c_str());
    return false;
  }
  int len = mpModels.size();

  mModelNames = "";
  for (int i = 0; i < len; i++) {
    ModelBase* obj = mpModels[i];
    mModelNames += obj->getArgument().name + ",";
    mModelNameTops.insert(make_pair(obj->getArgument().top, obj->getArgument().name));
    mModelNamesSet.insert(obj->getArgument().name);
  }
  INFO(LOGNAME, "model list: %s", mModelNames.c_str());

  return true;
}

static bool predict_iterator(
    std::vector<ModelBase*>& models, string model_name, std::vector<bool>& mask,
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd, const string& model_names,
    std::multimap<string, string>& top_names,
    std::set<std::string>& needed_models) {
  // 1.根据模型名称找到相应索引
  // DEBUG(LOGNAME, "processing model name: %s", model_name.c_str());
  int model_index = -1;
  for (int i = 0; i < models.size(); i++) {
    // DEBUG(LOGNAME, "seeking model name: %s",
    // models[i]->getArgument().name.c_str());
    if (models[i]->getArgument().name == model_name) {
      model_index = i;
      break;
    }
  }
  if (model_index == -1) return false;
  if (mask[model_index]) return true;

  // 2.根据当前模型依赖的关系依次执行前面的模型Transform函数
  /*
    存在一个隐患，不允许2个模型top一致
  */
  const model_arg_t& model_arg = models[model_index]->getArgument();
  int bottom_num = model_arg.bottom.size();
  bool flag = true;
  for (int k = 0; (k < bottom_num) && (model_arg.bottom[k] != "begin"); k++) {
    // 如果输入为begin则跳过
    if (model_arg.bottom[k] == "begin") continue;
    // 如果输入在实验配置的模型列表中没有找到，则跳过
    bool bottom_exist = false;
    std::string bottom_model;
    typedef std::multimap<std::string, std::string> mmap_ss_T;
    std::pair<mmap_ss_T::iterator, mmap_ss_T::iterator> range =
        top_names.equal_range(model_arg.bottom[k]);
    for (mmap_ss_T::iterator iter = range.first; iter != range.second; ++iter) {
      if (needed_models.find(iter->second) != needed_models.end()) {
        bottom_exist = true;
        bottom_model.assign(iter->second);
        break;
      }
    }
    if (!bottom_exist) {
      //模型最终依赖特征group，这里终结处理
      continue;
    }

    DEBUG(LOGNAME, "dfs for model: %s->%s of %d", bottom_model.c_str(),
          model_arg.bottom[k].c_str(), models.size());
    flag &= predict_iterator(models, bottom_model, mask, fisher_request,
                             ad_list_for_rank, item_list_for_rank, ptd,
                             model_names, top_names, needed_models);
    if (!flag) {
      FATAL(LOGNAME, "predict_iterator error at model: %s",
            model_arg.bottom[k].c_str());
      break;
    }
    DEBUG(LOGNAME, "dfs for model: %s->%s of %d over", bottom_model.c_str(),
          model_arg.bottom[k].c_str(), models.size());
  }

  // 3.如果当前模型为最后一个，则进行ctr预测
  if (!flag) return flag;
  if (mask[model_index]) return true;

  DEBUG(LOGNAME, "execute model name: %s", model_arg.name.c_str());
  if (model_arg.role == "predictor") {
    flag &= models[model_index]->Predict(fisher_request, ad_list_for_rank, item_list_for_rank, ptd);
    if (!flag) ERROR(LOGNAME, "predict error at end");
  } else if (model_arg.role == "transformer") {
    flag &= models[model_index]->Transform(fisher_request, ad_list_for_rank, item_list_for_rank, ptd);
    if (!flag) ERROR(LOGNAME, "transform error at end");
  }
  mask[model_index] = true;
  DEBUG(LOGNAME, "execute model name: %s over", model_arg.name.c_str());

  return flag;
}

bool ModelManager::Predict(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  DEBUG(LOGNAME, "predict ads begin");

  std::vector<ModelBase*>& models = mpModels;
  struct timespec start, end;
  long _ss = 0;
  
  string end_model = "", model_name = "";
  vector<bool> mask(models.size(), false);
  int num_need_to_run = models.size();

#ifdef faraday_model_mask
  std::set<std::string> needed_models;
  std::vector<ModelBase*> expr_models;  
  const Json::Value& rank_model = ptd.strategy_json["rank_module_list"]["value"]["model"];
  num_need_to_run = rank_model.size();
  // 先拿到实验配置和所有配置模型的交集,后面用交集进行约束
  // 这里认为模型重名问题由人来保证或者上线流程来保证，我们不把这种检查放到这个需要效率的地方
  for (Json::ArrayIndex j = 0; j < rank_model.size(); ++j) {
    for (int i = 0; i < models.size(); i++) {
      std::string conf_model_name(rank_model[j].asString());
      if (conf_model_name.compare(models[i]->getArgument().name) == 0) {
        needed_models.insert(conf_model_name);
        expr_models.push_back(models[i]);
        DEBUG(LOGNAME, "needed models %s", conf_model_name.c_str());
      }
    }
  }
#else
  std::set<std::string>& needed_models = mModelNamesSet;
  std::vector<ModelBase*>& expr_models = models;
#endif

  if (num_need_to_run < 1)
    return true;
  if (models.size() < 1) {
    ptd.error_code = 402;
    ptd.error_msg = "models manager holds zero models";
    return false;
  }

  bool run = false;
  for (int i = 0; i < expr_models.size(); i++) {
#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif
    run = true;

    // 获取所有输出为ctr（即end）的模型，从后向前依次查找依赖的模型，并执行
    end_model = expr_models[i]->getArgument().top;
    model_name = expr_models[i]->getArgument().name;
    //int64_t begin_time = base::TimeUtil::CurrentTimeInMSec();
    if (end_model == "end" || end_model == "cvr" || end_model == "ctr" || end_model == "otr" || end_model == "ctcvr") {
      DEBUG(LOGNAME, "dfs executing for model name: %s->%s of %d", model_name.c_str(), end_model.c_str(), expr_models.size());
      bool _run_ok = predict_iterator(models, model_name, mask, fisher_request, ad_list_for_rank,
          item_list_for_rank, ptd, mModelNames, mModelNameTops,
          needed_models);

      if (!_run_ok) {
        ERROR(LOGNAME, "model %s run failed", model_name.c_str());
        ptd.error_code = 400;
        ptd.error_msg = "model " + model_name + " run failed";
        return false;
      }

      DEBUG(LOGNAME, "dfs executing for model name: %s->%s of %d over", model_name.c_str(), end_model.c_str(), expr_models.size());
    }

#ifdef PROF
    clock_gettime(CLOCK_MONOTONIC, &end);
    long sum = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    _ss += sum;
    FATAL(LOGNAME, "model time: %s: %ld us", model_name.c_str(), sum);
#endif
    //int64_t end_time = base::TimeUtil::CurrentTimeInMSec();
    //ptd.RecordStatsInfo(kModel, model_name, end_time - begin_time, 0);
  }
  if (!run) {
    ptd.error_code = 401;
    ptd.error_msg = "no models runs";
    ERROR(LOGNAME, "no models runs");
    return false;
  }
#ifdef PROF
  FATAL(LOGNAME, "total model time: %ld us", _ss);
#endif

  DEBUG(LOGNAME, "predict ads end");
  return true;
}

void ModelManager::clear() {
  Service<ObjectFactory> pFactory;
  int len = mpModels.size();

  for (int i = 0; i < len; i++) {
    ModelBase* obj = mpModels[i];
    pFactory->destroyObject(obj, obj->getRegistName());
  }
  std::vector<ModelBase*>().swap(mpModels);  // mpModels.clear();
  mModelNames = "";
  std::set<std::string>().swap(mModelNamesSet);
  std::multimap<string, string>().swap(mModelNameTops);
}
}  // namespace rank
