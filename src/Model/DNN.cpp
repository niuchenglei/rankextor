#ifdef DNN_COMPILE



#include "omp.h"
#include "DNN.h"

using namespace std;
using namespace boost;
using namespace kf::base;
using namespace dnn;






#define SCALE 10000

float dnn::FMTransformer::ctr_map_bin[] = {0,0.0004,0.0005,0.0006,0.0007,0.0008,0.0009,0.001,0.0011,0.0012,0.0013,0.0014,0.0015,0.0016,0.0017,0.0018,0.0019,0.002,0.0021,0.0022,0.0023,0.0024,0.0025,0.0026,0.0027,0.0028,0.0029,0.003,0.0031,0.0032,0.0033,0.0034,0.0035,0.0036,0.0037,0.0038,0.0039,0.004,0.0041,0.0042,0.0043,0.0044,0.0045,0.0046,0.0047,0.0048,0.0049,0.005,0.0051,0.0052,0.0053,0.0054,0.0055,0.0056,0.0057,0.0058,0.0059,0.006,0.0061,0.0062,0.0063,0.0064,0.0065,0.0066,0.0067,0.0068,0.0069,0.007,0.0071,0.0072,0.0073,0.0074,0.0075,0.0076,0.0077,0.0078,0.0079,0.008,0.0081,0.0082,0.0083,0.0084,0.0085,0.0086,0.0087,0.0088,0.0089,0.009,0.0091,0.0092,0.0093,0.0094,0.0095,0.0096,0.0097,0.0098,0.0099,0.01,0.0101,0.0102,0.0103,0.0104,0.0105,0.0106,0.0107,0.0108,0.0109,0.011,0.0111,0.0112,0.0113,0.0114,0.0115,0.0116,0.0117,0.0118,0.0119,0.012,0.0121,0.0122,0.0123,0.0124,0.0125,0.0126,0.0127,0.0128,0.0129,0.013,0.0131,0.0132,0.0133,0.0134,0.0135,0.0136,0.0137,0.0138,0.0139,0.014,0.0141,0.0142,0.0143,0.0144,0.0145,0.0146,0.0147,0.0148,0.0149,0.015,0.0151,0.0152,0.0153,0.0154,0.0155,0.0156,0.0157,0.0158,0.0159,0.016,0.0161,0.0162,0.0163,0.0165,0.0166,0.0167,0.0168,0.0169,0.0171,0.0172,0.0173,0.0174,0.0175,0.0176,0.0178,0.0179,0.018,0.0181,0.0182,0.0183,0.0184,0.0186,0.0187,0.0188,0.019,0.0191,0.0193,0.0196,0.0198,0.0199,0.0201,0.0202,0.0204,0.0206,0.0208,0.021,0.0213,0.0216,0.0218,0.022,0.0221,0.0223,0.0225,0.0227,0.0229,0.023,0.0232,0.0233,0.0236,0.0237,0.0239,0.0241,0.0243,0.0244,0.0246,0.0247,0.0248,0.025,0.0253,0.0255,0.0256,0.0257,0.0258,0.026,0.0262,0.0265,0.0268,0.0272,0.0275,0.0279,0.0284,0.0285,0.0287,0.0289,0.0296,0.0298,0.0305,0.0309,0.0317,0.0323,0.0328,0.0334,0.0342,0.0349,0.0356,0.0362,0.0372,0.038,0.0385,0.039,0.0401,0.042,0.0425,0.0446,0.0489,0.0508,0.053,0.0535,0.0539,0.0542,0.0548,0.0564,0.0582,0.0594,0.0605,0.0617,0.0627,0.0632,0.0642,0.0648,0.0654,0.0662,0.0671,0.0683,0.0691,0.07,0.0707,0.0725,0.0744,0.0769,0.0802,0.0845,0.0901,0.0981,0.1004,0.1017,0.1032,0.104,0.1059,0.1076,0.1094,0.1107,0.1141,0.1203,0.125,0.1277,0.1319,0.1356,0.1383,0.1415,0.1446,0.148,0.1529,0.1609,0.1634,0.1712};

