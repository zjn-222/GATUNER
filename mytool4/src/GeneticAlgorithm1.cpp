// 遗传算法：只管执行遗传算法步骤，比较适应度，不涉及其中的细节（例如适应度的计算，变量列表的设置）
// 没有考虑误差，只考虑适应度，适应度越高越好

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <cstdio>

#include <iomanip>  // 包含该头文件以使用 std::setw

//文件读写
#include <fstream>
#include <iostream>
using namespace std;

//json相关
#include "/usr/include/nlohmann/json.hpp"
using json = nlohmann::json;

#include <queue> // 必须添加这个头文件

#include <cstdlib> // 包含 system() 函数

// 个体结构体（需要根据混合精度需求修改染色体编码）
struct Individual {
    std::vector<int> chromosome; // 染色体编码（例如：0表示fp16，1表示fp32,2表示fp64）
    double fitness = -1;         // 适应度得分
    double error = 0;  // 误差

    // 可添加混合精度特定参数
    // 例如：各层的精度配置、内存占用预估等
};

class GeneticAlgorithm {
private:
    // 算法参数（需要根据场景调整）
    const int populationSize = 10;    // 种群大小
    const int maxGenerations = 10;     // 最大迭代次数
    const double mutationRate = 0.01;  // 变异概率
    const double crossoverRate = 0.8;  // 交叉概率
    const int elitismCount = 2;        // 精英保留数量

    // 随机数生成器
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist{0.0, 1.0};

public:
    // 构造函数（初始化随机数生成器）
    GeneticAlgorithm() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
    }

    // 运行遗传算法主流程
    void run() {
        // 初始化种群
        std::vector<Individual> population = initializePopulation();
        
        for (int generation = 0; generation < maxGenerations; ++generation) {
            // 评估适应度（需实现混合精度评估逻辑）
            evaluateFitness(population);
            cout<<"适应度评估完成"<<endl;
            
            // 选择操作
            std::vector<Individual> newPopulation = selection(population);
            cout<<"选择操作完成"<<endl;
            
            // 交叉操作（需实现混合精度特定的交叉策略）
            crossover(newPopulation);
            cout<<"交叉操作完成"<<endl;
            
            // 变异操作（需实现混合精度特定的变异策略）
            mutate(newPopulation);
            cout<<"变异操作完成"<<endl;

            // 精英保留
            applyElitism(population, newPopulation);
            cout<<"精英保留完成"<<endl;
            
            population = newPopulation;
            
            // 输出当前最优解
            printBest(population, generation);
        }
    }

