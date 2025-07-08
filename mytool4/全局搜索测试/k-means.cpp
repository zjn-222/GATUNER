#include <iostream>
#include <cmath>




// 固定的数学常量pi
const double PI = std::acos(-1);



// 计算函数在x处的导数值
double df(double x) {
    return cos(x);
}

// 计算弧长的梯形规则近似
double arcLengthTrapezoidalRule(double a, double b, int n) {
    double h = (b - a) / n;  // 分割区间的步长
    double arcLength = 0.0; 
    double x;
    double fx1;
    double fx2;
    double temp1;
    double temp2;
    n=100000;
    for (int i = 0; i < n; ++i) {
        x = a + i * h;            // 当前区间的左端点
        temp1=cos(x);
        temp2=cos(x+h);
        fx1 = sqrt(1 + temp1);         // 当前区间的左端点处的函数值
        fx2 = sqrt(1 + temp2 ); // 当前区间的右端点处的函数值
        arcLength =arcLength+(fx1 + fx2) * h / 2;           // 梯形面积
    }

    return arcLength;
}

int main() {


    double a = 0.0;  // 积分的起点
    double b = PI;   // 积分的终点
    int n = 100000;    // 初始分割数量

    double arcLength = arcLengthTrapezoidalRule(a, b, n);


    




}