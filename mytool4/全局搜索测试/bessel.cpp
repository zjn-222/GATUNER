#include <stdio.h>
#include <cmath>




// 定义贝塞尔函数 J0 的实现逻辑
double bessel_J0(double x) {
    double EPS = 1e-10;
    int MAXIT = 10000;
    double P1 = 1.0;
    double P2 = -0.25;
    double P3 = 0.046875;
    double P4 = -0.00390625;
    double P5 = 0.000325520833333;
    double P6 = -0.000010850694444;

    double sum = 0.0;
    double term = 1.0;
    int k = 0;

    MAXIT=10000;
    for (int k = 0; fabs(term) > EPS && k < MAXIT; ++k) {
        term = pow(-1, k) * pow(x / 2.0, 2 * k) / (tgamma(k + 1) * tgamma(k + 1));
        sum += term;
    }

    return sum;
}

int main() {



    double x1;
    double result ;
    
    x1=1.3456;
    result= bessel_J0(x1);
    


    printf("结果为：%.12e\n",result);

    return 0;
}