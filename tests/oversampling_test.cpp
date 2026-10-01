#include "Nome6DSP.hpp"
#include <array>
#include <cmath>
#include <iostream>

static double run(int os) {
    using namespace nome6;
    Engine e;
    std::array<double, kNumOps> ph{};
    e.configure(48000.0, os, ph);

    Controls6 ctl{};
    for (int i = 0; i < kNumOps; ++i) {
        ctl[i].freq = 83.0 * (i + 1) * (1.0 + 0.031 * i);
        ctl[i].amp = 0.18 + 0.03 * i;
        ctl[i].m = 0.45 + 0.09 * i;
        ctl[i].family = 0.15 * i;
        ctl[i].bandwidth = 18000.0;
    }

    Matrix6 mod{};
    mod[0][1] = 1.6;
    mod[1][2] = -1.1;
    mod[2][0] = 0.8;
    mod[3][3] = 0.35;
    mod[4][5] = 1.2;
    mod[5][4] = -0.7;

    std::array<double, kNumOps> y{};
    double sum2 = 0.0;
    int count = 0;
    for (int n = 0; n < 48000; ++n) {
        e.processHostSample(ctl, mod, y);
        for (double v : y) {
            if (!std::isfinite(v))
                return -1.0;
            if (n > 512) {
                sum2 += v * v;
                ++count;
            }
        }
    }
    return std::sqrt(sum2 / double(count));
}

int main() {
    for (int os : {1, 2, 4}) {
        const double rms = run(os);
        std::cout << os << "x rms=" << rms << "\n";
        if (!(rms > 0.001 && rms < 2.0))
            return 1;
    }
    return 0;
}
