#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <boost/unordered_map.hpp>
#include "ini.c"
#include "INIReader.cpp"
#include "AdUtil.h"
#include "Util.h"

using namespace boost;

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

void iterate_tree(vector<node_t>& nodes, int current_idx, int indent) {
  for (int i = 0; i < indent; i++) printf(" ");
  printf("[%d] feature[%d]<%f\n", current_idx, nodes[current_idx].feature,
         nodes[current_idx].thresh);

  int left = nodes[current_idx].left, right = nodes[current_idx].right;

  if (left != -1) iterate_tree(nodes, left, indent + 4);
  if (right != -1) iterate_tree(nodes, right, indent + 4);
}

typedef struct {
  int feature;
  float thresh;
  int sign;  // 1:big or equal than thresh, -1:less than thresh$
} node_attr_t;
void iterate_parent(vector<node_t>& nodes, int child,
                    vector<node_attr_t>& attr) {
  if (child == 0) return;
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].left != child && nodes[i].right != child) continue;

    if (nodes[i].left == child) {
      node_attr_t _t;
      _t.feature = nodes[i].feature;
      _t.thresh = nodes[i].thresh;
      _t.sign = -1;
      attr.push_back(_t);
    }
    if (nodes[i].right == child) {
      node_attr_t _t;
      _t.feature = nodes[i].feature;
      _t.thresh = nodes[i].thresh;
      _t.sign = 1;
      attr.push_back(_t);
    }
    iterate_parent(nodes, i, attr);
  }
}
vector<node_attr_t> merge_node_attr(vector<node_attr_t>& attr) {
  vector<node_attr_t> new_attr;
  vector<int> feature;
  for (int i = 0; i < attr.size(); i++)
    if (find(feature.begin(), feature.end(), attr[i].feature) == feature.end())
      feature.push_back(attr[i].feature);
  for (int i = 0; i < feature.size(); i++) {
    int feature_id = feature[i];
    float max = 10000, min = -10000;
    for (int k = 0; k < attr.size(); k++) {
      if (attr[k].feature != feature_id) continue;
      if (attr[k].sign == 1) {
        if (max >= attr[k].thresh) max = attr[k].thresh;
      }
      if (attr[k].sign == -1) {
        if (min < attr[k].thresh) min = attr[k].thresh;
      }
    }
    node_attr_t _t;
    _t.feature = feature_id;
    _t.thresh = max;
    _t.sign = 1;
    if (max != 10000) new_attr.push_back(_t);
    _t.feature = feature_id;
    _t.thresh = min;
    _t.sign = -1;
    if (min != -10000) new_attr.push_back(_t);
  }
  return new_attr;
}
vector<vector<node_attr_t> > iterate_leaf(vector<node_t>& nodes) {
  vector<vector<node_attr_t> > nodes_attr;
  for (int i = 0; i < nodes.size(); i++) {
    if (nodes[i].left != -1 && nodes[i].right != -1) continue;

    int leaf_idx = i;
    vector<node_attr_t> leaf_attr;
    iterate_parent(nodes, leaf_idx, leaf_attr);

    vector<node_attr_t> leaf_attr_red = merge_node_attr(leaf_attr);
    nodes_attr.push_back(leaf_attr_red);
  }
  return nodes_attr;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <model-file> <output-file>" << endl;
    return -1;
  }
  const char* model_file = argv[1];
  gbdt_coeff_t model;

  if (!loadModel(model, model_file)) {
    cerr << "loadModel error" << endl;
  }

  int num_of_trees = model.mTrees.size();
  for (int i = 0; i < num_of_trees; i++) {
    vector<node_t>& nodes = model.mTrees[i];

    /*        int current_idx = 0;
            iterate_tree(nodes, current_idx, 0);
    break;*/
    vector<vector<node_attr_t> > nodes_attr = iterate_leaf(nodes);
    for (int i = 0; i < nodes_attr.size(); i++) {
      vector<node_attr_t>& attr = nodes_attr[i];
      float range[2] = {0, 10000};
      for (int k = 0; k < attr.size(); k++) {
        if (attr[k].feature != 0) continue;
        if (attr[k].sign == 1)
          range[0] = attr[k].thresh;
        else
          range[1] = attr[k].thresh;
        // printf("%c%.0f ", (attr[k].sign==1)?'>':'<', attr[k].thresh);
      }
      printf("%.0f,%.0f;", range[0], range[1]);
    }
    printf("\n");
  }

  return 0;
}
