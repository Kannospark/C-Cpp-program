#include <iostream>
#include <chrono>
#include <vector>
#include <random>

using namespace std;
using namespace chrono;

int main()
{
    const int N = 1000000000;
    vector<double> a(N), b(N);
    default_random_engine generator;
    uniform_real_distribution<double> distribution(0.0, 1.0);
    for (int i = 0; i < N; ++i) {
        a[i] = distribution(generator);
        b[i] = distribution(generator);
    }

    auto start = high_resolution_clock::now();
    double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        sum += a[i] * b[i];
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "C++ dot product computation took " << duration.count() << " milliseconds" << endl;
    cout << "Dot product result: " << sum << endl;

    return 0;
}