#include <cmath>
#include <complex>
#include <iostream>

#include <iostream>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iomanip>





const double PI = acos(-1.0);

void fft(int n, int step, std::complex<double>* a) {
    if (step < n) {
        fft(n, step * 2, a);
        fft(n, step * 2, a + step);
        for (int i = 0; i < n; i += 2 * step) {
            std::complex<double> t = std::polar(1.0, -PI * i / n) * a[i + step];
            a[i + step] = a[i] - t;
            a[i] += t;
        }
    }
}

int main() {


    const int N = 8;
    std::complex<double> data1[N];

    // 初始化输入数据
    for (int i = 0; i < N; ++i) {
        double real_part = cos(2 * PI * i / N);
        double imag_part = sin(2 * PI * i / N);
        data1[i] = std::complex<double>(real_part, imag_part);
    }


    // 执行FFT
    fft(N, 1, data1);




    return 0;
}