#include <cmath>
#include <iostream>
//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;


// Gaussian函数定义
double gaussian(float x, float mean, float stddev) {
    float variance = powf(stddev, 2);
    float coeff = 1.0 / (sqrt(2.0 * M_PI * variance));
    float exponent = -powf(x - mean, 2) / (2.0 * variance);
    return coeff * exp(exponent);
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    double x = 0.0; // 计算点
    double mean = 0.0; // 高斯分布的均值
    double stddev = 1.0; // 高斯分布的标准差

    double result = gaussian(x, mean, stddev);

  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(result)-std::abs(0.398942280401433));
  printf("绝对误差为：%.12e\n",program_error);
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