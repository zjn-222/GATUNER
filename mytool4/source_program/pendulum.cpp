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



// 全局常量定义[7](@ref)
const double g = 9.8;        // 重力加速度
const double PI = M_PI;       // π常量

// 单摆状态结构体[3,5](@ref)
struct PendulumState {
    double length;   // 摆长(m)
    double theta;    // 当前角度(rad)
    double omega;    // 角速度(rad/s)
};

// 微分方程计算函数[7](@ref)
double calculateAcceleration(double theta1, double length1) {
    return -g / length1 * sin(theta1);
}

// 四阶龙格-库塔法更新状态[2,7](@ref)
void updateState(PendulumState* state, double dt) {
    // 第一阶段计算
    double k1_theta ;
    k1_theta= state->omega;
    double k1_omega;
    k1_omega = calculateAcceleration(state->theta, state->length);

    // 第二阶段计算
    double tempTheta;
    tempTheta = state->theta + 0.5 * dt * k1_theta;
    double tempOmega;
    tempOmega = state->omega + 0.5 * dt * k1_omega;
    double k2_omega;
    k2_omega = calculateAcceleration(tempTheta, state->length);

    // 第三阶段计算
    tempTheta = state->theta + 0.5 * dt * tempOmega;
    tempOmega = state->omega + 0.5 * dt * k2_omega;
    double k3_omega ;
    k3_omega = calculateAcceleration(tempTheta, state->length);

    // 第四阶段计算
    tempTheta = state->theta + dt * tempOmega;
    tempOmega = state->omega + dt * k3_omega;
    double k4_omega ;
    k4_omega = calculateAcceleration(tempTheta, state->length);

    // 合并计算结果
    state->theta += dt * (k1_theta + 2*(tempOmega + tempOmega) + k4_omega) / 6;
    state->omega += dt * (k1_omega + 2*(k2_omega + k3_omega) + k4_omega) / 6;

    // 角度归一化处理[7](@ref)
    while(state->theta > PI) state->theta -= 2*PI;
    while(state->theta < -PI) state->theta += 2*PI;
}

// 打印当前状态[3](@ref)
double printState(const PendulumState& state, double t) {
    printf("t=%.2fs, θ=%.4f rad (%.1f°), ω=%.4f rad/s\n",
           t, state.theta, state.theta*180/PI, state.omega);
    return state.omega;
}

// 初始化单摆状态[5](@ref)
PendulumState createPendulum(double length, double initTheta) {
    return {length, initTheta, 0.0};  // 初始角速度为0
}

int main() {

  // 获取开始时间
  auto start = std::chrono::high_resolution_clock::now();
    
    // 模拟参数
    const double dt = 0.01;       // 时间步长[7](@ref)
    const int totalSteps = 1000;   // 总步数
    double res;
    
    // 初始化单摆[7](@ref)
    PendulumState pendulum = createPendulum(1.0, PI/6);  // 1m摆长，初始30度
    double currentTime;
    // 运动模拟循环
    for(int step = 0; step < totalSteps; ++step) {
        currentTime = step * dt;
        
        // 输出500步（5秒时）的状态
        if(step == 500) {
            res=printState(pendulum, currentTime);
        }

        // 更新物理状态
        updateState(&pendulum, dt);
    }
    printf("结果为：%.15e\n",res);


  // 获取结束时间
  auto end = std::chrono::high_resolution_clock::now();
  // 计算并输出执行时间
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
  double program_error=abs(abs(res)-abs(2.493313038524778e-02));
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