REGISTE_CLASS("dnn", DNNModel)

DNNModel::DNNModel() {

}

DNNModel::~DNNModel() {
    
}

bool DNNModel::Initialize(model_arg_t& arg) {
    mpCondition = NULL;
    mArg = arg;
    mpCondition = NULL;
    
    DEBUG(LOGNAME, "model_file: %s, model_feature_map_file: %s", mArg.model_file.c_str(), mArg.map_file.c_str());
    
    input_layer.rows = 1; input_layer.cols = 100;
    input_layer.data = (float*)malloc(input_layer.rows*input_layer.cols*sizeof(float));
    hidden_layer_1.rows = 1; hidden_layer_1.cols = 100;
    hidden_layer_1.data = (float*)malloc(hidden_layer_1.rows*hidden_layer_1.cols*sizeof(float));
    hidden_layer_2.rows = 1; hidden_layer_2.cols = 107;
    hidden_layer_2.data = (float*)malloc(hidden_layer_2.rows*hidden_layer_2.cols*sizeof(float));
    output_layer.rows = 1; output_layer.cols = 2;
    output_layer.data = (float*)malloc(output_layer.rows*output_layer.cols*sizeof(float));

    //string prefix = getDirPath(mArg.model_file) + getFileName(mArg.model_file);      // actually it watch /data/dnn.touch file
    
    return Update();
}

bool DNNModel::Transform(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, int ad_index) {
    ERROR(LOGNAME, "dnn transform not implemented");
    return false;
}

bool DNNModel::Transform(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, std::vector<int>& ad_indexs) {
    ERROR(LOGNAME, "dnn transform not implemented");
    return false;
}

bool DNNModel::Predict(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, int ad_index) {
    //std::vector<AdBaseInfo*>& adlist = *(algo_in->ad_list);
    std::vector<AdInfo>& adlist = *(algo_in->ad_list);
    size_t ad_size = adlist.size();

    bool flag;
    vector<int> ad_indices;
    if (ad_index != -1) {
        ad_indices.push_back(ad_index);
    } else {
        for (int i=0; i<ad_size; i++)
            ad_indices.push_back(i);
    }
    flag = Predict(request, algo_in, ptd, ad_indices);

    return flag;
}

