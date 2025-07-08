#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#ifndef Chromosome_Number
#define Chromosome_Number 0
#endif

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    int chromosome_number=Chromosome_Number;
    std::cout<<"chromosome_number为"<<chromosome_number<<"\n";


    // const int N = argc - 1;  // 获取输入文件数量

    json merged_data;

    // 遍历所有输入文件
    for (int i = 1; i <= chromosome_number; ++i) {
        //拼接文件路径
        std::string filename="/home/zjn/mytool4/json/chromosome";
        std::string str = std::to_string(i);
        filename+=str;
        filename+=".json";

        // 打开并解析JSON文件
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            continue;
        }

        json file_data;
        try {
            file >> file_data;
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error in " << filename << ": " << e.what() << std::endl;
            continue;
        }

        // 验证JSON结构
        if (!file_data.is_object() || file_data.size() != 1) {
            std::cerr << "Invalid format in " << filename << ": Expected single-key JSON object" << std::endl;
            continue;
        }

        // 合并数据
        auto it = file_data.begin();
        merged_data[it.key()] = it.value();
    }

    // 写入合并后的结果
    std::ofstream out_file("/home/zjn/mytool4/src/RuntimeAndError.json");
    if (!out_file) {
        std::cerr << "Error: Unable to create output file" << std::endl;
        return 1;
    }
    out_file << merged_data.dump(4);

    // 输出参数N和合并结果
    std::cout << "Successfully merged " << merged_data.size() 
              << " files into RuntimeAndError.json" << std::endl;

    return 0;
}