#include <iostream>
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

double calculate(double xarg) {
    double result;
    result = exp(-xarg);
    return result;
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    int n = 1000000;
    double t_start;
    double  t_end;
    double h;
    double s1;
    double t;

    t_start = 0.0;
    t_end = 1.0;
    h = (t_end - t_start) / (2.0 * n);
    s1 = 0.0;

    t = t_start;
    s1 = (calculate(t_start) + calculate(t_end));

    for(int i = 0; i < n; i++) { // ITERS before
        t =t+ h;
        s1 = s1 + 4.0 * calculate(t);
        t = t + h;
        s1 = s1 + 2.0 * calculate(t);
    }
    s1 = s1 * h / 3.0;


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(s1)-std::abs(6.321206814586861e-01));
  printf("绝对误差为：%.12e\n",program_error);
    // printf("绝对误差为：%.15e\n",s1);
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