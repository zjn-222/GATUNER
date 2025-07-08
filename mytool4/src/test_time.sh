#!/bin/bash

# N=${N:-10} # 默认值保障
# PARALLEL_JOBS=${PARALLEL_JOBS:-4} 
# LOG_DIR="/home/zjn/mytool4/logs"


# # cd /home/zjn/mytool4/file
# # for i in $(seq 1 $N); do
# #     (
# #         # 记录开始时间
# #         start_time=$(date +%s)
        
# #         g++ /home/zjn/mytool4/file/chromosome${i}.cpp -o chromosome${i} -lm -O3 -std=c++17
        
# #         # 检查编译是否成功
# #         if [ $? -eq 0 ]; then
# #             # 运行并记录性能
# #             ./chromosome${i} 
# #             exit_code=$?
            
# #             # 计算耗时
# #             end_time=$(date +%s)
# #             duration=$((end_time - start_time))
            
# #             # 记录结果
# #             echo "chromosome${i} | 状态:$exit_code | 耗时666:${duration}s" 
# #         else
# #             echo "chromosome${i} | 编译失败" 
# #         fi
# #     ) &
    
# #     # 控制GPU任务并行度
# #     if [[ $(jobs -r -p | wc -l) -ge $PARALLEL_JOBS ]]; then
# #         wait -n
# #     fi
# # done
# # wait


# # 第三阶段：并行编译和执行
# # echo "并行编译和执行（共$N个）..."
# # cd /home/zjn/mytool4/file
# # for i in $(seq 1 $N); do
# #     (
# #         # 记录开始时间
# #         start_time=$(date +%s)
        
# #         # 编译
# #         # nvcc -arch=sm_86 chromosome${i}.cu -o chromosome${i} > $LOG_DIR/compile_${i}.log 2>&1
# #         # nvcc -arch=sm_86 chromosome${i}.cpp -o chromosome${i} -lm --expt-relaxed-constexpr > $LOG_DIR/compile_${i}.log 2>&1
# #         # 指定 C++17 标准 + O3 优化 + 调试符号
# #         g++ chromosome${i}.cpp -o chromosome${i} -lm -O3 -std=c++17 -g > $LOG_DIR/compile_${i}.log 2>&1
        
# #         # 检查编译是否成功
# #         if [ $? -eq 0 ]; then
# #             # 运行并记录性能
# #             ./chromosome${i} >> $LOG_DIR/run_${i}.log 2>&1
# #             exit_code=$?
            
# #             # 计算耗时
# #             end_time=$(date +%s)
# #             duration=$((end_time - start_time))
            
# #             # 记录结果
# #             echo "chromosome${i} | 状态:$exit_code | 耗时666:${duration}s" >> $LOG_DIR/summary.log
# #         else
# #             echo "chromosome${i} | 编译失败" >> $LOG_DIR/summary.log
# #         fi
# #     ) &
    
# #     # 控制GPU任务并行度
# #     if [[ $(jobs -r -p | wc -l) -ge $PARALLEL_JOBS ]]; then
# #         wait -n
# #     fi
# # done
# # wait

# # #合并成RuntimeAndError.json
# # cd /home/zjn/mytool4/src
# # g++ -DChromosome_Number="$N" /home/zjn/mytool4/src/merge_RuntimeAndError.cpp -o merge_RuntimeAndError
# # ./merge_RuntimeAndError




# # N=${N:-10}  # 染色体数量
# # PARALLEL_JOBS=${PARALLEL_JOBS:-4}  # 并行任务数
# # LOG_DIR="/home/zjn/mytool4/logs"

# # # 创建日志目录
# # mkdir -p $LOG_DIR

# # # 阶段1：批量编译
# # echo "编译阶段（共$N个）..."
# # cd /home/zjn/mytool4/file

# # declare -a COMPILE_STATUS  # 编译状态数组

# # # 并行编译所有染色体
# # for i in $(seq 1 $N); do
# #     (
# #         echo "开始编译 chromosome$i"
# #         g++ chromosome${i}.cpp -o chromosome${i} -lm -O3 -std=c++17 -g > $LOG_DIR/compile_${i}.log 2>&1
# #         if [ $? -eq 0 ]; then
# #             COMPILE_STATUS[$i]=1
# #             echo "成功编译 chromosome$i"
# #         else
# #             COMPILE_STATUS[$i]=0
# #             echo "编译失败 chromosome$i"
# #         fi
# #     ) &
    
# #     # 控制并行度
# #     if [[ $(jobs -r | wc -l) -ge $PARALLEL_JOBS ]]; then
# #         wait -n
# #     fi
# # done
# # wait

# # # 阶段2：批量执行
# # echo "执行阶段（共$N个）..."
# # for i in $(seq 1 $N); do
# #     if [[ ${COMPILE_STATUS[$i]} -eq 1 ]]; then
# #         (
# #             # 记录纯执行时间
# #             start_time=$(date +%s%3N)  # 毫秒级时间戳
# #             ./chromosome${i} >> $LOG_DIR/run_${i}.log 2>&1
# #             exit_code=$?
# #             end_time=$(date +%s%3N)
# #             duration=$((end_time - start_time))
            
