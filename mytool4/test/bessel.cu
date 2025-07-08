#include <stdio.h>
#include <cmath>
#include <cuda_runtime.h>
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


// CUDA 设备端的贝塞尔函数计算
__device__ double bessel_J0(double x1) {
    const double EPS = 1e-10;
    const int MAXIT = 10000;
    
    double sum = 0.0;
    double term = 1.0;

    for (int k = 0; fabs(term) > EPS && k < MAXIT; ++k) {
        term = pow(-1, k) * pow(x1 / 2.0, 2 * k) / (tgamma(k + 1) * tgamma(k + 1));
        sum = sum + term;
    }
    return sum;
}

// CUDA 核函数
__global__ void bessel_kernel(double* d_x, double* d_result) {
    *d_result = bessel_J0(*d_x);
}

int main() {
    double x = 1.3456;
    double result;
    
    // 分配设备内存
    double *d_x, *d_result;
    cudaMalloc((void**)&d_x, sizeof(double));
    cudaMalloc((void**)&d_result, sizeof(double));

    // 拷贝数据到设备
    cudaMemcpy(d_x, &x, sizeof(double), cudaMemcpyHostToDevice);
    
  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    // 启动核函数（使用单个线程块和单个线程）
    bessel_kernel<<<1, 1>>>(d_x, d_result);

    // 等待设备计算完成
    cudaDeviceSynchronize();


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();


    // 拷贝结果回主机
    cudaMemcpy(&result, d_result, sizeof(double), cudaMemcpyDeviceToHost);

    // 输出结果（添加了输出语句）
    printf("J0(%.4f) = %.15f\n", x, result);

    // 释放设备内存
    cudaFree(d_x);
    cudaFree(d_result);

  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(result)-abs(0.596060578305021));
  printf("绝对误差为：%.15e\n",program_error);
  printf("结果为：%.15e\n",result);
  //并行版
  //每个chromosome个体独立写入数据到/home/zjn/mytool4/josn/chromosome1.json，最终再合并成RuntimeAndError.json
  json data;
  // 更新当前染色体的数据
  data["chromosome1"]["runtime"] = duration.count(); // 此处键名会被 sed 动态替换
  data["chromosome1"]["error"] = program_error;
  std::string json_path="/home/zjn/mytool4/json/";
  json_path+="chromosome1";
  json_path+=".json";
  // 写回文件
  std::ofstream file(json_path);
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