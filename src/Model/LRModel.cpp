#include "omp.h"
#include "Model/LRModel.h"

using namespace boost;

namespace rank {
#define SCALE 10000
#define CONST_FEATURE_CONFIG "CONST"

REGISTE_CLASS("lr", LRModel)

LRModel::LRModel() {}

LRModel::~LRModel() {
  Service<ResourceManager> resource_manager;
  resource_manager->UnRegisterResource(mResourceKey);
}

bool LRModel::Initialize(
    model_arg_t& arg, const std::map<std::string, ConditionBase*>& conditions,
    ResourceManager* resource_manager) {
  ModelBase::Initialize(arg, conditions, resource_manager);

  Service<ConfigurationSettings> pSetting;
  string alias = pSetting->getSetting("main/exp_id");
  mModelDir = pSetting->getSetting(alias + "/model_dir");
  mArg = arg;

  DEBUG(LOGNAME, "model_file: %s, model_feature_map_file: %s",
        mArg.model_file.c_str(), mArg.map_file.c_str());

  mResourceKey = mModelDir + "/" + mArg.model_file;
  string prefix = mResourceKey + ".touch";
  mPluginName = pSetting->getSetting("main/plugin_name");

  DEBUG(LOGNAME, "mResourceKey: %s, prefix: %s", mResourceKey.c_str(),
        prefix.c_str());

  REGISTE_RESOURCE_TOUCH(mResourceKey, prefix, lr_coeff_t, resource_manager);
  srand((unsigned)time(NULL));

  return Update();
}

bool LRModel::Transform(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  ERROR(LOGNAME, "lr transform not implemented");
  return false;
}

bool LRModel::Predict(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  // 更新获取资源
  Resource* res = NULL;
  if (ptd.resource_manager->FetchResource(mResourceKey, &res) == 0) {
    mpCoeff = static_cast<lr_coeff_t*>(res);
  } else {
    FATAL(LOGNAME, "resource_manager fetch %s failed", mResourceKey.c_str());
    return false;
  }

  DEBUG(LOGNAME, "lr predict begin");

  lr_coeff_t& model = *mpCoeff;
  std::vector<RankItemInfo>& item_list = item_list_for_rank;
  size_t ad_size = item_list.size();

  int k, bottom_num;
  float sum_weight;
  unsigned int pid, thread_offset;

#ifdef OMP
#pragma omp parallel for num_threads(NUM_THREAD) shared(mArg, ptd, item_list) private(k, sum_weight, pid, thread_offset)
#endif
  for (k = 0; k < ad_size; k++) {
    // pid = omp_get_thread_num();

    sum_weight = model.mModelWeights[0];  // 根据是否存在常量字段，决定从何处读取常量数值
    bottom_num = mArg.bottom.size();
    string bottom_name;

    // 执行condition
    //if (!CheckCondition(item_list[k], ptd)) continue;
    if (item_list[k].filter_strategy != 10000) continue;

    // DEBUG(LOGNAME, "%s predict %d, %x", mArg.name.c_str(), k,
    // mpCondition); //->getRegistName().c_str());
    // 遍历当前模型依赖的所有输入（即bottom）
    vector<int64_t> features;
    for (int bottom_id = 0; bottom_id < bottom_num; bottom_id++) {
      bottom_name = mArg.bottom[bottom_id];
      boost::unordered_map<int64_t, Features::Item>& _map = ptd.features[k].features_group[bottom_name];

      // 遍历当前输入中的所有特征
      for (boost::unordered_map<int64_t, Features::Item>::iterator iter = _map.begin(); iter != _map.end(); iter++) {
        if (iter->first >= 0 && iter->second.type == 'n' && iter->second.value > 0.0f)
          features.push_back(iter->first);
      }
    }

    float pred = mArg.weight * wTx(features, model);

    // CTR校正
    pred = mArg.sample_ratio / (1.0 / (pred + 0.0000001) - 1.0 + mArg.sample_ratio);
    if (mArg.top != "") {
      // 如果features中没有top特征，则添加一个
      if (ptd.features[k].features_group.find(mArg.top) == ptd.features[k].features_group.end()) {
        unordered_map<int64_t, Features::Item> _map;
        ptd.features[k].features_group[mArg.top] = _map;
      }
      unordered_map<int64_t, Features::Item>* top_map = &(ptd.features[k].features_group[mArg.top]);
      struct Features::Item item = {'n', pred};
      top_map->insert(make_pair(0, item));
      DEBUG(LOGNAME, "[%s] produce ctr [%s]=%f", mArg.model_file.c_str(), mArg.top.c_str(), pred);
    }
    if (mArg.top == "ctr" || mArg.top == "end") {
      item_list[k].ctr = pred;

      /*string feature_msg = mArg.name + ":" + boost::lexical_cast<string>(pred) + ",";
      int64_t f_dim = features.size();
      for (int64_t k = 0; k < f_dim; k++)
        feature_msg += boost::lexical_cast<string>(features[k]) + ",";
      item_list[k].extend["FEATURE"] = feature_msg.substr(0, strlen(feature_msg.c_str()) - 1);
      */
    } else if (mArg.top == "cvr") {
      item_list[k].cvr = pred;
    } else if (mArg.top == "ctcvr") {
      item_list[k].ctcvr = pred;
    }
  }

  DEBUG(LOGNAME, "lr predict end");
  return true;
}

bool LRModel::Update() { return true; }

float LRModel::wTx(const vector<int64_t>& feature, lr_coeff_t& model) {
  float sum = model.mModelWeights[0];
  size_t nn = feature.size();
  if (model.mFeatureKeys.size() == 0) {
      ERROR(LOGNAME,"lr model has no data");
      return 0.5;
  }

  // std::ostringstream os;
  // os << "0:"<<sum <<", ";
  for (int k = 0; k < nn; k++) {
    if (model.mModelWeights.find(feature[k]) != model.mModelWeights.end()) {
      sum += model.mModelWeights[feature[k]];
      // os << feature[k] << ":" << model.mModelWeights[feature[k]] << ", ";
    } else {
      if (mArg.name == "ctr_ftrl_iv" &&
          ((feature[k] > 100000000 && feature[k] <= 110000000))) {
        int rnd = (rand() % model.mFeatureKeys.size());
        sum += model.mModelWeights[model.mFeatureKeys[rnd]];
        // sum += model.mDefaultWeight;
      }
      // os << feature[k] << ":null, ";
    }
  }

  float pred_ctr = 1.0 / (1.0 + exp(-sum));
  NOTICE(LOGNAME, "lr pred %f \n", pred_ctr);
  // FATAL(LOGNAME, "msg: %s, %f", os.str().c_str(), pred_ctr);
  return pred_ctr;
}
/////////////////////////////////////////////////

int lr_coeff_t::Load(const std::string& config_file) {
  mModelFile = config_file;
  loadModel();

  return 0;
}

bool lr_coeff_t::loadModel() {
  ifstream ifs(mModelFile.c_str());
  if (!ifs) {
    ERROR(LOGNAME, "can'f find file: %s", mModelFile.c_str());
    return false;
  }

  // FATAL(LOGNAME, "now read lr model: %s", mModelFile.c_str());

  unordered_map<int64_t, float>().swap(mModelWeights);  //释放内存
  vector<int64_t>().swap(mFeatureKeys);

  string line;
  int cnt = 0;
  mDefaultWeight = 0;
  while (getline(ifs, line)) {
    vector<string> tmp;
    AdUtil::Split2(tmp, line);
    if (tmp.size() < 2) {
      tmp.clear();
      AdUtil::Split2(tmp, line, ',');
    }

    // 本地模型文件方式，默认模型大小为1000万行，防止内存爆掉
    if (cnt > 10000000) {
      FATAL(LOGNAME, "model num line > 10000000");
      break;
    }
    if (tmp.size() < 2) continue;

    float weight = 0;
    if (tmp[0] ==
        CONST_FEATURE_CONFIG) {  //第一行规定为常数特征, 譬如 "CONST^-4.331855"
      mModelWeights[0] = 0;
      if (isFloat(tmp[1], &weight)) mModelWeights[0] = weight;
      continue;
    }

    int64_t feature_id = 0;
    if ((tmp.size() == 2) && isInt64(tmp[0], &feature_id) &&
        isFloat(tmp[1], &weight)) {
      mModelWeights[feature_id] = weight;
      if (cnt < 5) {
        DEBUG(LOGNAME, "feature_id=%ld weight=%f", feature_id, weight);
      }
      if (feature_id > 100000000 && feature_id <= 110000000) {
        mFeatureKeys.push_back(feature_id);
        mDefaultWeight += weight;
      }
    } else {
      ERROR(LOGNAME, "error line=%s", line.c_str());
      return false;
    }
    cnt++;
  }
  if (mFeatureKeys.size() == 0)
    mDefaultWeight = 0;
  else {
    mDefaultWeight /= mFeatureKeys.size();
  }
  mFeatureKeys.clear();

  for (unordered_map<int64_t, float>::iterator iter = mModelWeights.begin();
       iter != mModelWeights.end(); iter++) {
    if (iter->first > 100000000 && iter->first <= 110000000) {
      if (iter->second < mDefaultWeight) mFeatureKeys.push_back(iter->first);
    }
  }

  DEBUG(LOGNAME, "mFeatureKeys weight size=%d", mFeatureKeys.size());
  ifs.close();
  if (mModelWeights.size() < 10000) {
    FATAL(LOGNAME, "feature weight size=%d, it maybe a error model",
          mModelWeights.size());
  }

  return true;
}

}  // namespace rank
