#ifndef DNN_H
#define DNN_H

#ifdef DNN_COMPILE


#include <sstream>
#include <time.h>
#include <math.h>
#include <sharelib/util/string_util.h>
#include <boost/unordered_map.hpp>
#include "AdUtil.h"
#include "ConstantField.h"
#include "ModelBase.h"

using namespace std;

typedef struct matrix {
    int rows, cols;
    float *data;
};
typedef struct layer_coeff_t {
    int n_input, n_output;
    dnn::matrix weight;
    dnn::matrix biases;
};
typedef struct dnn_coeff_t {
    boost::unordered_map<string, dnn::matrix> fm_coeff;
    layer_coeff_t h1, h2, h3;
};

class FMTransformer {
public:
    static float ctr_map_bin[304];
    // 输入fm参数、特征，输出样本变换之后的矩阵64（7个特征，每个特征9个维度，另加1个常数项）
    // coeff中有w0,age,gender,plat,type,cls,hie_ctr_bin,his_ctr_bin
    // output: m*64
    FMTransformer() {
        // feature_names = ['age', 'gender', 'plat', 'type', 'cls', 'loc', 'ins', 'ls', 'fw', 'hie_ctr_bin', 'his_ctr_bin']
        // [4,2,4,2,74,379,99,26,2199,304,304]
        //feature_offset.insert(make_pair()); 
    }

    bool transform(boost::unordered_map<string, dnn::matrix>& coeff, 
        boost::unordered_map<uint32_t, double>& feature,
        dnn::matrix& output) {
        int _offset[11] = {4,2,4,2,74,379,99,26,2199,304,304}, offset[12]={0};
        for (int k=0; k<11; k++) 
            offset[k+1] = _offset[k] + offset[k];
        int v = -1;
        for (int i=0; i<output.rows*output.cols; i++) output.data[i] = 0;

        for (boost::unordered_map<uint32_t, double>::iterator iter=feature.begin(); iter!=feature.end(); iter++) {
            //if (feature_offset.find(iter->first) != feature_offset.end())
            //    v = feature_offset.find(iter->first).second;

            int feature_name = iter->first;
            if (feature_name == 8) // age
                v = 0; //offset[0];
            else if (feature_name == 9) // gender
                v = 1; //offset[1];
            else if (feature_name == 10) // plat
                v = 2; //offset[2];
            else if (feature_name == 11) // type
                v = 3; //offset[3];
            // cls/loc/ins/ls/fw特征的存储方式是<特征值,1.0>，而其他特征是<特征名,特征值>
            else if (feature_name >= 100000 && feature_name < 200000) // cls
                v = 4; //offset[4];
            else if (feature_name >= 200000 && feature_name < 300000) // loc
                v = 5; //offset[5];
            else if (feature_name >= 300000 && feature_name < 400000) // ins
                v = 6; //offset[6];
            else if (feature_name >= 400000 && feature_name < 500000) // ls
                v = 7; //offset[7];
            else if (feature_name >= 500000 && feature_name < 600000) // fw
                v = 8; //offset[8];
            else if (feature_name == 0)
                v = 9;
            else if (feature_name == 1)
                v = 10;
            else
                continue;

            if (!mapping(iter->first, iter->second, coeff, output.data+v*9+1))
                break;
            }
        }
        //printf("===");

