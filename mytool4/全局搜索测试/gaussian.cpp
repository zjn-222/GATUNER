#include <cmath>
#include <iostream>


// Gaussian函数定义
double gaussian(float x, float mean, float stddev) {
    float variance = powf(stddev, 2);
    float coeff = 1.0 / (sqrt(2.0 * M_PI * variance));
    float exponent = -powf(x - mean, 2) / (2.0 * variance);
    return coeff * exp(exponent);
}

int main() {


    double x = 0.0; // 计算点
    double mean = 0.0; // 高斯分布的均值
    double stddev = 1.0; // 高斯分布的标准差

    double result = gaussian(x, mean, stddev);


    return 0;
}