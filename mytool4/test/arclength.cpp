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

#define ABS(x) ( ((x) < 0.0) ? (-(x)) : (x) )

#define PI  3.1415926535897932L
#define ANS 5.795776322412856L
#define EPS 1e-10

#define N 1000000

double h  = PI / (double)N;     // double

double t1 = 0.0;
double t2;
double t3;                      // double

double s1 = 0.0;                // double

double d1 = 1.0;
double d2;                      // float

double fun (double x)
{
    d2 = d1;    // also d1 in original
    t3 = x;     // also t1 in original

    int k;
    for (k = 1; k <= 5; k+=1)
    {
        d2 = 2.0 * d2;
        t3 = t3 + sin (d2 * x) / d2;
    }
    return t3;
}

void do_fun ()
{
    int i;
    for (i = 1; i <= N; i+=1)
    {
        t2 = fun (i * h);
        s1 = s1 + sqrt (h * h + (t2 - t1) * (t2 - t1));
        t1 = t2;
    }
}

int main (int argc, char **argv)
{
  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    do_fun();
    double error = ABS((double)ANS - (double)s1);


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(s1)-abs(5.795776322413e+00));
  printf("绝对误差为：%.12e\n",program_error);
  printf("结果为：%.12e\n",s1);
  
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
