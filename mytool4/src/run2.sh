#!/bin/bash
# 使用方法：./run.sh 10 ，其中10为指定的chromosome染色体数量

# 从config.json------>RuntimeAndError.json
#并行版本！！！！CPU和GPU并行

# 检查参数
if [ $# -ne 1 ]; then
    echo "错误：必须指定染色体数量参数"
    echo "示例: ./run.sh 100"
    exit 1
fi

N=$1
PARALLEL_JOBS=50  # 根据GPU显存调整并行任务数
LOG_DIR="/home/zjn/mytool4/logs"
mkdir -p $LOG_DIR

# 第一阶段：生成ConfigMerged.json
echo "生成合并配置..."
g++ -std=c++17 /home/zjn/mytool4/src/merge.cpp -o /home/zjn/mytool4/src/merge
cd /home/zjn/mytool4/src/
./merge > $LOG_DIR/merge.log 2>&1

# 清理工作目录
cd /home/zjn/mytool4/file/
rm -f *
echo "清空/file目录..."
#清理logs
cd /home/zjn/mytool4/logs/
rm -f *
echo "清空/logs目录..."
#清理json
cd /home/zjn/mytool4/json/
rm -f *
echo "清空/json目录..."

# 第二阶段：并行处理染色体（使用两阶段并行）
cd /home/zjn/mytool4/src/
echo "并行生成染色体文件（共$N个）..."
for i in $(seq 1 $N); do
    (
        # 步骤1：loop-convert20处理
        /home/zjn/llvm-project-main/build/bin/loop-convert20 --chromosome=$i /home/zjn/mytool4/source_program/simpsons1.cu -- >> /home/zjn/mytool4/file/chromosome_res_20_${i}.cu 2>/dev/null
        
        # 步骤2：替换JSON键名
        sed -i "s/\"chromosome1\"/\"chromosome${i}\"/g" /home/zjn/mytool4/file/chromosome_res_20_${i}.cu
        
        # 步骤3：loop-convert16处理
        /home/zjn/llvm-project-main/build/bin/loop-convert16 --chromosome=$i /home/zjn/mytool4/file/chromosome_res_20_${i}.cu -- >> /home/zjn/mytool4/file/chromosome_res${i}.cu 2>/dev/null
        
        # 步骤4：loop-convert19处理
        /home/zjn/llvm-project-main/build/bin/loop-convert19 /home/zjn/mytool4/file/chromosome_res${i}.cu -- >> /home/zjn/mytool4/file/chromosome${i}.cu 2>$LOG_DIR/convert19_${i}.log
    ) &
    
    # 控制并行任务数量
    if [[ $(jobs -r -p | wc -l) -ge $PARALLEL_JOBS ]]; then
        wait -n
    fi
done
wait

echo "--------------------------------------------------编译阶段---------------------------------------------------"

# 第三阶段：并行编译和执行
echo "并行编译和执行（共$N个）..."
cd /home/zjn/mytool4/file
for i in $(seq 1 $N); do
    (
        # 记录开始时间
        start_time=$(date +%s)
        
        # 编译
        # nvcc -arch=sm_86 chromosome${i}.cu -o chromosome${i} > $LOG_DIR/compile_${i}.log 2>&1
        nvcc -arch=sm_86 chromosome${i}.cu -o chromosome${i} -lm --expt-relaxed-constexpr > $LOG_DIR/compile_${i}.log 2>&1
        
        # 检查编译是否成功
        if [ $? -eq 0 ]; then
            # 运行并记录性能
            ./chromosome${i} >> $LOG_DIR/run_${i}.log 2>&1
            exit_code=$?
            
            # 计算耗时
            end_time=$(date +%s)
            duration=$((end_time - start_time))
            
            # 记录结果
            echo "chromosome${i} | 状态:$exit_code | 耗时:${duration}s" >> $LOG_DIR/summary.log
        else
            echo "chromosome${i} | 编译失败" >> $LOG_DIR/summary.log
        fi
    ) &
    
    # 控制GPU任务并行度
    if [[ $(jobs -r -p | wc -l) -ge $PARALLEL_JOBS ]]; then
        wait -n
    fi
done
wait

#合并成RuntimeAndError.json
cd /home/zjn/mytool4/src
g++ -DChromosome_Number="$N" /home/zjn/mytool4/src/merge_RuntimeAndError.cpp -o merge_RuntimeAndError
./merge_RuntimeAndError

echo "--------------------------------------------------执行完成---------------------------------------------------"
echo "详细日志查看: $LOG_DIR"
echo "结果汇总:"
cat $LOG_DIR/summary.log