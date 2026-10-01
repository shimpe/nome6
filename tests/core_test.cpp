#include "Nome6DSP.hpp"
#include <cmath>
#include <iostream>

int main() {
    using namespace nome6;

    PreparedElliptic p{};
    prepareElliptic(0.5, p);

    const double a = renderElliptic(0.0, p, kMaxPartials, 0.0);
    const double b = renderElliptic(0.25, p, kMaxPartials, 0.0);
    const double c = renderElliptic(0.5, p, kMaxPartials, 0.0);
    const double d = renderElliptic(0.75, p, kMaxPartials, 0.0);

    if (std::abs(a) > 1.0e-9 || std::abs(b - 1.0) > 1.0e-7 ||
        std::abs(c) > 1.0e-7 || std::abs(d + 1.0) > 1.0e-7) {
        std::cerr << "Jacobi sn quarter-cycle regression failed: "
                  << a << " " << b << " " << c << " " << d << "\n";
        return 1;
    }

    PreparedElliptic sine{};
    prepareElliptic(0.0, sine);
    double maxErr = 0.0;
    for (int i = 0; i < 4096; ++i) {
        const double ph = double(i) / 4096.0;
        const double y = renderElliptic(ph, sine, 1, 0.0);
        const double ref = std::sin(kTwoPi * ph);
        maxErr = std::max(maxErr, std::abs(y - ref));
    }

    std::cout << "sine-limit max error=" << maxErr << "\n";
    if (maxErr >= 1.0e-6)
        return 1;

    // Regression: Release builds use fast floating-point optimizations.  Do
    // not use NaN as an initialization sentinel: under /fp:fast or
    // -ffast-math a NaN comparison may be optimized under the assumption that
    // NaNs never occur, leaving the engine stuck on its sine-limit table.
    auto renderEngineRms = [](double m) {
        Engine engine;
        std::array<double, kNumOps> phase{};
        engine.configure(48000.0, 1, phase);

        Controls6 ctl{};
        Matrix6 matrix{};
        ctl[0].freq = 110.0;
        ctl[0].amp = 1.0;
        ctl[0].m = m;
        ctl[0].family = 0.0;
        ctl[0].bandwidth = 18000.0;

        std::array<double, kNumOps> y{};
        double e2 = 0.0;
        int count = 0;
        for (int n = 0; n < 4096; ++n) {
            engine.processHostSample(ctl, matrix, y);
            if (n >= 128) {
                e2 += y[0] * y[0];
                ++count;
            }
        }
        return std::sqrt(e2 / double(count));
    };

    const double rmsSine = renderEngineRms(0.0);
    const double rmsElliptic = renderEngineRms(0.999999);
    std::cout << "engine m=0 rms=" << rmsSine
              << " m=.999999 rms=" << rmsElliptic << "\n";
    if (std::abs(rmsElliptic - rmsSine) < 0.05)
        return 1;

    return 0;
}
