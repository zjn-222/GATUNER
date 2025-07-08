#include <stdio.h>
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

/* macros */
#define ABS(x) ( ((x) < 0.0) ? (-(x)) : (x) )

/* constants */
#define PI     3.1415926535897932384626433832795
#define EPS    5e-7

/* loop  iterations; OUTER is X */
#define INNER    25
#define OUTER    2000

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();

 
    double sum = 0.0;
    double tmp;
    double acc;
    int i, j;

    for (i=0; i<OUTER; i++) {
        acc = 0.0;
        for (j=1; j<INNER; j++) {

            /* accumulatively calculate pi */
            tmp = PI / pow(2.0, j);
            acc = acc + tmp;
        }
        sum = sum + acc;
    }

    // double answer = (double)OUTER * PI;             /* correct answer */
    // double diff = (double)answer-(double)sum;
    // double error = ABS(diff);
    //  printf("为：%.12e\n",EPS*answer);

    // if ((double)error < (double)EPS*answer) {
    //     printf("SUM2PI_X - SUCCESSFUL!\n");
    // } else {
    //     printf("SUM2PI_X - FAILED!!!\n");
    // }


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=std::abs(std::abs(sum)-std::abs(6.283184932672303e+03));
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