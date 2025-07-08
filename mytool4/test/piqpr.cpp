#include <iostream>
#include <iomanip>
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


const int N = 30000000; // 0.3 * 10^8

double bbp_term(int n, int k) {
    double term = 0.0;
    double denom = 8 * k + n;
    term =term+ 4.0 / denom;
    term =term- 2.0 / (denom + 2);
    term =term- 1.0 / (denom + 4);
    term =term- 1.0 / (denom + 6);
    term =term/ pow(16.0, k);
    return term;
}

double bbp_pi(int n) {
    double sum = 0.0;
    for (int k = 0; k <= n; ++k) {
        sum =sum+ bbp_term(1, k);
    }
    sum = fmod(sum, 1.0);
    return sum;
}
int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    double result=bbp_pi(N);


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(result)-std::abs(0.9980729228592482));
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