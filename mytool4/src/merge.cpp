// 将Config.json与变量名称列表对应起来，生成新的ConfigMerged.json文件

#include <fstream>
#include <iostream>


#include <iostream>
#include <fstream>
#include "/usr/include/nlohmann/json.hpp" // 假设使用 nlohmann/json 库处理 JSON
#include <cstdlib>            // 用于 system 函数
#include <iomanip>


using json = nlohmann::json;
using namespace std;

int main() {
    // 第一步：读取 JSON 文件的结果，获取所有精度配置的数据
    ifstream i("/home/zjn/mytool4/src/Config.json");
    json config_json;
    i >> config_json;
    i.close();

    //变量名称列表
    // std::vector<std::string> keys={"result","sin","pi","xarg","a","b","h","x","s1","acos"}; //simpsons
    // std::vector<std::string> keys={"h","t1","t2","t3","s1","d1","d2","x","sin", "sqrt"}; //arclength
    // std::vector<std::string> keys={"term","denom","sum","pow","fmod"}; //piqpr无result
    // std::vector<std::string> keys={"term","denom","sum","pow","fmod","result"}; //piqpr
    // std::vector<std::string> keys={"PI","imag_part","real_part","sin","cos"}; //fft
    // std::vector<std::string> keys={"x","mean","stddev","variance","coeff","exponent","result","pow","sqrt","exp"}; //gaussian
    // std::vector<std::string> keys={"result","data1","data2","data3","data4","data5","result1"}; //sum
    // std::vector<std::string> keys={"P1","P2","P3","P4","P5","P6","sum","term","tgamma","pow"}; //bessel
    // std::vector<std::string> keys={"h","arcLength","x","fx1","fx2","a","b","sqrt","cos","temp1","temp2"}; //k-means
    // std::vector<std::string> keys={"sum","temp","acc","pow"}; //sum2pi_x
    std::vector<std::string> keys={"in_real","in_imag","out_real","out_imag", "W_real", "W_imag","arg","flag","x","norm","sin","cos","sqrt"}; //dft
    // std::vector<std::string> keys={"xarg","result","t_start","t_end", "h", "s1","t","exp"}; //carbonGas
    // std::vector<std::string> keys={"t1","result","t0","dt", "sum", "t","cos"}; //dlopper1
    // std::vector<std::string> keys={"t1","result","t0","dt", "sum", "t","cos"}; //dlopper2
    // std::vector<std::string> keys={"t2","t1","result","t0","dt", "sum", "t","sin","doppler_value"}; //dlopper3
    // std::vector<std::string> keys={"x0","x1","x2","alpha0","alpha1","alpha2","alpha3","A_0_0","A_0_1","A_0_2",
    // "A_1_0","A_1_1","A_1_2","A_2_0","A_2_1","A_2_2","A_3_0","A_3_1","A_3_2",
    // "P_0_0","P_0_1","P_0_2","P_1_0","P_1_1","P_1_2","P_2_0","P_2_1","P_2_2","P_3_0","P_3_1","P_3_2",
    // "exponent0","exponent1","exponent2","exponent3",
    // "sum","test_x0","test_x1","test_x2","min_x0","min_x1","min_x2","result","pow","exp"}; //hartman.cpp
        // std::vector<std::string> keys={"length","theta","omega","theta1","length1", "sin", "dt","k1_theta","k1_omega"
        // , "tempTheta","tempOmega","k2_omega","k3_omega","k4_omega","currentTime","res"}; //pendulum.cpp


    // std::vector<std::string> keys={"pi","xarg","sin","acos","a", "b", "s1","h1","x", "val","s2","h2","h_s1"}; //simpsons2.cu
    // std::vector<std::string> keys={"pi","xarg","sin","acos","a","b","h1","x","val","h2"}; //simpsons1.cu
    // std::vector<std::string> keys={"h1","x1","h2","dt","x2","d2","t3","h","sin","sqrt"}; //arclength.cu
    // std::vector<std::string> keys={"x1","sum","term","x","tgamma","pow"}; //arclength.cu

    // 创建合并后的JSON对象
    json merged_json;

    // 遍历每个染色体配置
    for (auto& [chromosome_name, values] : config_json.items()) {
        // 确保数组长度匹配
        if (values.size() != keys.size()) {
            cerr << "Error: 数组长度不匹配Array size mismatch for " << chromosome_name << endl;
            return 1;
        }

        // 创建键值对映射
        json chromosome_mapping;
        for (size_t i = 0; i < keys.size(); ++i) {
            chromosome_mapping[keys[i]] = values[i];
        }
        
        // 添加到合并后的JSON
        merged_json[chromosome_name] = chromosome_mapping;
    }    
    // 第二步：将精度配置与变量名称一一对应起来，形成新的 ConfigMerged.json 文件
    ofstream o("/home/zjn/mytool4/src/ConfigMerged.json");
    o << setw(4) << merged_json << endl;
    o.close();

    cout << "ConfigMerged.json generated successfully!" << endl;


    return 0;
}