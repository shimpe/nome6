#include "Nome6DSP.hpp"
#include <array>
#include <chrono>
#include <iostream>

int main() {
    using namespace nome6;
    Engine e;
    std::array<double, kNumOps> ph{};
    e.configure(48000.0, 2, ph);

    Controls6 ctl{};
    for (int i = 0; i < kNumOps; ++i) {
        ctl[i].freq = 90.0 * (i + 1.13);
        ctl[i].amp = 0.35;
        ctl[i].m = 0.88 - 0.05 * i;
        ctl[i].family = 0.12 * i;
        ctl[i].bandwidth = 18000.0;
    }

    Matrix6 mod{};
    mod[0][1] = 2.0;
    mod[1][2] = 1.4;
    mod[2][0] = 0.9;
    mod[3][4] = -1.2;
    mod[4][5] = 1.1;
    mod[5][3] = 0.7;

    std::array<double, kNumOps> y{};
    constexpr int N = 48000 * 5;
    const auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        e.processHostSample(ctl, mod, y);
    const auto t1 = std::chrono::high_resolution_clock::now();

    const double sec = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "rendered 5 seconds in " << sec
              << " s; realtime factor=" << (5.0 / sec) << "x\n";
    return 0;
}
