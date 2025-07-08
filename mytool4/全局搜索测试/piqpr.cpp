#include <iostream>
#include <iomanip>
#include <cmath>




const int N = 30000000; // 0.3 * 10^8

double bbp_term(int n, int k) {
    double term = 0.0;
    double denom = 8 * k + n;
    term =term+ 4.0 / denom;
    term =term- 2.0 / (denom + 2);
    term =term- 1.0 / (denom + 4);
    term =term- 1.0 / (denom + 6);
    term =term/ pow(16.0, k);
    return term;
}

double bbp_pi(int n) {
    double sum = 0.0;
    for (int k = 0; k <= n; ++k) {
        sum =sum+ bbp_term(1, k);
    }
    sum = fmod(sum, 1.0);
    return sum;
}
int main() {


    double result=bbp_pi(N);




    return 0;
}