#include <iostream>
#include <cmath>
#include <cstdio>

//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;


int N = 100000;

double compute_doppler(double t2) {
    double result;
    result=sin(t2) / (1 + t2 * t2);
    return result;
}



int main() {

      // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    double t0 = 0.0;
    double t1 = 10.0;
    double dt = (t1 - t0) / N;
    double sum = 0.0;

    for (int i = 0; i < N; ++i) {
        double t;
        double doppler_value;
        t = t0 + i * dt;
        doppler_value = compute_doppler(t);
        sum =sum + doppler_value * dt;
    }


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(sum)-std::abs(6.556190561865509e-01));
  printf("绝对误差为：%.12e\n",program_error);
    // printf("绝对误差为：%.15e\n",sum);
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