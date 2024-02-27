rankextor

# High performance rank executor for advertisement and recommendation system, implemented in C/C++ and support ensembled into Java host as plguin or independent process.

Rankextor is a set of algorithm execution engines based on C/C++. It is mainly used for algorithm serving in the fields of recommendation, advertising, and search. It supports embedding into Java service processes or providing RPC sorting services as an independent process.

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
        - Deep Learning (Tensorflow)
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
Need integrity within your own code base.

