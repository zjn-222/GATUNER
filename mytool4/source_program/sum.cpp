
#include <cmath>  // 包含math库以支持数学函数

//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;



// 计算数组元素之和的函数
double sum(double* data, size_t n) {
    double result = 0.0;  // 结果变量
    for (size_t i = 0; i < n; ++i) {  // 使用for循环遍历数组
        result += data[i];  // 累加每个元素
    }
    return result;  // 返回结果
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

    size_t n = 5;  // 数组大小
    double data1 = 1.1234522345353457;  // 数据元素1
    double data2 = 2.2657337452524567;  // 数据元素2
    double data3 = 3.345756524265246;  // 数据元素3
    double data4 = 4.4534645376452435427;  // 数据元素4
    double data5 = 5.55647426235462345;  // 数据元素5

      // 使用sum函数计算数组元素之和
    double data_array[] = {data1, data2, data3, data4, data5};
    
    double result1 = sum(data_array, n);


      // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();

  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(result1)-std::abs(16.7448813040529));
  printf("绝对误差为：%.12e\n",program_error);
  printf("结果为：%.12e\n",result1);
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
  data["chromosome1"]["runtime"] = duration.count(); // 此处键名会被 sed 动态替换
  data["chromosome1"]["error"] = program_error;
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

