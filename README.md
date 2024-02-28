# High performance rank executor for advertisement and recommendation system, implemented in C/C++ and support ensembled into Java/Scala host as plguin or independent process, and it support ML models trained by Xgboost in Python or event Tensorflow deep learning models.

Rankextor is a set of algorithm execution engines based on C/C++. It is mainly used for algorithm serving in the fields of recommendation, advertising, and search. It supports embedding into Java/Scala service processes or providing RPC ranking services as an independent process.

## Features
1. Flexible and efficient "graph" execution engine.

2. Convenient feature, strategy, model, and feature engineering support.
    - Features such as:
        - Basic user portrait characteristics
        - Statistical feedback CTR features
        - Material side characteristics
    - Click-through rate estimation algorithm is as follows:
        - LR
        - FM
        - GBDT+LR
        - Deep Learning (Pytorch)
    - Feature engineering processing such as:
        - Equal frequency, equal distance scattering
        - Bucket
        - Feature crossover
        - Multiple feature support
        - Sample sampling
        - hash discrete
    - Strategies such as:
        - Scoring and ranking
        - Sort and adjust
        - Material filtration

3. Embed Java services, or provide RPC services as independent processes

4. Dynamic loading of model files and configuration files

## installation steps

```
cmake ..
make

```

For pytorch model training and serving part, check the `pytorch_ranking`

## DAG Configuration
The following configuration defines what kind of features we need, and what models shall take charge and what kind of rank strategies are needed.
```
name: "model"
condition {
    name:"bidfeed"
}
condition {
    name:"cpm"
}
condition {
    name:"cpc"
}
condition {
    name:"ocpm"
}
feature {
    name: "request_feature"
    data: { key: "redis_user_schema" value: "redis_user_schema.txt" }
    group: "begin"
}
extractor {
    name: "xfea1"
    type: "xfea"
    bottom: "begin"
    top: "ctcvr_fea"
    config: "suning_features.conf"
}
model {
    name: "gds_ctcvr1"
    type: "fm"
    bottom: "ctcvr_fea"
    top: "ctcvr"
    role: PREDICTOR
    model_file: "fm.model"
} 
strategy {
    name: "reset"
    order: -20
}
strategy {
    name: "calibrate"
    order: 1
    ids: "1"
    desc: "cut"
}
strategy {
    name: "ocpx"
    order: 2
}
strategy {
    name: "rankscore"
    order: 3
}
strategy {
    name: "sort"
    order: 4
}
```
