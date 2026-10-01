#include "Nome6DSP.hpp"
#include <cmath>
#include <iostream>

int main() {
    using namespace nome6;

    PreparedElliptic p{};
    prepareElliptic(0.97, p);

    double diff2 = 0.0;
    double peakA = 0.0;
    double peakB = 0.0;
    for (int i = 0; i < 4096; ++i) {
        const double ph = double(i) / 4096.0;
        const double sn = renderElliptic(ph, p, kMaxPartials, 0.0);
        const double sd = renderElliptic(ph, p, kMaxPartials, 1.0);
        const double d = sn - sd;
        diff2 += d * d;
        peakA = std::max(peakA, std::abs(sn));
        peakB = std::max(peakB, std::abs(sd));
    }

    const double rmsDiff = std::sqrt(diff2 / 4096.0);
    std::cout << "sn-vs-normalized-sd rms difference=" << rmsDiff
              << " peaks=" << peakA << "," << peakB << "\n";

    if (!std::isfinite(rmsDiff) || rmsDiff < 0.05)
        return 1;
    if (peakA > 1.25 || peakB > 1.25)
        return 1;
    return 0;
}
