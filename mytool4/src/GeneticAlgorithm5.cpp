// 遗传算法：只管执行遗传算法步骤，比较适应度，不涉及其中的细节（例如适应度的计算，变量列表的设置）
// 考虑误差和适应度,增加了初始化种群
// 变量分组版本
//新增为auto_test.sh服务,修改了日志

//  编译指令：g++ -std=c++17 GeneticAlgorithm.cpp -o GeneticAlgorithm

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <cstdio>
#include <iomanip>  // 包含该头文件以使用 std::setw
//测量时间的
#include <sys/time.h>
#include <iostream>
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
    // 变量分组定义 (可以不按顺序写)(merge不用修改顺序)(只写你想将哪些变量的代号分到同一组即可)
    std::vector<std::vector<int>> groups = {  
        {4, 5,6,7},   
        {9,2, 1,3},
        {0,8}      
    };
    std::vector<int> group_params = std::vector<int>(groups.size(), 2); // 每个组对应参数初始化为2

    // 可添加混合精度特定参数
    // 例如：各层的精度配置、内存占用预估等
};
int DNA_Num=10;
class GeneticAlgorithm {
private:
    // 算法参数（需要根据场景调整）
    const int populationSize = 100;    // 种群大小,即染色体数量
    const int maxGenerations = 1000;     // 最大迭代次数
    const double mutationRate = 0.6;  // 变异概率
    const double crossoverRate = 0.2;  // 交叉概率
    const int elitismCount = 2;        // 精英保留数量
    const double ErrorThreshold = 1e-8;
    const double FenzuCrossoverRate = 1.0;  //分组交叉概率
    const double FenzuMutationRate = 1.0;   //分组变异概率

    // 随机数生成器
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist{0.0, 3.0};



    // 新增的打印种群染色体函数
    void printPopulation(const std::vector<Individual>& population, int generation) {
        std::ostringstream oss;
        oss << "\nGeneration " << generation << " Chromosomes:\n";
        for (size_t i = 0; i < population.size(); ++i) {
            oss << "Individual " << std::setw(2) << i << ": ";
            for (int gene : population[i].chromosome) {
                oss << gene << " ";
            }
            oss << std::endl;
        }
        oss << "------------------------\n";
        
        // 同时输出到控制台和文件
        std::cout << oss.str();
        logFile << oss.str() << std::flush;  // 添加 flush 操作
    }

public:
    std::ofstream logFile;  // 新增日志文件流

