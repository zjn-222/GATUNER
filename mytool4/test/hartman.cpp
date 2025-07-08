// #include <iostream>
// #include <cmath>
// using namespace std;

// // Hartman三维函数实现
// double hartman3D(double x[3]) {
//     // 参数配置 (来自标准测试函数定义)
//     double alpha[4] = {1.0, 1.2, 3.0, 3.2};
    
//     double A[4][3] = {
//         {3.0, 10.0, 30.0},
//         {0.1, 10.0, 35.0},
//         {3.0, 10.0, 30.0},
//         {0.1, 10.0, 35.0}
//     };
    
//     double P[4][3] = {
//         {0.3689, 0.1170, 0.2673},
//         {0.4699, 0.4387, 0.7470},
//         {0.1091, 0.8732, 0.5547},
//         {0.0381, 0.5743, 0.8828}
//     };

//     double sum = 0.0;
//     for(int i=0; i<4; ++i) {
//         double exponent = 0.0;
//         for(int j=0; j<3; ++j) {
//             exponent += A[i][j] * pow(x[j] - P[i][j], 2);
//         }
//         sum += alpha[i] * exp(-exponent);
//     }
//     return -sum;  // 转换为最小值问题
// }



// int main() {
//     // 示例输入测试
//     double test_point1[3] = {0.5, 0.5, 0.5};
//     cout << "测试点(0.5, 0.5, 0.5)结果: " 
//          << hartman3D(test_point1) << endl;

//     // 验证最小值
//     // 已知最小值点 (近似值)
//     double min_point[3] = {0.114614, 0.555649, 0.852547};
    
//     double result = hartman3D(min_point);
//     printf("理论最小值点计算结果:%.15e\n",result);
//     cout << "预期理论最小值: -3.86278" << endl;
//     printf("实际误差:%.15e\n",fabs(result + 3.862779786949337e+00));
    
//     return 0;
// }



#include <iostream>
#include <cmath>
using namespace std;


//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;





// 将三维坐标分解为独立变量，参数矩阵展开为命名常量
double hartman3D(double x0, double x1, double x2) {
    // Alpha参数定义
    double alpha0 ;
    alpha0= 1.0;
    double alpha1 ;
    alpha1= 1.2;
    double alpha2;
    alpha2 = 3.0;
    double alpha3 ;
    alpha3= 3.2;

    // A矩阵参数展开（4x3矩阵）
    double A_0_0 ;
   A_0_0 = 3.0;
    double A_0_1 ;
  A_0_1  = 10.0;
    double A_0_2 ;
  A_0_2  = 30.0;
    double A_1_0 ;
   A_1_0 = 0.1;
    double A_1_1;
    A_1_1 = 10.0; 
    double A_1_2;
    A_1_2 = 35.0;
    double A_2_0 ;
   A_2_0 = 3.0;
    double A_2_1 ;
    A_2_1= 10.0;
    double  A_2_2 ;
    A_2_2= 30.0;
    double A_3_0 ;
   A_3_0 = 0.1;
    double  A_3_1;
    A_3_1 = 10.0;
    double A_3_2 ;
    A_3_2= 35.0;

    // P矩阵参数展开（4x3矩阵）
    double P_0_0 = 0.3689;
    double P_0_1 = 0.1170;
    double  P_0_2 = 0.2673;
    double P_1_0 = 0.4699;
    double P_1_1 = 0.4387;
    double P_1_2 = 0.7470;
    double P_2_0 = 0.1091;
    double P_2_1 = 0.8732;
    double  P_2_2 = 0.5547;
    double P_3_0 = 0.0381;
    double P_3_1 = 0.5743;
    double  P_3_2 = 0.8828;

    // 展开循环计算四个项
    double exponent0;
    exponent0 = A_0_0*pow(x0-P_0_0,2) + 
                      A_0_1*pow(x1-P_0_1,2) + 
                      A_0_2*pow(x2-P_0_2,2);
    
    double exponent1 ;
    exponent1 = A_1_0*pow(x0-P_1_0,2) + 
                      A_1_1*pow(x1-P_1_1,2) + 
                      A_1_2*pow(x2-P_1_2,2);
    
    double exponent2;
    exponent2 = A_2_0*pow(x0-P_2_0,2) + 
                      A_2_1*pow(x1-P_2_1,2) + 
                      A_2_2*pow(x2-P_2_2,2);
    
    double exponent3;
    exponent3 = A_3_0*pow(x0-P_3_0,2) + 
                      A_3_1*pow(x1-P_3_1,2) + 
                      A_3_2*pow(x2-P_3_2,2);

    // 累加四项计算结果
    double sum ;
    sum = alpha0 * exp(-exponent0)
               + alpha1 * exp(-exponent1)
               + alpha2 * exp(-exponent2)
               + alpha3 * exp(-exponent3);
    
    return -sum;  // 保持原问题的最小值转换
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    // 测试点分解为独立变量
    double test_x0 ;
    test_x0= 0.5;
    double test_x1;
    test_x1 = 0.5;
    double test_x2;
    test_x2 = 0.5;
    cout << "测试点(0.5, 0.5, 0.5)结果: " 
         << hartman3D(test_x0, test_x1, test_x2) << endl;

    // 最小值点分解为独立变量
    double min_x0 ;
    min_x0 = 0.114614; 
    double min_x1 ;
    min_x1 = 0.555649;
    double min_x2;
    min_x2 = 0.852547;
    double result;
     result = hartman3D(min_x0, min_x1, min_x2);


   // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(result)-abs(3.862779786949337e+00));
  printf("绝对误差为：%.15e\n",program_error);
  //并行版
  //每个chromosome个体独立写入数据到/home/zjn/mytool4/josn/chromosome1.json，最终再合并成RuntimeAndError.json
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




    printf("理论最小值点计算结果:%.15e\n", result);
    cout << "预期理论最小值: -3.86278" << endl;
    printf("实际误差:%.15e\n", fabs(result + 3.862779786949337e+00));
    
    return 0;
}