        output.data[0] = coeff["w0"].data[0];
/*for(int k=0;k<output.cols;k++){
printf(" %f,", output.data[k]);
}
printf("\n");
*/
        return true;
    }
    /*
        Feature('age', 'map', map_table={'age_1':0, 'age_2':1, 'age_3':2, 'age_4':3}),
        Feature('gender', 'map', map_table={'gender_1':0, 'gender_2':1}),
        Feature('plat', 'map', map_table={'plat_1101':0, 'plat_110200':1, 'plat_110201':2, 'plat_110202':3}),
        Feature('type', 'map', map_table={'type_4':0, 'type_8':1}),
        Feature('cls', 'hash', hash_size=100),
        Feature('hie_ctr_bin', 'bins', column='hie_ctr', bin_desc=bin_map),
        Feature('his_ctr_bin', 'bins', column='his_ctr', bin_desc=bin_map),
    */
    bool mapping(int feature_name, double feature_value, 
        boost::unordered_map<string, dnn::matrix>& coeff,
        float* pool) {
        int vv = feature_value;
        int fmk = 9;

        if (feature_name == 8) {    // age
            dnn::matrix& cc = coeff["age"];
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name == 9) {    // gender
            if (coeff.find("gender") == coeff.end()) return false;
            dnn::matrix& cc = coeff["gender"];
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name == 10) {    // plat
            if (coeff.find("plat") == coeff.end()) return false;
            dnn::matrix& cc = coeff["plat"];
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name == 11) {    // type
            if (coeff.find("type") == coeff.end()) return false;
            dnn::matrix& cc = coeff["type"];
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name > 100000) {    // cls_101000
            if (coeff.find("cls") == coeff.end()) return false;
            // cls 特征需要特殊处理下，因为会有多个数值返回，在特征层面如果是大于100000的就是cls特征
            dnn::matrix& cc = coeff["cls"];
            vv = feature_name;
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name > 200000) {
            dnn::matrix& cc = coeff["loc"];
            vv = feature_name;
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name > 300000) {
            dnn::matrix& cc = coeff["ins"];
            vv = feature_name;
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name > 400000) {
            dnn::matrix& cc = coeff["ls"];
            vv = feature_name;
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name > 500000) {
            dnn::matrix& cc = coeff["fw"];
            vv = feature_name;
            for (int k=0; k<fmk; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name == 0) {    // hie_ctr_bin
            if (coeff.find("hie_ctr_bin") == coeff.end()) return false;
            // 查找在map_bin中第几个bin
            dnn::matrix& cc = coeff["hie_ctr_bin"];
            int bi = 0;
            for (int k=0; k<303; k++) {
                if (feature_value>=ctr_map_bin[k] && feature_value<ctr_map_bin[k+1]) {
                    bi = k;
                    break;
                }
            }
            vv = bi;
            if (vv >303) vv = 303; if (vv<0) vv = 0;

            for (int k=0; k<9; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        } else if (feature_name == 1) {    // his_ctr_bin
            if (coeff.find("his_ctr_bin") == coeff.end()) return false;
            // 查找在map_bin中第几个bin
            dnn::matrix& cc = coeff["his_ctr_bin"];
            int bi = 0;
            for (int k=0; k<303; k++) {
                if (feature_value>=ctr_map_bin[k] && feature_value<ctr_map_bin[k+1]) {
                    bi = k;
                    break;
                }
            }
            vv = bi;
            if (vv >303) vv = 303; if (vv<0) vv = 0;
            
            for (int k=0; k<9; k++) {
                pool[k] += cc.data[vv*cc.cols+k];
            }
        }
        //printf("-- features: %d, %f->%d\n", feature_name, feature_value,vv);
        return true;
    }
};

class NNTransformer {
public:
    // 输入隐含层参数、输入层，输出变换后结果，可以一次处理多个样本
    // active_func: -1/null, 0/sigmoid, 1/relu, 2/softmax
    bool transform(layer_coeff_t& coeff,
        dnn::matrix* input, size_t input_n,
        dnn::matrix* output, int active_func=-1, int offset=0) {
        if (input_n != 1)
            return false;
        /*if (input->cols != coeff.weight.rows ||
            coeff.weight.cols != coeff.biases.rows ||
            coeff.weight.cols != output->cols) {
            ERROR(LOGNAME, "dnn matrix cross-product not match");
            return false;
        }*/
        if (input->rows != 1 || output->rows != 1) {
            ERROR(LOGNAME, "dnn matrix sample size error");
            return false;
        }

        for (int k=0; k<output->rows*output->cols; k++)
            output->data[k] = 0;
        for (int r=0; r<coeff.weight.cols; r++) {
            float v = 0;
            for (int c=0; c<coeff.weight.rows; c++) {
                v += coeff.weight.data[c*coeff.weight.cols+r]*input->data[c];
            }
            if (r>coeff.biases.rows) {FATAL(LOGNAME, "dnn transform 0, %d, %d", r, coeff.biases.rows); return false;}
            v += coeff.biases.data[r];
            if ((v+offset)>output->cols) {FATAL(LOGNAME, "dnn transform 1, %d, %d", v+offset, output->cols); return false;}
            output->data[r+offset] = v;
        }
        if (active_func == 1) {
            // Relu
            for (int k=0; k<output->cols; k++) {
                if (output->data[k] < 0)
                    output->data[k] = 0;
            }
        } else if (active_func == 2) {
            // Softmax
            float v = 0, vv;
            for (int k=0; k<output->cols; k++) {
                vv = exp(output->data[k+offset]);
                output->data[k+offset] = vv;
                v += vv;
            }
            for (int k=0; k<output->cols; k++) {
                output->data[k+offset] /= v;
            }
        }
        return true;
    }
};

class DNNModel : public ModelBase {
public:
    OBJECT_CLASS(DNNModel)

    DNNModel();
    ~DNNModel();

    bool Initialize(model_arg_t& arg);
    bool Transform(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, int ad_index);
    bool Transform(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, std::vector<int>& ad_indices);
    bool Predict(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, int ad_index);
    bool Predict(const AdRequest* request, const AlgoIns* algo_in, RankDataTuple &ptd, std::vector<int>& ad_indices);
    bool Update();
    const model_arg_t& getArgument() const;

private:
    model_arg_t mArg;
    dnn::dnn_coeff_t mCoeff;
    dnn::FMTransformer fm_transer;
    dnn::NNTransformer nn_transer;

    // input_layer: m*64
    // hidden_layer_1: m*50
    // hidden_layer_2: m*57
    // output_layer: m*2
    dnn::matrix input_layer, hidden_layer_1, hidden_layer_2, output_layer;

    bool loadModel();
    int mapping();
};

#endif
#endif