    // 构造函数（初始化随机数生成器）
    GeneticAlgorithm() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);

        // 打开日志文件
        logFile.open("/home/zjn/mytool4/log/log.txt", std::ios::out);
        if (!logFile.is_open()) {
            std::cerr << "无法打开日志文件 log.txt!" << std::endl;
            exit(EXIT_FAILURE);
        }
        logFile << "Genetic Algorithm Log\n";
        logFile << "======================\n";
    }

    // 新增析构函数
    ~GeneticAlgorithm() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    
    bool is_same_individual(const Individual& a, const Individual& b) {
        return a.chromosome == b.chromosome;
    }

    // 运行遗传算法主流程
    void run() {
        // 初始化种群
        std::vector<Individual> population = initializePopulation();

        // 新增：记录前一代最优个体和连续相同次数
        Individual prevBest;
        int sameBestCount = 0; 
        bool shouldTerminate = false;

        for (int generation = 0; generation < maxGenerations; ++generation) {
            // 添加染色体打印
            printPopulation(population, generation);

            
            if (generation==0) {
            // 评估适应度（需实现混合精度评估逻辑）
            evaluateFitness(population);
            cout<<"适应度第一次评估完成"<<endl;
            }

            // 输出当前最优解
            // printBest(population, generation);
            Individual currentBest = printBest(population, generation);

            // 新增：判断是否连续相同
            if (generation > 0 && is_same_individual(currentBest, prevBest)) {
                sameBestCount++;
                cout << "连续相同次数: " << sameBestCount << endl;
                
                // 达到10次提前终止,已经收敛
                if (sameBestCount >= 20) {
                    cout << "提前终止：连续10代最优个体未变化" << endl;
                    shouldTerminate = true;
                }
            } else {
                sameBestCount = 0; // 重置计数器
                prevBest = currentBest; // 深拷贝需要确保Individual支持拷贝操作
            }

            if (shouldTerminate) break; // 提前退出循环

            // 选择操作
            std::vector<Individual> newPopulation = selection(population);
            cout<<"选择操作完成"<<endl;
            // std::vector<Individual> newPopulation = population;

            // 交叉操作（需实现混合精度特定的交叉策略）
            crossover(newPopulation);
            cout<<"交叉操作完成"<<endl;
            
            // 变异操作（需实现混合精度特定的变异策略）
            mutate(newPopulation);
            cout<<"变异操作完成"<<endl;

            evaluateFitness(newPopulation);
            cout<<"适应度第二次评估完成"<<endl;


            // 精英保留
            applyElitism(population, newPopulation);
            cout<<"精英保留完成"<<endl;
            
            population = newPopulation;
            
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

        for (auto& individual : population) {
            individual.chromosome.resize(DNA_Num); // 假设10个可配置层

            for (auto& u : individual.group_params) {
                double rand = dist(rng);  // 生成一次随机数
                if (rand > 1.5) u = 2;
                else u = 1;
            }
            // 按组索引填充染色体
            for (int j=0; j<individual.groups.size(); ++j) {
                for (int m : individual.groups[j]) {
                    individual.chromosome[m] = individual.group_params[j];
                }
            }
        }

        
        //用第一个工作的精度配置初始化chromosome1和chromosome2
        //初始化全double和全float，增加种群的解
        population[0].chromosome={1,1,1,1,1,1,1,1,1,1};
        population[0].group_params = std::vector<int>(population[0].groups.size(), 1); // 所有组参数设为1
        population[1].chromosome={2,2,2,2,2,2,2,2,2,2};
        population[1].group_params = std::vector<int>(population[1].groups.size(), 2);
        
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


    // 筛选符合条件的个体
    std::vector<Individual> eligible;
    for (const auto& ind : population) {
        if (ind.error <= ErrorThreshold) {
            eligible.push_back(ind);
        }
    }

    bool hasEligible = !eligible.empty();
    std::vector<Individual> candidates;

    if (hasEligible) {
        // 按适应度降序排序合格个体
        candidates = eligible;
        std::sort(candidates.begin(), candidates.end(),
            [](const Individual& a, const Individual& b) {
                return a.fitness > b.fitness;
            });
    } else {
        // 无合格个体，按误差升序排序整个种群
        candidates = population;
        std::sort(candidates.begin(), candidates.end(),
            [](const Individual& a, const Individual& b) {
                return a.error < b.error;
            });
    }

    // 计算基于排名的选择概率
    size_t M = candidates.size();
    std::vector<double> weights(M);
    for (size_t i = 0; i < M; ++i) {
        weights[i] = M - i; // 排名越高（i越小），权重越大
    }

    // 计算总权重并归一化
    double totalWeight = std::accumulate(weights.begin(), weights.end(), 0.0);
    std::vector<double> probabilities(M);
    for (size_t i = 0; i < M; ++i) {
        probabilities[i] = weights[i] / totalWeight;
    }

    // 构建累积概率轮盘
    std::vector<double> wheel(M);
    wheel[0] = probabilities[0];
    for (size_t i = 1; i < M; ++i) {
        wheel[i] = wheel[i-1] + probabilities[i];
    }

    // 轮盘赌选择
    std::vector<Individual> newPopulation;
    newPopulation.reserve(population.size());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (size_t i = 0; i < population.size(); ++i) {
        double r = dist(rng);
        auto it = std::lower_bound(wheel.begin(), wheel.end(), r);
        size_t index = std::distance(wheel.begin(), it);
        index = std::min(index, M-1); // 防止越界
        newPopulation.push_back(candidates[index]);
    }

    return newPopulation;


    }


//组交叉and单点交叉
// 交叉操作（以组为单位交换参数）
void crossover(std::vector<Individual>& population) {
    std::vector<Individual> newPopulation;
    newPopulation.reserve(population.size());
    std::uniform_real_distribution<double> rateDist(0.0, 1.0);  //随即生成，决定是否触发交叉
    std::uniform_int_distribution<size_t> groupDist(0, population[0].group_params.size() - 1);  //获取组的数量，随机生成组的索引


// ​​步长设计​​：i += 2 保证每次处理一对父代
// ​​奇数处理​​：当种群数量为奇数时，最后一个个体直接保留
    for (size_t i = 0; i < population.size(); i += 2) {
        if (i + 1 >= population.size()) {
            newPopulation.push_back(population[i]);
            break;
        }

        //通过值拷贝创建父代副本
        Individual parent1 = population[i];   
        Individual parent2 = population[i+1];

        // 检查交叉条件
        // ​​核心逻辑​​：当随机数大于交叉率时，直接保留父代
        // ​效果示例​​：假设crossoverRate = 0.9，则有20%的概率跳过交叉
        if (rateDist(rng) > crossoverRate) {
            newPopulation.push_back(parent1);
            newPopulation.push_back(parent2);
            continue;
        }

        // 生成随机交叉点 (1 <= point <= length-1)
        std::uniform_int_distribution<size_t> dist_point(1, parent1.chromosome.size()-1);
        size_t crossover_point = dist_point(rng);

        // 创建子代并继承组参数
        Individual child1, child2;
        child1.group_params = parent1.group_params;
        child2.group_params = parent2.group_params;

        //确定分组交叉还是单点交叉
        if (rateDist(rng)<FenzuCrossoverRate) {
            // 随机选择一个组进行参数交换
            // 随机选择一个组索引（如0或1）
            // 交换两个子代在该组的参数值
            size_t groupIdx = groupDist(rng);
            std::swap(child1.group_params[groupIdx], child2.group_params[groupIdx]);

            // 根据新的组参数生成染色体
            auto generateChromosome = [](Individual& ind) {
                ind.chromosome.resize(DNA_Num); // 假设染色体长度固定
                for (size_t j = 0; j < ind.group_params.size(); ++j) {
                    for (int gene : ind.groups[j]) {
                        ind.chromosome[gene] = ind.group_params[j];
                    }
                }
            };

            generateChromosome(child1);
            generateChromosome(child2);
        } else {
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
        }

        newPopulation.push_back(child1);
        newPopulation.push_back(child2);
    }

    population = std::move(newPopulation);
}

//单纯的组交叉
// void crossover(std::vector<Individual>& population) {
//     std::vector<Individual> newPopulation;
//     newPopulation.reserve(population.size());
//     std::uniform_real_distribution<double> rateDist(0.0, 1.0);  //随即生成，决定是否触发交叉
//     std::uniform_int_distribution<size_t> groupDist(0, population[0].group_params.size() - 1);  //获取组的数量，随机生成组的索引


// // ​​步长设计​​：i += 2 保证每次处理一对父代
// // ​​奇数处理​​：当种群数量为奇数时，最后一个个体直接保留
//     for (size_t i = 0; i < population.size(); i += 2) {
//         if (i + 1 >= population.size()) {
//             newPopulation.push_back(population[i]);
//             break;
//         }

//         //通过值拷贝创建父代副本
//         Individual parent1 = population[i];   
//         Individual parent2 = population[i+1];

//         // 检查交叉条件
//         // ​​核心逻辑​​：当随机数大于交叉率时，直接保留父代
//         // ​效果示例​​：假设crossoverRate = 0.9，则有20%的概率跳过交叉
//         if (rateDist(rng) > crossoverRate) {
//             newPopulation.push_back(parent1);
//             newPopulation.push_back(parent2);
//             continue;
//         }


//     // 创建子代并继承组参数
//     Individual child1, child2;
//     child1.group_params = parent1.group_params;
//     child2.group_params = parent2.group_params;


//         // 随机选择一个组进行参数交换
//         // 随机选择一个组索引（如0或1）
//         // 交换两个子代在该组的参数值
//         size_t groupIdx = groupDist(rng);
//         std::swap(child1.group_params[groupIdx], child2.group_params[groupIdx]);

//         // 根据新的组参数生成染色体
//         auto generateChromosome = [](Individual& ind) {
//             ind.chromosome.resize(DNA_Num); // 假设染色体长度固定
//             for (size_t j = 0; j < ind.group_params.size(); ++j) {
//                 for (int gene : ind.groups[j]) {
//                     ind.chromosome[gene] = ind.group_params[j];
//                 }
//             }
//         };

//         generateChromosome(child1);
//         generateChromosome(child2);


//         newPopulation.push_back(child1);
//         newPopulation.push_back(child2);
//     }

//     population = std::move(newPopulation);
// }

    // // 交叉操作（需要实现混合精度优化策略）
    // void crossover(std::vector<Individual>& population) {
    // // 创建临时子代容器
    // std::vector<Individual> newPopulation;
    // newPopulation.reserve(population.size());

    // // 遍历种群，每次处理两个个体
    // for (size_t i = 0; i < population.size(); i += 2) {
    //     // 处理最后一个奇数个体
    //     if (i + 1 >= population.size()) {
    //         newPopulation.push_back(population[i]);
    //         break;
    //     }

    //     Individual& parent1 = population[i];
    //     Individual& parent2 = population[i+1];

    //     // 检查交叉条件和染色体有效性
    //     if (dist(rng) > crossoverRate || 
    //         parent1.chromosome.empty() || 
    //         parent2.chromosome.empty() ||
    //         parent1.chromosome.size() != parent2.chromosome.size()) {
    //         // 不满足交叉条件
    //         newPopulation.push_back(parent1);
    //         newPopulation.push_back(parent2);
    //         continue;
    //     }

    //     // 生成随机交叉点 (1 <= point <= length-1)
    //     std::uniform_int_distribution<size_t> dist_point(1, parent1.chromosome.size()-1);
    //     size_t crossover_point = dist_point(rng);

    //     // 创建子代
    //     Individual child1, child2;

    //     // 执行单点交叉
    //     child1.chromosome.insert(child1.chromosome.end(),
    //                             parent1.chromosome.begin(),
    //                             parent1.chromosome.begin() + crossover_point);
    //     child1.chromosome.insert(child1.chromosome.end(),
    //                             parent2.chromosome.begin() + crossover_point,
    //                             parent2.chromosome.end());

    //     child2.chromosome.insert(child2.chromosome.end(),
    //                             parent2.chromosome.begin(),
    //                             parent2.chromosome.begin() + crossover_point);
    //     child2.chromosome.insert(child2.chromosome.end(),
    //                             parent1.chromosome.begin() + crossover_point,
    //                             parent1.chromosome.end());

    //     // 保留子代
    //     newPopulation.push_back(child1);
    //     newPopulation.push_back(child2);
    // }

    // // 用新种群替换旧种群
    // population = std::move(newPopulation);
    // }

    // // 变异操作（需要实现混合精度优化策略）
    // void mutate(std::vector<Individual>& population) {
    //     std::uniform_real_distribution<double> probDist(0.0, 1.0);

    //     for (auto& individual : population) {
    //         for (auto& gene : individual.chromosome) {
    //             if (probDist(rng) < mutationRate) {
    //                 // 强制在1和2之间切换
    //                 gene = (gene == 1) ? 2 : 1; 
    //             }
    //         }
    //     }
    // }

// 组变异+单点变异
void mutate(std::vector<Individual>& population) {
    std::uniform_real_distribution<double> rateDist(0.0, 1.0);  

    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    std::uniform_int_distribution<int> valueDist(1, 2); // 关键修改：仅生成1或2


    for (auto& individual : population) {
        if (rateDist(rng)<FenzuMutationRate) {
            for (size_t groupIdx = 0; groupIdx < individual.group_params.size(); ++groupIdx) {
                if (probDist(rng) < mutationRate) {
                    int original = individual.group_params[groupIdx];
                    int newValue;
                    
                    // 优化版：直接生成不同值（避免循环）
                    newValue = (original == 1) ? 2 : 1; // 强制切换

                    // 更新组参数
                    individual.group_params[groupIdx] = newValue;

                    // 同步染色体（保持不变）
                    for (int gene : individual.groups[groupIdx]) {
                        individual.chromosome[gene] = newValue;
                    }
                }
            }
        } else {
            for (auto& gene : individual.chromosome) {
                if (probDist(rng) < mutationRate) {
                    // 强制在1和2之间切换
                    gene = (gene == 1) ? 2 : 1; 
                }
            }
        }

    }
}


// 变异操作（组变异）- 仅允许变异为1或2
// void mutate(std::vector<Individual>& population) {
//     std::uniform_real_distribution<double> probDist(0.0, 1.0);
//     std::uniform_int_distribution<int> valueDist(1, 2); // 关键修改：仅生成1或2

//     for (auto& individual : population) {
//         for (size_t groupIdx = 0; groupIdx < individual.group_params.size(); ++groupIdx) {
//             if (probDist(rng) < mutationRate) {
//                 int original = individual.group_params[groupIdx];
//                 int newValue;
                
//                 // 优化版：直接生成不同值（避免循环）
//                 newValue = (original == 1) ? 2 : 1; // 强制切换

//                 // 更新组参数
//                 individual.group_params[groupIdx] = newValue;

//                 // 同步染色体（保持不变）
//                 for (int gene : individual.groups[groupIdx]) {
//                     individual.chromosome[gene] = newValue;
//                 }
//             }
//         }
//     }
// }

// // 变异操作（确保变异到不同值）
// void mutate(std::vector<Individual>& population) {
//     std::uniform_real_distribution<double> probDist(0.0, 1.0);

//     // 预定义所有可能值
//     const std::array<int, 3> values = {0, 1, 2};

//     for (auto& individual : population) {
//         for (auto& gene : individual.chromosome) {
//             if (probDist(rng) < mutationRate) {
//                 // 创建临时数组排除当前值
//                 std::array<int, 2> candidates;
//                 if (gene == 0) {
//                     candidates = {1, 2};
//                 } else if (gene == 1) {
//                     candidates = {0, 2};
//                 } else {
//                     candidates = {0, 1};
//                 }

//                 // 使用静态分布避免重复创建
//                 static std::uniform_int_distribution<int> indexDist(0, 1);
//                 gene = candidates[indexDist(rng)];
//             }
//         }
//     }
// }

    // // 精英保留策略
    void applyElitism(const std::vector<Individual>& oldPop, 
                    std::vector<Individual>& newPop) {
                        
    if (elitismCount <= 0 || oldPop.empty() || newPop.empty()) return;
    // 1. 筛选旧种群精英 (满足误差的优先)
    std::vector<Individual> eligible, ineligible;
    for (const auto& ind : oldPop) {
        (ind.error <= ErrorThreshold) ? eligible.push_back(ind) : ineligible.push_back(ind);
    }

    // 排序逻辑：合格按适应度降序，不合格按误差升序
    std::sort(eligible.begin(), eligible.end(), 
        [](auto& a, auto& b) { return a.fitness > b.fitness; });
    std::sort(ineligible.begin(), ineligible.end(), 
        [](auto& a, auto& b) { return a.error < b.error; });

    // 合并排序结果：合格个体在前，不合格在后
    std::vector<Individual> sortedElites;
    sortedElites.insert(sortedElites.end(), eligible.begin(), eligible.end());
    sortedElites.insert(sortedElites.end(), ineligible.begin(), ineligible.end());
    if (sortedElites.size() > elitismCount) sortedElites.resize(elitismCount);

    // 2. 定位新种群中最差个体 (自定义优先级)
    auto cmp = [&](size_t a_idx, size_t b_idx) {
        const Individual& a = newPop[a_idx];
        const Individual& b = newPop[b_idx];
        
        // 优先级规则：
        // 1. 不满足误差的比满足的更差
        // 2. 都满足时适应度低的更差 
        // 3. 都不满足时误差大的更差
        const bool a_valid = (a.error <= ErrorThreshold);
        const bool b_valid = (b.error <= ErrorThreshold);
        
        if (a_valid != b_valid) return !a_valid; // 不满足的优先被替换
        if (a_valid) return a.fitness < b.fitness; // 都满足则比适应度
        return a.error > b.error; // 都不满足则比误差
    };

    std::priority_queue<size_t, std::vector<size_t>, decltype(cmp)> pq(cmp);
    
    // 构建最差个体队列
    for (size_t i = 0; i < newPop.size(); ++i) {
        if (pq.size() < elitismCount) {
            pq.push(i);
        } else if (cmp(i, pq.top())) {
            pq.pop();
            pq.push(i);
        }
    }

    // 3. 精英替换
    std::vector<size_t> worstIndexes;
    while (!pq.empty()) {
        worstIndexes.push_back(pq.top());
        pq.pop();
    }

    for (size_t i = 0; i < sortedElites.size() && i < worstIndexes.size(); ++i) {
        newPop[worstIndexes[i]] = sortedElites[i];
    }

    }



    Individual printBest(const std::vector<Individual>& pop, int gen) {
    std::ostringstream oss; //日志服务

    // 筛选满足误差阈值的个体
    std::vector<Individual> validIndividuals;
    for (const auto& ind : pop) {
        if (ind.error <= ErrorThreshold) {
            validIndividuals.push_back(ind);
        }
    }

    Individual best_res;

    // 根据是否存在有效个体决定输出内容
    if (!validIndividuals.empty()) {
        // 在有效个体中找适应度最高的
        auto best = *std::max_element(validIndividuals.begin(), validIndividuals.end(),
            [](const auto& a, const auto& b) { return a.fitness < b.fitness; });
        
        oss << "Generation " << gen 
                << " Best VALID fitness: " << best.fitness
                << " runtime:" << (1.0 / best.fitness)
                << " error:" << best.error  
                << " Chromosome: ";
        for (int g : best.chromosome) oss << g<<" ";
        oss << std::endl;
        best_res=best;
    } else {
                //返回一个无效的个体
        Individual res {
            .chromosome = {1, 0, 2, 1, 0}, // fp32, fp16, fp64, fp32, fp16
            .fitness = -1,                 // 默认未评估
            .error = 0.0                   // 初始误差为0
        };
        best_res=res;

        // 没有满足误差的个体
        oss << "Generation " << gen 
                << " WARNING: All individuals exceed error threshold!" 
                << std::endl;
    }

    // 同时输出到控制台和文件
    std::cout << oss.str();
    logFile << oss.str() << std::flush;  // 添加 flush 操作

    return best_res;
    }


};


double getCurrentTime() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}
// 使用示例
int main() {
    GeneticAlgorithm ga;
    // 开始计时
    double start = getCurrentTime();
    ga.run();
    // 结束计时
    double end = getCurrentTime();
    // 新增：获取时间差并格式化
    double total_time = end - start;
    std::string time_msg = "遗传算法总运行时间: " + std::to_string(total_time) + " 秒\n";
    // 双重输出（控制台+日志文件）
    std::cout << time_msg;
    ga.logFile << time_msg << std::flush; // 直接使用类的日志文件流

    return 0;
}