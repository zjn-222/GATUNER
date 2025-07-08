#include <cmath>
#include <complex>
#include <iostream>

#include <iostream>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iomanip>


//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;
#include <cmath>


const double PI = acos(-1.0);

void fft(int n, int step, std::complex<double>* a) {
    if (step < n) {
        fft(n, step * 2, a);
        fft(n, step * 2, a + step);
        for (int i = 0; i < n; i += 2 * step) {
            std::complex<double> t = std::polar(1.0, -PI * i / n) * a[i + step];
            a[i + step] = a[i] - t;
            a[i] += t;
        }
    }
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    const int N = 8;
    std::complex<double> data1[N];

    // 初始化输入数据
    for (int i = 0; i < N; ++i) {
        double real_part = cos(2 * PI * i / N);
        double imag_part = sin(2 * PI * i / N);
        data1[i] = std::complex<double>(real_part, imag_part);
    }


    // 执行FFT
    fft(N, 1, data1);


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(data1[0])-std::abs(-3.44509e-16));
  printf("绝对误差为：%.12e\n",program_error);
 // 方案2：改用C++流输出（自动处理复数格式）
std::cout << "结果为：" << data1[0] << std::endl; // 需包含<iostream>
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