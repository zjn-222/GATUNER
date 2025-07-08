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


// 固定的数学常量pi
const double PI = std::acos(-1);



// 计算函数在x处的导数值
double df(double x) {
    return cos(x);
}

// 计算弧长的梯形规则近似
double arcLengthTrapezoidalRule(double a, double b, int n) {
    double h = (b - a) / n;  // 分割区间的步长
    double arcLength = 0.0; 
    double x;
    double fx1;
    double fx2;
    double temp1;
    double temp2;
    n=100000;
    for (int i = 0; i < n; ++i) {
        x = a + i * h;            // 当前区间的左端点
        temp1=cos(x);
        temp2=cos(x+h);
        fx1 = sqrt(1 + temp1);         // 当前区间的左端点处的函数值
        fx2 = sqrt(1 + temp2 ); // 当前区间的右端点处的函数值
        arcLength =arcLength+(fx1 + fx2) * h / 2;           // 梯形面积
    }

    return arcLength;
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    double a = 0.0;  // 积分的起点
    double b = PI;   // 积分的终点
    int n = 100000;    // 初始分割数量

    double arcLength = arcLengthTrapezoidalRule(a, b, n);

      // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();


    

  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(arcLength)-std::abs(2.828427124688e+00));
  printf("绝对误差为：%.12e\n",program_error);
    printf("结果为：%.12e\n",arcLength);
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




}