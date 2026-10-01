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
    return maxErr < 1.0e-6 ? 0 : 1;
}
