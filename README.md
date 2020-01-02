rankextor

# High performance rank executor for advertisement and recommendation system, implemented in C/C++ and support ensembled into Java host as plguin or independent process.

rankextor是一套基于C/C++的算法执行引擎，主要用于推荐、广告、搜索领域的算法排序，支持嵌入到Java服务进程中，或作为独立的进程提供RPC排序服务。

## 功能特点

1. 灵活高效的“图”执行引擎。

2. 方便的特征、策略、模型、特征工程支持。

    * 特征如：

        * 基本用户画像特征

        * 统计反馈CTR特征

        * 物料侧特征

    * 点击率预估算法如：

        * LR

        * FM

        * GBDT+LR

        * Wide\&Deep (coming soon)

    * 特征工程处理如：

        * 等频、等距离散

        * 分桶

        * 特征交叉

        * 多特征支持

        * 样本采样

        * hash离散

    * 策略如：

        * 排序打分

        * 排序调权

        * 物料过滤

3. 嵌入Java服务，或作为独立进程提供RPC服务

4. 模型文件、配置文件动态热加载

## 安装步骤

## 使用指南
