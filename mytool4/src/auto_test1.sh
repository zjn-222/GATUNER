#!/bin/bash
program=dft
cd /home/zjn/mytool4/src

for err in 10 8 6 4; do
# for err in 10; do
    # 替换 ErrorThreshold 值
    sed -i "s/double ErrorThreshold = 1e-[0-9]\+/double ErrorThreshold = 1e-${err}/g" GeneticAlgorithm.cpp
    # for mu in $(seq 0.8 0.1 0.9); do
    # for mu in 0.9; do
        # for cs in $(seq 0.1 0.1 0.9); do
            # 修正浮点转整数逻辑
            # mu_str=$(printf "%02d" $(echo "$mu*10/1" | bc))
            # cs_str=$(printf "%02d" $(echo "$cs*10/1" | bc))
            
            # 替换参数和日志路径
            # sed -i "s/mutationRate *= *[0-9.]\+/mutationRate = ${mu}/g" GeneticAlgorithm.cpp
            # sed -i "s/crossoverRate *= *[0-9.]\+/crossoverRate = ${cs}/g" GeneticAlgorithm.cpp
            sed -i -E "s#/home/zjn/mytool4/log/auto_test1_${program}_e[0-9]+\.txt#/home/zjn/mytool4/log/auto_test1_${program}_e${err}.txt#g" GeneticAlgorithm.cpp
            # 编译执行
            g++ -std=c++17 GeneticAlgorithm.cpp -o GeneticAlgorithm
            ./GeneticAlgorithm
        # done
    # done
done