#include "Nome6DSP.hpp"
#include <array>
#include <cmath>
#include <iostream>

using namespace nome6;

static double diffRms(const Controls6& aCtl,
                      const Controls6& bCtl,
                      const NoiseControl& nc,
                      int samples = 24000) {
    Engine a, b;
    std::array<double, kNumOps> ph{};
    a.configure(48000.0, 2, ph, 0x12345678u);
    b.configure(48000.0, 2, ph, 0x12345678u);
    Matrix6 mod{};
    std::array<double, kNumOps> ya{}, yb{};
    double d2 = 0.0;
    int count = 0;
    for (int n = 0; n < samples; ++n) {
        a.processHostSample(aCtl, mod, nc, ya);
        b.processHostSample(bCtl, mod, nc, yb);
        if (!std::isfinite(ya[0]) || !std::isfinite(yb[0]))
            return -1.0;
        if (n > 256) {
            const double d = ya[0] - yb[0];
            d2 += d * d;
            ++count;
        }
    }
    return std::sqrt(d2 / double(count));
}

int main() {
    NoiseControl nc{};
    nc.color = -0.35;
    nc.cutoff = 5500.0;

    Controls6 base{};
    base[0].freq = 110.0;
    base[0].amp = 0.6;
    base[0].m = 0.97;
    base[0].family = 0.2;
    base[0].bandwidth = 18000.0;

    Controls6 ampNoise = base;
    ampNoise[0].noiseAmp = 0.15;
    const double ampDiff = diffRms(base, ampNoise, nc);

    Controls6 phaseNoise = base;
    phaseNoise[0].noisePhase = 0.35;
    const double phaseDiff = diffRms(base, phaseNoise, nc);

    Controls6 mNoise = base;
    mNoise[0].noiseM = 0.025;
    const double mDiff = diffRms(base, mNoise, nc);

    std::cout << "noise diff rms amp=" << ampDiff
              << " phase=" << phaseDiff
              << " m=" << mDiff << "\n";

    if (!(ampDiff > 0.01 && phaseDiff > 0.01 && mDiff > 0.001))
        return 1;

    // Shared-source regression: opposite additive routes on two otherwise
    // silent operators should cancel sample-for-sample after identical
    // decimation paths.
    Engine shared;
    std::array<double, kNumOps> ph{};
    shared.configure(48000.0, 2, ph, 0xabcdef01u);
    Controls6 ctl{};
    ctl[0].noiseAmp = 0.2;
    ctl[1].noiseAmp = -0.2;
    Matrix6 mod{};
    std::array<double, kNumOps> y{};
    double worst = 0.0;
    for (int n = 0; n < 4096; ++n) {
        shared.processHostSample(ctl, mod, nc, y);
        worst = std::max(worst, std::abs(y[0] + y[1]));
    }
    std::cout << "shared-noise cancellation max=" << worst << "\n";
    if (worst > 1.0e-10)
        return 1;

    return 0;
}
