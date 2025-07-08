#!/bin/bash
# 使用方法：./run.sh 10 ，其中10为指定的chromosome染色体数量

# 从config.json------>RuntimeAndError.json
#并行版本！！！！CPU和GPU并行
# 注意！！！修复了run3.sh的“并行执行有问题”的问题


# 参数检查与初始化
if [ $# -ne 1 ]; then
    echo "错误：必须指定染色体数量参数"
    echo "示例: ./run.sh 100"
    exit 1
fi

N=$1
# PARALLEL_COMPILE=$(nproc)    # 自动获取CPU核心数[4,8](@ref)
PARALLEL_COMPILE=1000
PARALLEL_EXEC=100             # 根据GPU显存调整
LOG_DIR="/home/zjn/mytool4/logs"
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

# 第一阶段：生成配置与清理工作（保持原逻辑）
export LOG_DIR="/home/zjn/mytool4/logs"
export TIMESTAMP=$(date +%Y%m%d-%H%M%S)
mkdir -p $LOG_DIR
echo "生成合并配置..."
cd /home/zjn/mytool4/src/
g++ -std=c++17 merge.cpp -o merge
./merge > $LOG_DIR/merge.log 2>&1

# 清理工作目录（保持原逻辑）
rm -f /home/zjn/mytool4/file/* /home/zjn/mytool4/logs/* /home/zjn/mytool4/json/*

# 第二阶段：并行生成染色体文件（优化为GNU Parallel）
echo "并行生成染色体文件（共$N个）..."
cd /home/zjn/mytool4/src/
seq 1 $N | parallel -j $PARALLEL_COMPILE --joblog $LOG_DIR/generate_$TIMESTAMP.log \
    '/home/zjn/llvm-project-main/build/bin/loop-convert21 --chromosome={} /home/zjn/mytool4/source_program/dft.cpp -- >> /home/zjn/mytool4/file/chromosome{}.cpp 2>/dev/null && sed -i "s/\"chromosome1\"/\"chromosome{}\"/g" /home/zjn/mytool4/file/chromosome{}.cpp'

# 第三阶段：并行编译（仅记录失败染色体）
echo "--------------------------------------------------编译阶段---------------------------------------------------"
compile_task() {
    i=$1
    if ! g++ /home/zjn/mytool4/file/chromosome${i}.cpp -o /home/zjn/mytool4/file/chromosome${i} -lm -O0 -std=c++17 -g >/dev/null 2>&1; then
        echo "染色体 $i 编译失败" >> "${LOG_DIR}/compile_errors.log"
    fi
}
export -f compile_task

cd /home/zjn/mytool4/file/
seq 1 "$N" | parallel --env LOG_DIR -j "$PARALLEL_COMPILE" --joblog "${LOG_DIR}/compile_${TIMESTAMP}.log" compile_task {}

# # 第四阶段：并行执行（独立阶段）
# echo "--------------------------------------------------执行阶段---------------------------------------------------"
# exec_task() {
#     i=$1
#     start=$(date +%s%3N)
#     ./chromosome${i} >> $LOG_DIR/run_${i}.log 2>&1
#     exit_code=$?
#     duration=$(( $(date +%s%3N) - start ))
#     echo "chromosome${i} | 状态:$exit_code | 耗时:${duration}ms" | tee -a $LOG_DIR/summary_$TIMESTAMP.log
# }
# export -f exec_task
# cd /home/zjn/mytool4/file/
# find . -name "*.success" -printf "%f\n" | \
#     sed -E 's/chromosome([0-9]+)\.success/\1/' | \
#     parallel -j $PARALLEL_EXEC --joblog $LOG_DIR/exec_$TIMESTAMP.log exec_task {} :::: -



# 第四阶段：串行执行
echo "--------------------------------------------------执行阶段---------------------------------------------------"
for i in $(seq 1 $N); do
    ./chromosome${i}
    sleep 0.2
done



# 最终结果合并与输出
cd /home/zjn/mytool4/src
g++ -DChromosome_Number="$N" merge_RuntimeAndError.cpp -o merge_RuntimeAndError
./merge_RuntimeAndError > "${LOG_DIR}/summary.log" 2>&1

echo "--------------------------------------------------执行完成---------------------------------------------------"
echo "编译失败染色体："
[ -f "${LOG_DIR}/compile_errors.log" ] && cat "${LOG_DIR}/compile_errors.log" || echo "所有染色体编译成功"
echo "结果汇总："
cat "${LOG_DIR}/summary.log"