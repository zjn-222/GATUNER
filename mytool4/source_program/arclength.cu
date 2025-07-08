#include <stdio.h>
#include <math.h>
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

#define N 1000000
// 修改为双精度常量
#define PI 3.14159265358979323846  // 移除L后缀
#define ANS 5.795776322412856      // 移除L后缀

// __device__ double fun(double x);
__device__ double fun(double x2) {
    double d2 = 1.0;
    double t3 = x2;
    
    for (int k = 1; k <= 5; ++k) {
        d2 =d2* 2.0;
        t3 =t3+ sin(d2 * x2) / d2;
    }
    return t3;
}

__global__ void compute_t(double* d_t, int num_points, double h1) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < num_points) {
        double x1;
        x1 = i * h1;
        d_t[i] = fun(x1);
    }
}

__global__ void compute_and_sum(const double* d_t, int num_segments, double h2, double* d_sum) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < num_segments) {
        double dt ;
        dt = d_t[i+1] - d_t[i];
        atomicAdd(d_sum, sqrt(h2*h2 + dt*dt));
    }
}



int main() {



    double h = PI / N;
    const int num_points = N + 1;
    const int num_segments = N;

    // 分配设备内存
    double *d_t = nullptr, *d_sum = nullptr;
    cudaMalloc(&d_t, num_points * sizeof(double));
    cudaMalloc(&d_sum, sizeof(double));
    cudaMemset(d_sum, 0, sizeof(double));

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    // 计算函数值
    const int block_size = 256;
    int grid_size = (num_points + block_size - 1) / block_size;
    compute_t<<<grid_size, block_size>>>(d_t, num_points, h);
    cudaDeviceSynchronize();

    // 计算并累加线段长度
    grid_size = (num_segments + block_size - 1) / block_size;
    compute_and_sum<<<grid_size, block_size>>>(d_t, num_segments, h, d_sum);
    cudaDeviceSynchronize();

    // 获取结果
    double s1 = 0;
    cudaMemcpy(&s1, d_sum, sizeof(double), cudaMemcpyDeviceToHost);

  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();

    // 修正printf格式说明符
    printf("Calculated value: %.15f\n", s1);
    printf("Expected value:    %.15f\n", ANS);  // ANS现在为双精度
    printf("Absolute error:    %e\n", fabs(ANS - s1));  // 结果转为双精度

    // 释放资源
    cudaFree(d_t);
    cudaFree(d_sum);


  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(fabs(ANS - s1));
  printf("绝对误差为：%.15e\n",program_error);
  printf("结果为：%.15e\n",s1);
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