#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <boost/unordered_map.hpp>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "ini.c"
#include "INIReader.cpp"
#include "AdUtil.h"
//#include "Util.h"

using namespace boost;

static bool isLong(const std::string& str, long* l) {
  try {
    string str1(str);
    boost::trim(str1);
    *l = boost::lexical_cast<long>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not long string: str=%s", str.c_str());
    return false;
  }
  return true;
}
static bool isFloat(const std::string& str, float* f) {
  try {
    string str1(str);
    boost::trim(str1);
    *f = boost::lexical_cast<float>(str1);
  }
  catch (...) {
    // DEBUG(LOGNAME,"not float string: str=%s", str.c_str());
    return false;
  }
  return true;
}

typedef struct {
  long node_id;
  long left, right;
  float impurity;
  long feature;
  float thresh;
  long feature_type;
  std::vector<std::string> feature_list;
  float value;  // only leaf node has value
} node_t;

typedef struct {
  boost::unordered_map<int, float> mTreeWeights;
  boost::unordered_map<int, std::vector<node_t> > mTrees;
  boost::unordered_map<int, boost::unordered_map<int, int> > mLeafCodes;
  float mConstWeight;
  boost::unordered_map<uint64_t, std::vector<long> > mFeatureMappingKey;
  boost::unordered_map<uint64_t, boost::unordered_map<long, long> >
      mFeatureMappingValue;
} gbdt_coeff_t;

bool parseNodeFromString(const std::string& node_str, int node_id,
                         node_t& node) {
  // left_id, right_id, impurity(infomation gain), feature_id/feature_name,
  // thresh, feature_type(0 value/1 category), value_list(category list),
  // value(only leaf valid)
  // 1,2,0.010769,2,575.500000,0,,0.000000
  if (node_str == "") return false;

  vector<string> tmp;
  AdUtil::Split3(tmp, node_str, ',');
  long left, right, feature, feature_type;
  float impurity, thresh, value;
  if (!isLong(tmp[0], &left) || !isLong(tmp[1], &right) ||
      !isFloat(tmp[2], &impurity) || !isLong(tmp[3], &feature) ||
      !isFloat(tmp[4], &thresh) || !isLong(tmp[5], &feature_type) ||
      !isFloat(tmp[7], &value)) {
    return false;
  }
  node.left = left;
  node.right = right;
  node.impurity = impurity;
  node.feature = feature;
  node.thresh = thresh;
  node.feature_type = feature_type;
  node.value = value;
  string value_list = tmp[6];
  tmp.clear();
  AdUtil::Split3(tmp, value_list, ';');
  for (int i = 0; i < tmp.size(); i++) {
    node.feature_list.push_back(tmp[i]);
  }

  return true;
}

bool loadModel(gbdt_coeff_t& model, const char* model_file) {
  ifstream ifs(model_file);
  if (!ifs) {
    return false;
  }

  unordered_map<int, std::vector<node_t> >().swap(model.mTrees);  //释放内存
  INIReader* gbdt_define = new INIReader(model_file);
  if (gbdt_define == NULL) {
    return false;
  }

  int num_trees = gbdt_define->GetInteger("declare", "num", 0);
  string weights = gbdt_define->Get("declare", "weights", "");
  vector<string> tmp;
  AdUtil::Split3(tmp, weights, ',');
  for (int i = 0; i < tmp.size(); i++) {
    float v;
    if (!isFloat(tmp[i], &v)) {
      return false;
    }
    model.mTreeWeights.insert(make_pair(i, v));
  }

  char tree_name[256], node_name[256];
  int max_leaf_index = 0;
  int leaf_node_code = 0;  // 叶子节点编号，用于做特征变换
  for (int i = 0; i < num_trees; i++) {
    sprintf(tree_name, "tree_%d", i);
    string tree_name_str(tree_name);

    int node_id = 0, max_leaf_index = -1;
    vector<node_t> tree;
    boost::unordered_map<int, int> leaf_code;
    while (true) {
      sprintf(node_name, "node_%d", node_id);
      string node_name_str(node_name);

      string node_str = gbdt_define->Get(tree_name_str, node_name_str, "");
      if (node_str == "" && max_leaf_index > node_id) {
        node_t node;
        tree.push_back(node);
        node_id += 1;
        continue;
      }
      if (node_str == "" && max_leaf_index <= node_id) {
        break;
      }

      // 解析树节点
      node_t node;
      node.left = -1;
      node.right = -1;
      if (!parseNodeFromString(node_str, node_id, node)) return false;
      if (max_leaf_index < node.left) max_leaf_index = node.left;
      if (max_leaf_index < node.right) max_leaf_index = node.right;
      tree.push_back(node);
      if (node.left == -1 && node.right == -1) {
        leaf_code.insert(make_pair(node.node_id, leaf_node_code));
        leaf_node_code += 1;
      }

      node_id += 1;
    }
    model.mTrees.insert(make_pair(i, tree));
    model.mLeafCodes.insert(make_pair(i, leaf_code));
  }
  delete gbdt_define;

  return true;
}

bool transform(unordered_map<uint32_t, uint32_t>& features, gbdt_coeff_t& model,
               unordered_map<uint32_t, uint32_t>& output) {
  int num_trees = model.mTreeWeights.size();

  for (int i = 0; i < num_trees; i++) {
    std::vector<node_t>& tree = model.mTrees[i];
    boost::unordered_map<int, int>& leaf_code = model.mLeafCodes[i];

    int leaf_index = 0;
    while (tree[leaf_index].left != -1 && tree[leaf_index].right != -1) {
      int feature_index = tree[leaf_index].feature;

      int feature_value = 0;
      if (features.find(feature_index) != features.end())
        feature_value = features[feature_index];

      if (feature_value <= tree[leaf_index].thresh)
        leaf_index = tree[leaf_index].left;
      else
        leaf_index = tree[leaf_index].right;
    }
    long feature_idx =
        leaf_code[leaf_index];  // 经过gbdt叶子节点编号后的叶节点id

    output.insert(make_pair(feature_idx, 1));
  }

  return true;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <model-file>" << endl;
    return -1;
  }
  const char* model_file = argv[1];
  gbdt_coeff_t model;

  if (!loadModel(model, model_file)) {
    cerr << "loadModel error" << endl;
    return -1;
  }

  string line;
  while (getline(cin, line)) {
    //-1 1 |f 609:1 3097:1 3769:1 1728:1 4573:1 1293:1 2441:1 4213:1 4201:1
    unordered_map<uint32_t, uint32_t> input;
    unordered_map<uint32_t, uint32_t> output;

    vector<string> tmp;
    AdUtil::Split3(tmp, line, ' ');
    for (int k = 3; k < tmp.size(); k++) {
      input.insert(make_pair(k - 3, atoi(tmp[k].c_str())));
    }

    if (!transform(input, model, output)) {
      cerr << "transform error" << endl;
      continue;
    }

    cout << tmp[0] << "\t" << tmp[1] << "\t" << tmp[2] << "\t";
    for (unordered_map<uint32_t, uint32_t>::iterator iter = output.begin();
         iter != output.end(); iter++) {
      cout << iter->first << ":1"
           << "\t";
    }
    cout << endl;
  }

  return 0;
}
