
//测量时间加的
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>





#include <stdio.h>

#define N 10000000         // vector size
#define A 10.0              // scalar
#define I 20                // # of scaling iterations
#define X 0.00000003e-20    // initial vector value
#define Y 1.00000003        // initial sum value

double a = A;               // can be double
double x[N];                // can be double
double y[N];                // must be double




int main() {

    // 获取开始时间
    auto start = std::chrono::high_resolution_clock::now();


 
    for (int j=0; j<N; j++) {           // initialize x and y
        x[j] = X;
        y[j] = Y;
    }

    for (int j=0; j<N; j++) {           // compute a*x multiple times
        for (int i=0; i<I; i++) {
            x[j] *= a;
        }
    }

    for (int j=0; j<N; j++) {           // compute x+y
        y[j] += x[j];
    }

    // 获取结束时间
    auto end = std::chrono::high_resolution_clock::now();
    printf("运行结果为%.8f\n", (double)y[0]);     // should print 1.00000006
    printf("绝对误差为:%.15e\n",(y[0]-1.00000006));
    // std::cout << "数据类型为: " << typeid(Y).name() << std::endl;








    // 计算并输出执行时间
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;

    // 求平均运行时间
  std::ofstream file("/home/zjn/adapt-fp-master/apxy/time.txt", std::ios::app); // 添加模式
  file << duration.count() << std::endl; // 写入持续时间并换行
  file.close();
    return 0;
}