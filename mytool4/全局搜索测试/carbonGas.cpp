#include <iostream>
#include <cmath>




double calculate(double xarg) {
    double result;
    result = exp(-xarg);
    return result;
}

int main() {


    int n = 1000000;
    double t_start;
    double  t_end;
    double h;
    double s1;
    double t;

    t_start = 0.0;
    t_end = 1.0;
    h = (t_end - t_start) / (2.0 * n);
    s1 = 0.0;

    t = t_start;
    s1 = (calculate(t_start) + calculate(t_end));

    for(int i = 0; i < n; i++) { // ITERS before
        t =t+ h;
        s1 = s1 + 4.0 * calculate(t);
        t = t + h;
        s1 = s1 + 2.0 * calculate(t);
    }
    s1 = s1 * h / 3.0;


  printf("结果为：%.12e\n",s1);





    return 0;
}