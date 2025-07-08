//全Fp64
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <math.h>
#include <stdio.h>


//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;

// typedef half fp16; // CUDA半精度类型
// typedef float fp32;
// typedef double fp64;

// 设备端全局变量
__device__ double pi;        // 存储π值
__device__ double s1_global; // 存储中间计算结果

// 设备函数
__device__ double fun(double xarg) {
    return sin(pi * xarg);
}

// 初始化π值的核函数
__global__ void compute_pi_kernel() {
    pi = acos(-1.0f);
}

// 初始化中间结果的核函数
__global__ void init_s1_kernel() {
    s1_global = 0.0;
}

// 积分计算核函数（去除了指针参数）
__global__ void integrate_kernel(double a, double b, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= 2 * n) return;

    double h1;
    h1 = (b - a) / (2.0 * n);
    double x ;
    x= idx * h1;

    double val ;
    val= (idx % 2 == 0) ? (4.0 * fun(x)) : (2.0 * fun(x));
    atomicAdd(&s1_global, val);
}

// 最终计算核函数（去除了指针参数）
__global__ void finalize_kernel(double a, double b, int n) {
    double h2;
    h2 = (b - a) / (2.0 * n);
    s1_global = s1_global * h2 * pi / 3.0;
}

int main() {
    const int n = 1000000;
    double a = 0.0;
    double b = 1.0;
    double ss1 = 0.0;

    // 初始化设备端变量
    init_s1_kernel<<<1, 1>>>();
    compute_pi_kernel<<<1, 1>>>();
    cudaDeviceSynchronize();

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


     // 执行积分计算
    dim3 block(256);
    dim3 grid((2 * n + block.x - 1) / block.x);
    integrate_kernel<<<grid, block>>>(a, b, n);
    
    // 执行最终计算
    finalize_kernel<<<1, 1>>>(a, b, n);
    
    // 拷贝结果回主机
    cudaMemcpyFromSymbol(&ss1, s1_global, sizeof(double));
  
  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(ss1)-abs(1.999999999999244e+00));
  printf("绝对误差为：%.15e\n",program_error);
  printf("绝对误差为：%.15e\n",ss1);
  json data;
  // 读取现有 JSON 文件
  std::ifstream inFile("/home/zjn/mytool4/src/RuntimeAndError.json");
  if (inFile.good()) {
      try {
          data = json::parse(inFile);
      } catch (json::parse_error& e) {
          std::cerr << "JSON 解析错误，创建新文件。错误信息: " << e.what() << std::endl;
      }
  }
  inFile.close();
  // 更新当前染色体的数据
  data["chromosome1"]["runtime"] = duration.count(); // 此处键名会被 sed 动态替换
  data["chromosome1"]["error"] = program_error;
  // 写回文件
  std::ofstream file("/home/zjn/mytool4/src/RuntimeAndError.json");
  std::cout << "RuntimeAndError.json更新完成"<< std::endl;
  if (file.is_open()) {
      file << data.dump(4);
      file.close();
  } else {
      std::cerr << "无法写入文件！" << std::endl;
      return 1;
  }



    return 0;
}