#include <iostream>
#include <cmath>
#include <vector>




//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
//json库
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;






using namespace std;

// 交流瞬时电流计算（带相位差）
struct ACParameters {
    double voltage_rms;   // 电压有效值
    double current_rms;   // 电流有效值
    double frequency;     // 频率(Hz)
    double phase_angle;   // 相位差(rad)
};

vector<double> generate_current_wave(const ACParameters& params, double duration, int samples) 
{
    vector<double> waveform;
    double omega ;
    omega= 2 * M_PI * params.frequency;
    
    for(int i=0; i<samples; ++i){
        double t;
        t = duration * i / samples;
        double current;
        current = sqrt(2) * params.current_rms * sin(omega * t + params.phase_angle);
        waveform.push_back(current);
    }
    return waveform;
}

int main()
{


  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();


    // 示例：220V/50Hz交流电路，5A电流，滞后30度
    ACParameters circuit {
        .voltage_rms = 220,
        .current_rms = 5,
        .frequency = 50,
        .phase_angle = -M_PI/6  // 滞后30度
    };

    auto current_wave = generate_current_wave(circuit, 0.1, 1000);
    
    // 输出前10个采样点
    // for(int i=0; i<10; ++i){
    //     cout << "t=" << 0.0001*i << "s: " 
    //          << current_wave[i] << "A" << endl;
    // }
    printf("结果为:%.15e\n",current_wave[0]);




  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(current_wave[0])-abs(3.535533905932737e+00));
  printf("绝对误差为：%.12e\n",program_error);
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




    return 0;
}