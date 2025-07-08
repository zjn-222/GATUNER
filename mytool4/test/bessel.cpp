#include <stdio.h>
#include <cmath>

//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;



// 定义贝塞尔函数 J0 的实现逻辑
double bessel_J0(double x) {
    double EPS = 1e-10;
    int MAXIT = 10000;
    double P1 = 1.0;
    float P2 = -0.25;
    float P3 = 0.046875;
    float P4 = -0.00390625;
    float P5 = 0.000325520833333;
    float P6 = -0.000010850694444;

    float sum = 0.0;
    double term = 1.0;
    int k = 0;

    MAXIT=10000;
    for (int k = 0; fabs(term) > EPS && k < MAXIT; ++k) {
        term = pow(-1, k) * pow(x / 2.0, 2 * k) / (tgammaf(k + 1) * tgammaf(k + 1));
        sum += term;
    }

    return sum;
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    double x1;
    double result ;
    
    x1=1.3456;
    result= bessel_J0(x1);
    


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(result)-std::abs(0.5960605783050211));
  printf("绝对误差为：%.12e\n",program_error);
    printf("结果为：%.12e\n",result);
  json data;
  // 读取现有 JSON 文件
//   std::ifstream inFile("/home/zjn/mytool4/src/RuntimeAndError.json");
//   if (inFile.good()) {
//       try {
//           data = json::parse(inFile);
//       } catch (json::parse_error& e) {
//           std::cerr << "JSON 解析错误，创建新文件。错误信息: " << e.what() << std::endl;
//       }
//   }
//   inFile.close();
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