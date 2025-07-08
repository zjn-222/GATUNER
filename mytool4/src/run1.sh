#!/bin/bash
# 使用方法：./run.sh 10 ，其中10为指定的chromosome染色体数量

# 从config.json------>RuntimeAndError.json
#串行版本


# 调用merge.cpp
# 将Config.json与变量名称列表对应起来，生成新的ConfigMerged.json文件
g++ -std=c++17 /home/zjn/mytool4/src/merge.cpp -o /home/zjn/mytool4/src/merge
cd /home/zjn/mytool4/src/
./merge

cd /home/zjn/mytool4/file/
rm *
cd /home/zjn/mytool4/src/



# 调用loop-convert16
# 检查是否提供了参数
if [ $# -ne 1 ]; then
    echo "错误：必须指定染色体数量参数"
    echo "示例: ./run.sh 100"
    exit 1
fi

N=$1

echo "运行loop-convert20"
   # loop-convert20根据ConfigMerged.json，先把=右边的DelRefExpr节点改成__half2float(a)，这样不会影响=左边
for i in $(seq 1 $N); do
    # 生成染色体文件
    /home/zjn/llvm-project-main/build/bin/loop-convert20 --chromosome=$i /home/zjn/mytool4/source_program/simpsons1.cu -- >>/home/zjn/mytool4/file/chromosome_res_20_${i}.cu 2>/dev/null
    # 替换JSON键名为当前染色体编号
    sed -i "s/\"chromosome1\"/\"chromosome${i}\"/g" /home/zjn/mytool4/file/chromosome_res_20_${i}.cu
done

echo "运行loop-convert16"
   # loop-convert16根据ConfigMerged.json无脑替换变量定义精度为half,float,double，这样不会影响到=左边的
for i in $(seq 1 $N); do
    # 生成染色体文件
    /home/zjn/llvm-project-main/build/bin/loop-convert16 --chromosome=$i /home/zjn/mytool4/file/chromosome_res_20_${i}.cu -- >>/home/zjn/mytool4/file/chromosome_res${i}.cu 2>/dev/null
    # 替换JSON键名为当前染色体编号
    # sed -i "s/\"chromosome1\"/\"chromosome${i}\"/g" /home/zjn/mytool4/file/chromosome_res${i}.cu
done

# echo "运行loop-convert18"
#   #loop-convert18 无脑将所有half变量添加__half2float()
# for i in $(seq 1 $N); do
#     # 生成染色体文件
#     /home/zjn/llvm-project-main/build/bin/loop-convert18 /home/zjn/mytool4/file/chromosome_res${i}.cu -- >>/home/zjn/mytool4/file/chromosome_res_res${i}.cu 2>/dev/null
# done

echo "运行loop-convert19"
  #loop-convert19 检测到=号右边如果全部都是half，则去掉__half2float()
for i in $(seq 1 $N); do
    # 生成染色体文件
    /home/zjn/llvm-project-main/build/bin/loop-convert19 /home/zjn/mytool4/file/chromosome_res${i}.cu -- >>/home/zjn/mytool4/file/chromosome${i}.cu 2>/dev/null
done

echo "------------------------------------------------------准备编译---------------------------------------------------------"


# 运行这些精度配置
cd /home/zjn/mytool4/file
for i in $(seq 1 $N); do
    nvcc -arch=sm_86 /home/zjn/mytool4/file/chromosome${i}.cu -o chromosome${i}
done

echo "产生 $N 个编译后的文件."


for i in $(seq 1 $N); do
    ./chromosome${i}
done