# #             # 原子写入日志
# #             flock -x $LOG_DIR/summary.log -c "echo \"chromosome${i} | 状态:$exit_code | 耗时:${duration}ms\" >> $LOG_DIR/summary.log"
# #         ) &
        
# #         # 控制并行度
# #         if [[ $(jobs -r | wc -l) -ge $PARALLEL_JOBS ]]; then
# #             wait -n
# #         fi
# #     else
# #         echo "chromosome${i} | 编译失败" >> $LOG_DIR/summary.log
# #     fi
# # done
# # wait

# # # 合并结果（同原脚本）
# # cd /home/zjn/mytool4/src
# # g++ -DChromosome_Number="$N" /home/zjn/mytool4/src/merge_RuntimeAndError.cpp -o merge_RuntimeAndError
# # ./merge_RuntimeAndError



# N=${N:-10}                  # 染色体总数
# PARALLEL_COMPILE=4          # 编译并行度（根据CPU核心数设置）
# PARALLEL_EXEC=2             # 执行并行度（根据GPU/CPU资源设置）
# LOG_DIR="/home/zjn/mytool4/logs"

# # 创建日志目录
# mkdir -p "$LOG_DIR"

# # 阶段1：并行编译
# echo "[阶段1] 开始并行编译（共$N个，并行度$PARALLEL_COMPILE）..."
# cd /home/zjn/mytool4/file || exit 1

# # 使用数组跟踪编译结果
# declare -A compile_status
# for i in $(seq 1 "$N"); do
#     compile_status[$i]=0  # 0表示未编译，1成功，2失败
# done

# # 并行编译函数
# compile_task() {
#     local i=$1
#     echo "[编译] chromosome$i 开始" 
#     if g++ chromosome${i}.cpp -o chromosome${i} -lm -O3 -std=c++17 -g > "${LOG_DIR}/compile_${i}.log" 2>&1; then
#         echo "[编译] chromosome$i 成功"
#         compile_status[$i]=1
#     else
#         echo "[编译] chromosome$i 失败"
#         compile_status[$i]=2
#     fi
# }

# # 使用xargs实现并行编译池
# export -f compile_task
# export LOG_DIR N
# seq 1 "$N" | xargs -P "$PARALLEL_COMPILE" -I {} bash -c 'compile_task "$@"' _ {}

# # 阶段2：并行执行
# echo "[阶段2] 开始并行执行（并行度$PARALLEL_EXEC）..."
# timestamp=$(date +%Y%m%d-%H%M%S)  # 统一时间戳

# # 执行任务函数
# exec_task() {
#     local i=$1
#     if [[ ${compile_status[$i]} -eq 1 ]]; then
#         local start_time=$(date +%s%3N)  # 毫秒时间戳
#         echo "[执行] chromosome$i 开始于 $(date +'%T.%3N')"
        
#         # 执行并捕获退出码
#         ./chromosome"${i}" >> "${LOG_DIR}/run_${i}.log" 2>&1
#         local exit_code=$?
        
#         local end_time=$(date +%s%3N)
#         local duration=$((end_time - start_time))
        
#         # 原子化写入日志
#         echo "chromosome${i} | 状态:$exit_code | 耗时:${duration}ms | 批次:$timestamp" >> "${LOG_DIR}/summary_${timestamp}.log"
#         echo "[执行] chromosome$i 完成，耗时${duration}ms"
#     else
#         echo "chromosome${i} | 编译失败" >> "${LOG_DIR}/summary_${timestamp}.log"
#     fi
# }

# # 使用GNU Parallel控制执行并行度
# export -f exec_task
# seq 1 "$N" | parallel -j "$PARALLEL_EXEC" --joblog "${LOG_DIR}/job_${timestamp}.log" exec_task

# # 阶段3：结果合并
# echo "[阶段3] 合并结果..."
# cd /home/zjn/mytool4/src || exit 1
# g++ -DChromosome_Number="$N" /home/zjn/mytool4/src/merge_RuntimeAndError.cpp -o merge_RuntimeAndError
# ./merge_RuntimeAndError

# 定义输出文件名
output_file="/home/zjn/mytool4/file/output.log"

# # 清空旧日志文件（如果不想清空，可以删除这行）
# > "$output_file"

# # 循环执行 100 次
# for i in {1..100}
# do
#   echo "==== 第 $i 次执行 ====" >> "$output_file"
#   ..//file/chromosome2 >> "$output_file" 2>&1    # 执行程序并追加输出
  
#   # 记录执行状态（可选）
#   exit_code=$?
#   echo "状态码: $exit_code" >> "$output_file"
#   echo >> "$output_file"  # 添加空行分隔
# done

# echo "执行完成，结果已保存到 $output_file"

# output_file="results.log"
total=4

echo "开始执行 $total 次任务..."
for ((i=1; i<=total; i++)); do
    # 执行程序并记录
    ../file/a >> "$output_file" 2>&1
    
    # 显示进度（可选）
    printf "\r已完成: %3d/%d" $i $total
    
    # # 添加延迟
    sleep 0.02
done
echo -e "\n所有任务执行完成"