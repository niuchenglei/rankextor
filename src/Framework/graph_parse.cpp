#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "Framework/graph_parse.h"
#include "Framework/graph.pb.h"

using namespace rank::graph;

using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::Message;

namespace rank {
/*
void WriteProtoToTextFile(const Message& proto, const char* filename) {
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  FileOutputStream* output = new FileOutputStream(fd);
  CHECK(google::protobuf::TextFormat::Print(proto, output));
  delete output;
  close(fd);
}
*/

bool construct_condition(const std::string& filename,
                         std::vector<ConditionBase*>& conditions,
                         ResourceManager* resource_manager) {
  Service<ObjectFactory> pFactory;
  ModelGroup group;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return false;
  FileInputStream* input = new FileInputStream(fd);
  bool success = google::protobuf::TextFormat::Parse(input, &group);
  // google::protobuf::TextFormat::PrintToString(vm, &tmp);
  delete input;
  close(fd);

  int num = group.condition_size();
  for (int i = 0; i < num; i++) {
    Condition* condition_proto = group.mutable_condition(i);
    string condition_name = condition_proto->name();

    if (condition_name.empty() || condition_name == "null") continue;

    ConditionBase* obj =
        static_cast<ConditionBase*>(pFactory->createObject(condition_name));
    if (obj == NULL) {
      ERROR(LOGNAME, "unregistered name %s", condition_name.c_str());
      return false;
    }
    if (!obj->Initialize()) {
      ERROR(LOGNAME, "instance %s Initialize failed", condition_name.c_str());
      return false;
    }
    INFO(LOGNAME, "condition instance %s ok", condition_name.c_str());

    conditions.push_back(obj);
  }

  return true;
}

bool construct_feature(
    const std::string& filename, const std::string& feature_path,
    std::vector<FeatureBase*>& features, ResourceManager* resource_manager,
    const std::map<std::string, ConditionBase*>& conditions) {
  DEBUG(LOGNAME, "construct_feature");
  Service<ObjectFactory> pFactory;
  ModelGroup group;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return false;
  FileInputStream* input = new FileInputStream(fd);
  bool success = google::protobuf::TextFormat::Parse(input, &group);
  // google::protobuf::TextFormat::PrintToString(vm, &tmp);
  delete input;
  close(fd);

  int num = group.feature_size();
  for (int i = 0; i < num; i++) {
    Feature* feature_proto = group.mutable_feature(i);
    feature_arg_t arg;
    arg.name = feature_proto->name();
    // hebe不走adfea逻辑
    if ((arg.name).compare("adfea") == 0) {
      DEBUG(LOGNAME, "construct_feature jumpy adfea");
      continue;
    }
    if (feature_proto->type() == Feature_Type_FILE)
      arg.type = "FILE";
    else
      arg.type = "REDIS";
    arg.condition = feature_proto->condition();
    for (int k = 0; k < feature_proto->data_size(); k++) {
      Feature_kvstore* kv = feature_proto->mutable_data(k);
      arg.data.insert(make_pair(kv->key(), kv->value()));
    }
    arg.group = feature_proto->group();
    if (arg.group == "" || arg.group == " ") arg.group = "begin";

    arg.config = feature_proto->config();
    if (arg.name.empty() || arg.name == "null") continue;

    FeatureBase* obj =
        static_cast<FeatureBase*>(pFactory->createObject(arg.name));
    if (obj == NULL) {
      ERROR(LOGNAME, "unregistered name %s", arg.name.c_str());
      return false;
    }
    if (!obj->Initialize(arg, conditions, resource_manager)) {
      FATAL(LOGNAME, "instance %s Initialize failed", arg.name.c_str());
      return false;
    }
    INFO(LOGNAME, "feature instance %s ok", arg.name.c_str());

    features.push_back(obj);
  }

  return true;
}

bool construct_extractor(
    const std::string& filename, const std::string& feature_path,
    std::vector<FeatureExtractorBase*>& extractors,
    ResourceManager* resource_manager,
    const std::map<std::string, ConditionBase*>& conditions) {
  DEBUG(LOGNAME, "construct_extractor");
  Service<ObjectFactory> pFactory;
  ModelGroup group;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return false;
  FileInputStream* input = new FileInputStream(fd);
  bool success = google::protobuf::TextFormat::Parse(input, &group);
  // google::protobuf::TextFormat::PrintToString(vm, &tmp);
  delete input;
  close(fd);

  int num = group.extractor_size();
  for (int i = 0; i < num; i++) {
    Extractor* _proto = group.mutable_extractor(i);
    extractor_arg_t arg;
    arg.name = _proto->name();
    arg.type = _proto->type();
    arg.condition = _proto->condition();
    arg.config = _proto->config();
    arg.top = _proto->top();
    if (arg.top == "" || arg.top == " ") arg.top = arg.name;
    if (arg.name.empty() || arg.name == "null") continue;

    string bottom_name;
    for (int k = 0; k < _proto->bottom_size(); k++) {
      bottom_name = _proto->bottom(k);
      if (find(arg.bottom.begin(), arg.bottom.end(), bottom_name) !=
          arg.bottom.end())
        continue;
      arg.bottom.push_back(_proto->bottom(k));
    }
    if (arg.bottom.size() == 0) arg.bottom.push_back("begin");

    FeatureExtractorBase* obj =
        static_cast<FeatureExtractorBase*>(pFactory->createObject(arg.type));
    if (obj == NULL) {
      ERROR(LOGNAME, "unregistered name %s", arg.name.c_str());
      return false;
    }
    if (!obj->Initialize(arg, conditions, resource_manager)) {
      FATAL(LOGNAME, "instance %s Initialize failed", arg.name.c_str());
      return false;
    }
    INFO(LOGNAME, "feature instance %s ok", arg.name.c_str());

    extractors.push_back(obj);
  }

  return true;
}

bool construct_model(const std::string& filename, const std::string& model_path,
                     std::vector<ModelBase*>& models,
                     ResourceManager* resource_manager,
                     const std::map<std::string, ConditionBase*>& conditions) {
  Service<ObjectFactory> pFactory;
  ModelGroup group;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return false;
  FileInputStream* input = new FileInputStream(fd);
  bool success = google::protobuf::TextFormat::Parse(input, &group);
  // google::protobuf::TextFormat::PrintToString(vm, &tmp);
  delete input;
  close(fd);

  int model_num = group.model_size();
  for (int i = 0; i < model_num; i++) {
    Model* model_proto = group.mutable_model(i);
    model_arg_t arg;

    arg.name = model_proto->name();
    if (arg.name.empty()) continue;

    arg.type = model_proto->type();
    arg.top = model_proto->top();
    if (arg.top == "" || arg.top == " ") arg.top = arg.name;
    // 如果模型为transformer，而且输出为end时，强制把end改成其名字
    if (model_proto->role() == graph::Model_Role_TRANSFORMER &&
        arg.top == "end")
      arg.top = arg.name;

    // 模型如果没有输入，则其输入为begin，如果存在输入则可以没有begin
    string bottom_name;
    for (int k = 0; k < model_proto->bottom_size(); k++) {
      bottom_name = model_proto->bottom(k);
      if (find(arg.bottom.begin(), arg.bottom.end(), bottom_name) !=
          arg.bottom.end())
        continue;
      arg.bottom.push_back(model_proto->bottom(k));
    }
    if (arg.bottom.size() == 0) arg.bottom.push_back("begin");

    if (model_proto->has_model_file())
      arg.model_file = model_proto->model_file();
    else
      arg.model_file = "";
    if (model_proto->has_map_file())
      arg.map_file = model_proto->map_file();
    else
      arg.map_file = "";
    if (model_proto->has_weight())
      arg.weight = model_proto->weight();
    else
      arg.weight = 1.0;
    arg.offset = model_proto->offset();
    arg.condition = model_proto->condition();
    arg.role = "predictor";
    if (model_proto->role() == ::rank::graph::Model_Role_TRANSFORMER)
      arg.role = "transformer";
    if (model_proto->has_formula() && model_proto->formula() != "")
      arg.formula = model_proto->formula();
    if (model_proto->has_sample_ratio())
      arg.sample_ratio = model_proto->sample_ratio();
    else
      arg.sample_ratio = 1.0f;

    ModelBase* obj = static_cast<ModelBase*>(pFactory->createObject(arg.type));
    if (obj == NULL) {
      ERROR(LOGNAME, "unregistered name %s", arg.type.c_str());
      return false;
    }
    if (!obj->Initialize(arg, conditions, resource_manager)) {
      FATAL(LOGNAME, "instance %s Initialize failed", arg.type.c_str());
      return false;
    }
    INFO(LOGNAME, "model instance %s ok", arg.name.c_str());

    models.push_back(obj);
  }

  return true;
}

bool construct_strategy(
    const std::string& filename, const std::string& strategy_path,
    std::vector<StrategyBase*>& strategies, ResourceManager* resource_manager,
    const std::map<std::string, ConditionBase*>& conditions) {
  Service<ObjectFactory> pFactory;
  ModelGroup group;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return false;
  FileInputStream* input = new FileInputStream(fd);
  bool success = google::protobuf::TextFormat::Parse(input, &group);
  // google::protobuf::TextFormat::PrintToString(vm, &tmp);
  delete input;
  close(fd);

  int num = group.strategy_size();
  for (int i = 0; i < num; i++) {
    Strategy* strategy_proto = group.mutable_strategy(i);
    strategy_arg_t arg;
    arg.name = strategy_proto->name();
    arg.order = strategy_proto->order();
    if (strategy_proto->has_config() && strategy_proto->config() != "")
      arg.config = strategy_proto->config();
    arg.condition = strategy_proto->condition();
    if (strategy_proto->has_classtype() &&
        !strategy_proto->classtype().empty()) {
      arg.classtype = strategy_proto->classtype();
    }

    if (strategy_proto->has_ids()) {
      vector<string> tmp, tmp2;
      AdUtil::Split3(tmp, strategy_proto->ids(), ',');
      AdUtil::Split3(tmp2, strategy_proto->desc(), ',');
      for (int j = 0; j < tmp.size(); j++) {
        arg.ids.push_back(
            atoi(tmp[j].c_str()));  // boost::lexical_cast<int>(tmp[j]));
        arg.desc.insert(make_pair(tmp2[j], j));
        // printf("strategy %s -> %d\n", tmp2[j].c_str(),
        // boost::lexical_cast<int>(tmp[j]));
      }
    }

    if (arg.name.empty() || arg.name == "null") continue;

    std::string classtype = arg.name;
    if (!arg.classtype.empty()) {
      classtype = arg.classtype;
    }
    StrategyBase* obj =
        static_cast<StrategyBase*>(pFactory->createObject(classtype));
    if (obj == NULL) {
      ERROR(LOGNAME, "unregistered name %s", arg.name.c_str());
      return false;
    }
    if (!obj->Initialize(arg, conditions, resource_manager)) {
      FATAL(LOGNAME, "instance %s Initialize failed", arg.name.c_str());
      return false;
    }
    INFO(LOGNAME, "strategy instance %s ok", arg.name.c_str());

    strategies.push_back(obj);
  }

  return true;
}
}  // namespace rank
