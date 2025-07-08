#include <iostream>
#include <cmath>
#include <cstdio>




int N = 100000;

double compute_doppler(double t2) {
    double result;
    result=sin(t2) / (1 + t2 * t2);
    return result;
}



int main() {


    double t0 = 0.0;
    double t1 = 10.0;
    double dt = (t1 - t0) / N;
    double sum = 0.0;

    for (int i = 0; i < N; ++i) {
        double t;
        double doppler_value;
        t = t0 + i * dt;
        doppler_value = compute_doppler(t);
        sum =sum + doppler_value * dt;
    }



    return 0;
}