bool DNNModel::Predict(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, std::vector<int>& ad_indices) {
    DEBUG(LOGNAME, "dnn predict begin");

    dnn_coeff_t& model = mCoeff;
    //std::vector<AdBaseInfo*>& adlist = *algo_in->ad_list;
    std::vector<AdInfo>& adlist = *(algo_in->ad_list);
    size_t ad_size = ad_indices.size();

    int k, ad_index, bottom_num; 
    float sum_weight;
    unsigned int pid, thread_offset;
    
    #ifdef OMP
    #pragma omp parallel for \
        num_threads(NUM_THREAD) \ 
        shared(model, ptd, adlist) \
        private(k, ad_index, sum_weight, pid, thread_offset)
    #endif
    for (k=0; k<ad_size; k++) {
        //pid = omp_get_thread_num();

        ad_index = ad_indices[k];
        bottom_num = mArg.bottom.size();
        string bottom_name;

        // 执行condition
        if (!CheckCondition(adlist[ad_index], ptd))
            continue;

        unordered_map<uint32_t, double> _map;
        ptd.transform_features[ad_index].feature_list_["dnn_ctr"] = _map;

        // 遍历当前模型依赖的所有输入（即bottom）
        bool flag = true;
        for (int bottom_id=0; bottom_id<bottom_num; bottom_id++) {
            bottom_name = mArg.bottom[bottom_id];
            if (bottom_name != "begin") continue;
            boost::unordered_map<uint32_t, double>& _map = ptd.transform_features[ad_index].feature_list_[bottom_name];

            // 0.07 0.07 0.05 0.07 0.07 0.07 0.07
            _map[0] = 0.07; // 111.0/10000.0;
            _map[1] = 0.07;
            _map[2] = 0.05;
            _map[3] = 0.07;
            _map[4] = 0.07;
            _map[5] = 0.07;
            _map[6] = 0.07;

            flag &= fm_transer.transform(mCoeff.fm_coeff, _map, input_layer);
            if (!flag) FATAL(LOGNAME, "dnn fatal 1");


            printf("a0:");
            double arr[] = {0.4658,0.0341,0.0361,0.1002,-0.0475,0.0255,0.0635,-0.0827,0.0451,0.0278,0.0644,0.0173,0.347,-0.0094,-0.1498,0.1237,0.3584,0.1982,0.0559,0.0591,0.2118,0.1823,-0.063,-0.0272,0.0594,-0.167,0.2561,0.0674,0.0133,0.4873,0.1467,-0.0132,-0.1543,-0.4862,0.1314,-0.3095,0.0309,0,0,0,0,0,0,0,0,0,0.0154,0,-0.0142,-0.0248,-0.0113,0.0244,0.0161,-0.0239,0.0361,-0.2915,-0.3028,0.3301,-0.0542,-0.2429,0.039,-0.2319,-0.453,-0.1571,-0.0962,-0.0399,0.0128,0.0093,-0.0032,-0.0246,0.0113,-0.1395,0.0161,0.055,0.0094,-0.0128,-0.0444,0.0519,0.0249,0.0495,-0.0138,0.0445,-0.1285,-0.0655,-0.0549,0.0028,0.1378,0.0665,-0.0575,0.1309,-0.0501,-0.2025,-0.1201,-0.1052,0.0048,0.1809,0.0586,-0.0995,0.1757,-0.0481};
            for (int k=0; k<input_layer.rows*input_layer.cols; k++) {
                input_layer.data[k] = arr[k];
                printf("%f,", input_layer.data[k]);
            }
            printf("\n");

            
            flag &= nn_transer.transform(mCoeff.h1, &input_layer, 1, &hidden_layer_1, 1);
            if (!flag) FATAL(LOGNAME, "dnn fatal 2");

            printf("a1:");
            for (int k=0; k<hidden_layer_1.rows*hidden_layer_1.cols; k++) printf("%f,", hidden_layer_1.data[k]);
            printf("\n");


            flag &= nn_transer.transform(mCoeff.h2, &hidden_layer_1, 1, &hidden_layer_2, 1, 7);
            if (!flag) FATAL(LOGNAME, "dnn fatal 3");

            // 把连续特征加入
            hidden_layer_2.data[0] = _map[0]*100;
            hidden_layer_2.data[1] = _map[1]*100;
            hidden_layer_2.data[2] = _map[2]*100;
            hidden_layer_2.data[3] = _map[3]*100;
            hidden_layer_2.data[4] = _map[6]*100;
            hidden_layer_2.data[5] = _map[5]*100;
            hidden_layer_2.data[6] = _map[4]*100;

            printf("a2:");
            //float arr2[7] = {0.6500,0.6900,1.6000,0.6900,1.6800,0.6900,0.6100};
            //for (int k=0; k<7; k++) hidden_layer_2.data[k] = arr2[k];
            for (int k=0; k<hidden_layer_2.rows*hidden_layer_2.cols; k++) printf("%f,", hidden_layer_2.data[k]);
            printf("\n");


            // output layer with softmax
            flag &= nn_transer.transform(mCoeff.h3, &hidden_layer_2, 1, &output_layer, 2);

            printf("a3:");
            for (int k=0; k<output_layer.rows*output_layer.cols; k++) printf("%f,", output_layer.data[k]);
            printf("\n");

            // DNN 模型只有1个输入
        }
        //output_layer.data[0] = 0.5; output_layer.data[1] = 0.5;

        DEBUG(LOGNAME, "dnn predict prob is %f,%f", output_layer.data[0], output_layer.data[1]);
        //adlist[ad_index].ctr_ += mArg.weight * output_layer.data[1];
        unordered_map<uint32_t, double>& dnn_map = ptd.transform_features[ad_index].feature_list_["dnn_ctr"];
        dnn_map.insert(make_pair(0, mArg.weight * output_layer.data[1]));
    }

    //free(input_layer.data);
    //free(hidden_layer_1.data);
    //free(hidden_layer_2.data);
    //free(output_layer.data);

    DEBUG(LOGNAME, "dnn predict end");
    return true;
}

