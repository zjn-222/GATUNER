
#include <iostream>
#include <cmath>
#include <cstdio>



int N = 100000;

double compute_doppler(double t1) {
    float result;
    result=cosf(t1) / (1 + t1 * t1);
    return result;
}


int main() {


    double t0 = 0.0;
    double t1 = 10.0;
    double dt = (t1 - t0) / N;
    double sum = 0.0;

    for (int i = 0; i < N; ++i) {
        float t = t0 + i * dt;
        sum =sum+ compute_doppler(t) * dt;
    }


    return 0;
}