private:
    // 添加的
    std::string buildScriptCommand() const {
        std::ostringstream oss;
        oss << "/home/zjn/mytool4/src/run.sh " << populationSize;
        return oss.str();
    }

    // 初始化种群（需要根据混合精度需求修改）
    std::vector<Individual> initializePopulation() {
        std::vector<Individual> population(populationSize);
        
        // 示例：随机生成二进制染色体（1或2）
        for (auto& individual : population) {
            individual.chromosome.resize(10); // 假设10个可配置层
            for (auto& gene : individual.chromosome) {
                gene = (dist(rng) > 0.5) ? 1 : 2; // 随机初始化
            }
        }
        return population;
    }

    // 适应度评估函数（需要根据混合精度需求实现）
    void evaluateFitness(std::vector<Individual>& population) {
        int ii=1;
        json j2;
        for (auto& individual : population) {
            // 需要实现的逻辑：
            // 1. 根据染色体解码精度配置（如将染色体映射到各层精度）
            // 2. 评估配置的性能（精度、速度、内存等）
            // 3. 计算综合适应度得分
            
            // 将所有精度配置写入到json文件中
            std::string a="chromosome";

            char buffer[20];
            sprintf(buffer, "%d", ii);
            // 将字符数组转换为 std::string
            std::string str(buffer);

            std::string b=a+str;
            j2[b] = individual.chromosome;
            ii++;
        }
        // 将 JSON 对象写入文件
        std::ofstream o("/home/zjn/mytool4/src/Config.json");
        o << std::setw(4) << j2 << std::endl;
        o.close();


        //调用外部run.sh脚本，计算每一个精度配置的得分,把得分和误差写入到RuntimeAndError.json
        // 构建动态命令（关键修改）
        std::string command = buildScriptCommand();
        // 调试输出
        std::cout << "Executing: " << command << std::endl; 
        // 执行脚本并检查状态
        int script_status = system(command.c_str());
        if (script_status != 0) {
            std::cerr << "脚本执行失败，错误码: " << script_status 
                    << " 命令: " << command << std::endl;
            exit(EXIT_FAILURE);
        }


        // 读取json文件的结果，获取所有精度配置的运行时间和误差
        // 从文件中读取 JSON 数据
        std::ifstream i("/home/zjn/mytool4/src/RuntimeAndError.json");
        json j1;
        i >> j1;
        i.close();

        std::vector<json> chromosomeData; // 存储所有染色体数据
        for (auto& [key, value] : j1.items()) {
            // 可选：校验键名格式
            if (key.find("chromosome") != 0) {
                std::cerr << "忽略未知键: " << key << std::endl;
                continue;
            }

            // 校验数据结构
            if (!value.is_object() || 
                !value.contains("runtime") || 
                !value.contains("error")) {
                std::cerr << "无效数据结构: " << key << std::endl;
                continue;
            }

            try {
                // 提取数据
                chromosomeData.emplace_back();  // 先创建空对象
                auto& entry = chromosomeData.back();  // 再获取引用
                entry["name"] = key;
                entry["runtime"] = value["runtime"].get<uint64_t>();
                entry["error"] = value["error"].get<double>();
                
                // 此处可添加更多处理逻辑
            } catch (json::exception& e) {
                std::cerr << "数据解析错误 (" << key << "): " << e.what() << std::endl;
            }
        }

        // 后续可通过遍历 chromosomeData 处理所有染色体
        // for (const auto& data : chromosomeData) {
        //     std::cout << "处理 " << data["name"] 
        //             << " 运行时: " << data["runtime"] 
        //             << " 误差: " << data["error"] << std::endl;
        // }

    // 动态分配适应度值
    for (const auto& data : chromosomeData) {
        // 解析染色体编号（例如 "chromosome3" -> 2）
        int index;
        try {
            std::string key = data["name"];
            index = std::stoi(key.substr(10)) - 1; // 从第10个字符开始截取
        } catch (...) {
            continue; // 跳过无效编号
        }

        // 关联到对应个体
        if (index >= 0 && index < population.size()) {
            uint64_t runtime = data["runtime"];
            population[index].error = data["error"];  // 误差赋值
            
            // 核心修改：计算基于 runtime 的适应度
            if (runtime == 0) {
                population[index].fitness = std::numeric_limits<double>::max();
            } else {
                population[index].fitness = 1.0 / runtime; // 适应度与runtime成反比
            }
            
        }
    }
    }

    // 选择操作（轮盘赌选择）
    std::vector<Individual> selection(const std::vector<Individual>& population) {
        // 实现选择逻辑...
        std::vector<Individual> selected;
        selected.reserve(population.size());

        // 1. 创建适应度副本并排序（降序）
        std::vector<Individual> sortedPopulation = population;
        std::sort(sortedPopulation.begin(), sortedPopulation.end(),
            [](const Individual& a, const Individual& b) {
                return a.fitness > b.fitness; // 倒序排列（适应度越高排名越前）
            });

        // 2. 配置排名选择参数
        const int N = sortedPopulation.size();
        const double selectionPressure = 1.7; // 选择压力系数（推荐范围1.5-2.0）

        // 3. 计算排名概率（线性排名选择公式）
        std::vector<double> probabilities(N);
        for (int rank = 0; rank < N; ++rank) {
            probabilities[rank] = (2.0 - selectionPressure)/N 
                                + 2.0 * rank * (selectionPressure - 1.0)/(N * (N - 1));
        }

        // 4. 构建累积概率轮盘
        std::vector<double> wheel(N);
        wheel[0] = probabilities[0];
        for (int i = 1; i < N; ++i) {
            wheel[i] = wheel[i-1] + probabilities[i];
        }

        // 5. 轮盘赌选择
        std::uniform_real_distribution<double> dist(0.0, wheel.back());
        for (size_t i = 0; i < population.size(); ++i) {
            double spin = dist(rng);
            auto it = std::upper_bound(wheel.begin(), wheel.end(), spin);
            int selectedIndex = std::distance(wheel.begin(), it);
            
            // 边界保护
            selectedIndex = std::min(selectedIndex, N-1);
            selected.push_back(sortedPopulation[selectedIndex]);
        }

        return selected;
    }

    // 交叉操作（需要实现混合精度优化策略）
    void crossover(std::vector<Individual>& population) {
    // 创建临时子代容器
    std::vector<Individual> newPopulation;
    newPopulation.reserve(population.size());

    // 遍历种群，每次处理两个个体
    for (size_t i = 0; i < population.size(); i += 2) {
        // 处理最后一个奇数个体
        if (i + 1 >= population.size()) {
            newPopulation.push_back(population[i]);
            break;
        }

        Individual& parent1 = population[i];
        Individual& parent2 = population[i+1];

        // 检查交叉条件和染色体有效性
        if (dist(rng) > crossoverRate || 
            parent1.chromosome.empty() || 
            parent2.chromosome.empty() ||
            parent1.chromosome.size() != parent2.chromosome.size()) {
            // 不满足交叉条件
            newPopulation.push_back(parent1);
            newPopulation.push_back(parent2);
            continue;
        }

        // 生成随机交叉点 (1 <= point <= length-1)
        std::uniform_int_distribution<size_t> dist_point(1, parent1.chromosome.size()-1);
        size_t crossover_point = dist_point(rng);

        // 创建子代
        Individual child1, child2;

        // 执行单点交叉
        child1.chromosome.insert(child1.chromosome.end(),
                                parent1.chromosome.begin(),
                                parent1.chromosome.begin() + crossover_point);
        child1.chromosome.insert(child1.chromosome.end(),
                                parent2.chromosome.begin() + crossover_point,
                                parent2.chromosome.end());

        child2.chromosome.insert(child2.chromosome.end(),
                                parent2.chromosome.begin(),
                                parent2.chromosome.begin() + crossover_point);
        child2.chromosome.insert(child2.chromosome.end(),
                                parent1.chromosome.begin() + crossover_point,
                                parent1.chromosome.end());

        // 保留子代
        newPopulation.push_back(child1);
        newPopulation.push_back(child2);
    }

    // 用新种群替换旧种群
    population = std::move(newPopulation);
    }

    // 变异操作（需要实现混合精度优化策略）
    void mutate(std::vector<Individual>& population) {
        std::uniform_real_distribution<double> probDist(0.0, 1.0);

        for (auto& individual : population) {
            for (auto& gene : individual.chromosome) {
                if (probDist(rng) < mutationRate) {
                    // 强制在1和2之间切换
                    gene = (gene == 1) ? 2 : 1; 
                }
            }
        }
    }

    // // 精英保留策略
    void applyElitism(const std::vector<Individual>& oldPop, 
                    std::vector<Individual>& newPop) {
    if (elitismCount <= 0 || oldPop.empty() || newPop.empty()) return;

    // 1. 获取旧种群中的精英
    std::vector<Individual> sortedOld = oldPop;
    std::sort(sortedOld.begin(), sortedOld.end(),
             [](const auto& a, const auto& b) { return a.fitness > b.fitness; });
    
    // 2. 找到新种群中的最差个体索引
    using FitnessIndex = std::pair<double, size_t>;
    auto cmp = [](const FitnessIndex& a, const FitnessIndex& b) { 
        return a.first > b.first; // 最小堆
    };
    
    // 使用标准库的 priority_queue
    std::priority_queue<FitnessIndex, std::vector<FitnessIndex>, decltype(cmp)> pq(cmp);
    
    for (size_t i = 0; i < newPop.size(); ++i) {
        pq.emplace(newPop[i].fitness, i);
        if (pq.size() > elitismCount) pq.pop();
    }
    
    // 3. 替换最差个体
    for (int i = 0; i < elitismCount; ++i) {
        if (i >= sortedOld.size() || pq.empty()) break;
        newPop[pq.top().second] = sortedOld[i];
        pq.pop();
    }

    }



    void printBest(const std::vector<Individual>& pop, int gen) {
        auto best = *std::max_element(pop.begin(), pop.end(),
            [](const auto& a, const auto& b) { return a.fitness < b.fitness; });
        
        std::cout << "Generation " << gen 
                << " Best fitness: " << best.fitness
                << " runtime:" << (1.0 / best.fitness)
                << " error:" << best.error  
                << " Chromosome: ";
        for (int g : best.chromosome) std::cout << g;
        std::cout << std::endl;
    }
};

// 使用示例
int main() {
    GeneticAlgorithm ga;
    ga.run();
    return 0;
}