#define LOAD_MATRIX(r,c) \
dnn::matrix m; \
m.data = NULL; m.rows = r; m.cols = c; \ 
m.data = (float*)malloc(m.rows*m.cols*sizeof(float)); \
if (m.data == NULL) ERROR(LOGNAME, "malloc error"); \
if (m.rows*m.cols != values.size()) \
    ERROR(LOGNAME, "dnn theta error"); \
try { \
for (int k=0; k<m.rows*m.cols; k++) \
    m.data[k] = boost::lexical_cast<float>(values[k]); \
} catch (...) { ERROR(LOGNAME, "parse %s to float error", tmp[1].c_str()); } \
mCoeff.fm_coeff.insert(make_pair(tmp[0], m));

bool DNNModel::Update() {
    ifstream ifs(mArg.model_file.c_str());
    if (!ifs) {
        ERROR(LOGNAME, "can'f find file: %s", mArg.model_file.c_str());
        return false;
    }

    string line;
    int cnt = 0; 
    while (getline(ifs, line)) {
        vector<string> tmp, values; 
        AdUtil::Split2(tmp, line, ':');
        if (tmp.size() < 2)
            continue;

        int fmk = 9;
        AdUtil::Split2(values, tmp[1], ',');
        //['f0', 'f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'h2', 'h3', 'h1', 'b1', 'b2', 'b3']
        if (tmp[0] == "age") {
            LOAD_MATRIX(4, fmk);
        } else if (tmp[0] == "gender"){
            LOAD_MATRIX(2, fmk);
        } else if (tmp[0] == "plat"){
            LOAD_MATRIX(4, fmk);
        } else if (tmp[0] == "type"){
            LOAD_MATRIX(2, fmk);
        } else if (tmp[0] == "cls"){
            LOAD_MATRIX(74, fmk);
        } else if (tmp[0] == "loc"){
            LOAD_MATRIX(379, fmk);
        } else if (tmp[0] == "ins"){
            LOAD_MATRIX(99, fmk);
        } else if (tmp[0] == "ls"){
            LOAD_MATRIX(26, fmk);
        } else if (tmp[0] == "fw"){
            LOAD_MATRIX(2199, fmk);
        } else if (tmp[0] == "hie_ctr_bin"){
            LOAD_MATRIX(304, fmk);
        } else if (tmp[0] == "his_ctr_bin"){
            LOAD_MATRIX(304, fmk);
        } else if (tmp[0] == "w0"){
            LOAD_MATRIX(1, 1);
        } else if (tmp[0] == "h1"){
            LOAD_MATRIX(100, 100);
            mCoeff.h1.weight = m;
        } else if (tmp[0] == "b1"){
            LOAD_MATRIX(100, 1);
            mCoeff.h1.biases = m;
        } else if (tmp[0] == "h2"){
            LOAD_MATRIX(100, 100);
            mCoeff.h2.weight = m;
        } else if (tmp[0] == "b2"){
            LOAD_MATRIX(100, 1);
            mCoeff.h2.biases = m;
        } else if (tmp[0] == "h3"){
            LOAD_MATRIX(107, 2);
            mCoeff.h3.weight = m;
        } else if (tmp[0] == "b3"){
            LOAD_MATRIX(2, 1);
            mCoeff.h3.biases = m;
        } else
            continue;
    }
    mCoeff.h3.n_input = 107; mCoeff.h3.n_output = 2;
    mCoeff.h2.n_input = 100; mCoeff.h2.n_output = 100;
    mCoeff.h1.n_input = 100; mCoeff.h1.n_output = 100;


    return true;
}

const model_arg_t& DNNModel::getArgument() const {
    return mArg;
}

bool DNNModel::loadModel() {
    
    return true;
}

#endif
