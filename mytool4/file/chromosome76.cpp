#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;



#define PI 3.14159265358979323846

#define N 5000

typedef unsigned long int uint_t;

double in_real[N];
double in_imag[N];
double out_real[N];
double out_imag[N];

void dft()
{
    double W_real[N];
    double W_imag[N];
    float arg = 2.0*PI / (double)N;        // NOTE: added arg calculation
                                            //       (not in paper)

    /* Generation of coefficients W */
    for (uint_t i = 0; i < N; i++) {        // NOTE: fixed typo in paper here
        W_real[i] =  cosf(arg * (double)i);
        W_imag[i] = -sinf(arg * (double)i);
    }

    /* The main computation kernel */
    for (uint_t k = 0; k < N; k++) {        // Outer loop
        out_real[k] = in_real[k];           // NOTE: added array references
        out_imag[k] = in_imag[k];           //       (were scalar in paper)
        for (uint_t n = 0; n < N; n++) {    // Inner loop
            uint_t p = (n * k) % N;
            out_real[k] = out_real[k] + in_real[n] * W_real[p] - in_imag[n] * W_imag[p];
            out_imag[k] = out_imag[k] + in_real[n] * W_imag[p] + in_imag[n] * W_real[p];
        }
    }
}

double sgn(const double x)
{
    // return -1 if x is negative, 0 if it is zero and 1 if it is positive
    // NOTE: this is not differentiable; it is only used to generate inputs!
    float flag;
    if (x>0.0) flag=1;
    else if (x<0.0) flag=-1;
    else flag=0;
    // return ((x > 0.0) - (x < 0.0));
    return flag;
}


int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

 
     /* Generate inputs */
    for (uint_t i = 0; i < N; i++) {
        // scale i to [0,2*pi)
        double x = ((2.0*PI) / (double)N) * (double)i;

        // minor variation on a square wave (the classic FFT encoding example)
        in_real[i] = sgn(sinf(x+1.0))+cosf(x);
        in_imag[i] = sgn(cosf(x+1.0))+sinf(x);
    }


    /* Run discrete fourier transform routine */
    dft();

    /* Calculate and print output norm */
    double norm = 0.0;
    for (uint_t i = 0; i < N; i++) {
        norm += out_real[i] * out_real[i];
    }
    norm = sqrtf(norm);

  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs((double)norm)-std::abs(7.541410678987824e+03));
  printf("绝对误差为：%.12e\n",program_error);
    // printf("绝对误差为：%.15e\n",(double)norm);
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
  data["chromosome76"]["runtime"] = duration.count(); // 此处键名会被 sed 动态替换
  data["chromosome76"]["error"] = program_error;
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






