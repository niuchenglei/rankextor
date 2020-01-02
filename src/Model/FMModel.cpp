#include "omp.h"
#include "Model/FMModel.h"
using namespace boost;

namespace rank {

REGISTE_CLASS("fm", FMModel)

FMModel::FMModel() {}

FMModel::~FMModel() {
  Service<ResourceManager> resource_manager;
  resource_manager->UnRegisterResource(mResourceKey);
}

bool FMModel::Initialize(
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
  // if (mArg.map_file != "" && mArg.map_file != " ")
  //    mResourceKey += "," + mModelDir + "/" + mArg.map_file;

  // FATAL(LOGNAME, "registe resource %s, rely on %s", mResourceKey.c_str(),
  // prefix.c_str());
  REGISTE_RESOURCE_TOUCH(mResourceKey, prefix, fm_coeff_t, resource_manager);

  return Update();
}

bool FMModel::Transform(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  ERROR(LOGNAME, "fm transform not implemented");
  return false;
}

bool FMModel::Predict(
    const RankRequest& fisher_request,
    std::vector<RankAdInfo>& ad_list_for_rank,
    std::vector<RankItemInfo>& item_list_for_rank,
    RankDataContext& ptd) {
  // 更新获取资源
  Resource* res = NULL;
  if (ptd.resource_manager->FetchResource(mResourceKey, &res) == 0) {
    mpCoeff = static_cast<fm_coeff_t*>(res);
  } else {
    FATAL(LOGNAME, "resource_manager fetch %s failed", mResourceKey.c_str());
    return false;
  }

  DEBUG(LOGNAME, "%s predict begin", mArg.name.c_str());

  fm_coeff_t& model = *mpCoeff;
  std::vector<RankItemInfo>& item_list = item_list_for_rank;
  size_t ad_size = item_list.size();

  int k, bottom_num;
  float sum_weight;
  unsigned int pid, thread_offset;
  model_arg_t& _mArg = this->mArg;

  int omp_thread_num = NUM_THREAD;
  if (omp_thread_num*3 > ad_size)
    omp_thread_num = float(ad_size)/3.0;

  const size_t __MAX_FEATURE_SIZE__ = 1000;
  //uint64_t* feature_buffer = (uint64_t*)malloc(__MAX_FEATURE_SIZE__*sizeof(uint64_t)); // 默认一个
//#ifdef OMP
//#pragma omp parallel for num_threads(omp_thread_num) shared(_mArg, ptd, item_list) private(k, sum_weight, pid, thread_offset)
//#endif
  for (k = 0; k < ad_size; k++) {
    //pid = omp_get_thread_num();
    bottom_num = _mArg.bottom.size();
    string bottom_name;
    // 执行condition
    //if (!CheckCondition(item_list[k], ptd)) continue;
    if (item_list[k].filter_strategy != 10000) continue;

    // 遍历当前模型依赖的所有输入（即bottom）
    float hie_ctr = 0, v1, v2;
    uint64_t _cnt_features = 0;
    vector<uint64_t> features;
    // std::ostringstream os;
    for (int bottom_id = 0; bottom_id < bottom_num; bottom_id++) {
      bottom_name = _mArg.bottom[bottom_id];
      boost::unordered_map<int64_t, Features::Item>& _map = ptd.features[k].features_group[bottom_name];
      
      // 遍历当前输入中的所有特征
      for (boost::unordered_map<int64_t, Features::Item>::iterator iter = _map.begin();
           iter != _map.end(); iter++) {
        if (iter->first >= 0 && iter->second.type == 'n' && iter->second.value > 0.0f) {
          features.push_back(iter->first);
          //feature_buffer[_cnt_features] = iter->first;
          _cnt_features++;

          if (_cnt_features >= __MAX_FEATURE_SIZE__) 
            break;
          // os << iter->first<<",";
        }
      }

      if (_cnt_features >= __MAX_FEATURE_SIZE__)
        break;
    }
    // DEBUG(LOGNAME, "features: %s", os.str().c_str());
    /* 测试模型正确性用数据
    0 17:1 76:1 80:1 85:1 168:1 445:1 457:1 472:1 3333:1 3824:1 4528:1 5228:1
    5890:1 6561:1 7305:1 568:1 1146:1
    0 27:1 75:1 79:1 81:1 306:1 446:1 469:1 473:1 3253:1 3824:1 4528:1 5228:1
    5890:1 6561:1 7305:1 564:1 1340:1
    0 20:1 76:1 79:1 81:1 158:1 445:1 464:1 472:1 3253:1 3824:1 4528:1 5228:1
    5890:1 6561:1 7305:1 568:1 1316:1
    0 15:1 76:1 80:1 86:1 140:1 446:1 470:1 473:1 3030:1 3912:1 4609:1 5309:1
    5976:1 6658:1 7386:1 479:1 2394:1
    0	0.00808944
    0	0.00758244
    0	0.0096316
    0	0.00243989 
    int ff[] = {46853264,46853278,46853280,46853281,46853282,90888670,90888671,90888673,90888674,90888675,116168365,128564080,139156521,167242964,183771685,183771778,283480592,298697399,426139184,439445625,462229941,462229942,462229943,507434097,627928445,690413049,702098030,732937509,746498695,786311723,786311727,786311728,786311729,833383564,833383576,833383581,833383606,833383607,833383623,833383624,833383625,833383626,833383627,833383628,833383629,833383630,833383631,833383632,833383633,833383638,833383639,833383640,833383641,833383642,833383643,833383645,833383646,833383647,833383648,833383649,833383650,833383651,833383653,833383654,833383655,833383656,926438270,926438278,926438279,954396263,999729098};
    features.clear();
    for(int k=0; k<71; k++) features.push_back(ff[k]);

    DEBUG(LOGNAME, "tmp feature used: %d, %d", features[0], features[1]);
*/
    string msg;
    float pred = _mArg.weight * wTx(features, _cnt_features, model); //, &v1, &v2, msg);
    // CTR校正
    pred = _mArg.sample_ratio / (1.0 / (pred + 0.0000001) - 1.0 + _mArg.sample_ratio);
    if (_mArg.top != "") {
      // 如果features中没有top特征，则添加一个
      if (ptd.features[k].features_group.find(_mArg.top) == ptd.features[k].features_group.end()) {
        unordered_map<int64_t, Features::Item> _map;
        ptd.features[k].features_group[_mArg.top] = _map;
      }
      unordered_map<int64_t, Features::Item>* top_map = &(ptd.features[k].features_group[_mArg.top]);
      struct Features::Item item = {'n', pred};
      top_map->insert(make_pair(0, item));  //"predict_value", item));
    }
    if (_mArg.top == "ctr" || _mArg.top == "end") {
      item_list[k].ctr = pred;

      /*string feature_msg = _mArg.name + ":" + boost::lexical_cast<string>(pred) + ",";
      int64_t f_dim = features.size();
      for (int64_t k = 0; k < f_dim; k++)
        feature_msg += boost::lexical_cast<string>(features[k]) + ",";
      item_list[k].extend["FEATURE"] = feature_msg.substr(0, strlen(feature_msg.c_str()) - 1);
      */
    } else if (_mArg.top == "cvr") {
      item_list[k].cvr = pred;
    } else if (_mArg.top == "ctcvr") {
      item_list[k].ctcvr = pred;
    }
    /*printf("fm debug\n");
    float aaa = 0.0f;
    printf("aaa=1.0 %d\n", 1);
    printf("aaa=1.0 %f\n", aaa);
    printf("top=%s\n", _mArg.top.c_str());
    printf("%s predict id=%ld\n", _mArg.name.c_str(), item_list[k].item_id);
    printf("%s predict id=%ld, ctr=%f, top=%s\n", _mArg.name.c_str(), item_list[k].item_id, pred, _mArg.top.c_str());
    printf("fea_num=%d, msg=%s\n", features.size(), msg.c_str());
    */
  
    NOTICE(LOGNAME, "%s predict id=%ld, ctr=%f, top=%s, w=%.4f, v=%.4f, fea_num=%d, msg=%s, model_size=%d", _mArg.name.c_str(), item_list[k].item_id, pred, _mArg.top.c_str(), v1, v2, _cnt_features, msg.c_str(), model.mModelWeights.size());
  }

  //free(feature_buffer);
  DEBUG(LOGNAME, "%s predict end", mArg.name.c_str());
  return true;
}
float FMModel::wTx(const vector<uint64_t>& feature, uint64_t feature_size, fm_coeff_t& model) {  //, float* v1, float* v2, string& msg) {
//float FMModel::wTx(const uint64_t* feature, uint64_t feature_size, fm_coeff_t& model, float* v1, float* v2, string& msg) {
  float sum = model.mConstWeight;
  int num_weight = model.mModelWeights.size();
  int num_feature = model.mFeatureNum;
  int factor = model.mFactor;

  feature_size = min(feature_size, feature.size());

  //msg = "";
  //std::ostringstream os;
  for (uint64_t i = 0; i < feature_size; i++) {
    if (feature[i] >= num_feature) continue;
    // os << feature[i]<<":"<<model.mModelWeights[feature[i]]<<", ";
    if (model.mModelWeights.find(feature[i]) != model.mModelWeights.end())
      sum += model.mModelWeights[feature[i]];
  }
  //msg = "";  // os.str();
  //*v1 = sum;
  // pairwisie interation features 0.5 * sum((X*V).^2 - (X.*X)*(V.*V), 2)
  float sum1, sum2;
  for (int i = 0; i < factor; i++) {
    sum1 = 0.f;
    sum2 = 0.f;
    for (int j = 0; j < feature_size; ++j) {
      uint64_t n = num_feature + feature[j] * factor + i;
      /*if (n >= num_weight - 1) continue;*/
      float XV = 0;
      if (model.mModelWeights.find(n) != model.mModelWeights.end())
        XV = model.mModelWeights[n];
      sum1 += XV;
      sum2 += XV * XV;
    }
    sum += 0.5 * (sum1 * sum1 - sum2);
  }
  //*v2 = sum - *v1;
  float pred_ctr = sum;
  pred_ctr = 1.0 / (1.0 + exp(-sum));

  return pred_ctr;
}
bool FMModel::Update() { return true; }

/////////////////////////////////////////////////////////

int fm_coeff_t::Load(const std::string& config_file) {
  /*/vector<string> tmp;
  AdUtil::Split3(tmp, config_file, ',');
  if (tmp.size() != 2) {
      ERROR(LOGNAME, "fm_coeff_t load error: %s", config_file.c_str());
      return 1;
  }*/
  // FATAL(LOGNAME, "load fm model: %s", config_file.c_str());
  mModelFile = config_file;  // tmp[0];
  loadModel();

  return 0;
}

bool fm_coeff_t::loadModel() {
  mHasConstWeightFlag = false;

  ifstream ifs(mModelFile.c_str());
  if (!ifs) {
    ERROR(LOGNAME, "can't find file: %s", mModelFile.c_str());
    return false;
  }

  mModelWeights.clear();
  //boost::unordered_map<uint64_t, float>().swap(mModelWeights);  //释放内存

  string line;
  int cnt = 0;
  char delim = 'a';
  int vv = 0;
  while (getline(ifs, line)) {
    vector<string> tmp;

    if (delim == 'a') {
      delim = ';';
      AdUtil::Split2(tmp, line, ';');
      if (tmp.size() < 2) {
        tmp.clear();
        delim = '\t';
        AdUtil::Split2(tmp, line, '\t');
        if (tmp.size() < 2) {
          tmp.clear();
          delim = ',';
          AdUtil::Split2(tmp, line, ',');
          if (tmp.size() < 2) {
            tmp.clear();
            delim = ' ';
            AdUtil::Split2(tmp, line, ' ');
            if (tmp.size() < 2) continue;
          }
        }
      }
    }
    tmp.clear();
    AdUtil::Split2(tmp, line, delim);

    //bool flag = std::isdigit(tmp[0][0]);  //valid_number(tmp[0]);
    if (vv < 3) {
      if (tmp[0] == "factor" || tmp[0] == "Factor") {
        mFactor = atoi(tmp[1].c_str());
        vv += 1;
        continue;
      }
      if (tmp[0] == "feature_num" || tmp[0] == "dim") {
        mFeatureNum = atoi(tmp[1].c_str());
        vv += 1;
        continue;
      }

      if (tmp[0] == "CONST" || tmp[0] == "const") {  //第一行规定为常数特征, 譬如 "CONST^-4.331855"
        mConstWeight = boost::lexical_cast<float>(tmp[1]);
        mHasConstWeightFlag = true;
        vv += 1;
        continue;
      }
    }

    if (tmp.size() < mFactor + 2) {
      ERROR(LOGNAME, "error line=%s", line.c_str());
      continue; 
    }

    uint64_t feature_id = 0;
    float weight = 0;
    feature_id = STRING_TO_UINT64(tmp[0]);
    weight = STRING_TO_FLOAT(tmp[1]);
    /*if (!flag) {
      ERROR(LOGNAME, "error line=%s", line.c_str());
      return false;
    }*/
    mModelWeights[feature_id] = weight;
    for (uint64_t m = 0; m < mFactor; m++) {
      weight = STRING_TO_FLOAT(tmp[2+m]);
      //isFloat(tmp[2 + m], &weight);
      mModelWeights[mFeatureNum + feature_id * mFactor + m] = weight;
    }
    cnt++;
  }
  ifs.close();
  DEBUG(LOGNAME, "feature weight size=%d", mModelWeights.size());

  return true;
}

}  // namespace rank
