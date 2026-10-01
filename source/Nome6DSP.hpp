#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace nome6 {

constexpr int kNumOps = 6;
constexpr int kMaxPartials = 24;
constexpr double kPi = 3.14159265358979323846264338327950288;
constexpr double kHalfPi = 0.5 * kPi;
constexpr double kTwoPi = 2.0 * kPi;

inline double clampd(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

inline double lerpd(double a, double b, double t) {
    return a + (b - a) * t;
}

inline double wrap01(double x) {
    x -= std::floor(x);
    return x < 0.0 ? x + 1.0 : x;
}

// 11th-order sine polynomial on [0, pi/2], with quadrant reduction.
inline double sinQuarter(double x) {
    const double x2 = x * x;
    return x * (1.0 + x2 * (
        -1.0 / 6.0 + x2 * (
         1.0 / 120.0 + x2 * (
        -1.0 / 5040.0 + x2 * (
         1.0 / 362880.0 + x2 * (
        -1.0 / 39916800.0)))))
    );
}

inline double fastSin01(double phase) {
    phase = wrap01(phase);
    const double qf = phase * 4.0;
    const int q = int(qf);
    const double u = qf - double(q);
    switch (q & 3) {
    case 0: return  sinQuarter(u * kHalfPi);
    case 1: return  sinQuarter((1.0 - u) * kHalfPi);
    case 2: return -sinQuarter(u * kHalfPi);
    default:return -sinQuarter((1.0 - u) * kHalfPi);
    }
}

inline double agm(double a, double b) {
    // AGM converges quadratically. Six iterations is ample for double
    // precision over the parameter range used here.
    for (int i = 0; i < 8; ++i) {
        const double an = 0.5 * (a + b);
        const double bn = std::sqrt(a * b);
        a = an;
        b = bn;
        if (std::abs(a - b) <= 1.0e-14 * std::max(1.0, a))
            break;
    }
    return a;
}

struct HalfbandDecimator2 {
    static constexpr int kTaps = 31;
    std::array<double, kTaps> hist{};
    int pos = 0;
    int phase = 0;

    void reset() {
        hist.fill(0.0);
        pos = 0;
        phase = 0;
    }

    bool push(double x, double& y) {
        static constexpr int delay[17] = {
             0,  2,  4,  6,  8, 10, 12, 14, 15,
            16, 18, 20, 22, 24, 26, 28, 30
        };
        static constexpr double h[17] = {
            -4.9629878624e-05,
             6.4225402557e-04,
            -2.7344414175e-03,
             8.0203240103e-03,
            -1.9227637930e-02,
             4.1536704659e-02,
            -9.1224808140e-02,
             3.1304552743e-01,
             4.9998341448e-01,
             3.1304552743e-01,
            -9.1224808140e-02,
             4.1536704659e-02,
            -1.9227637930e-02,
             8.0203240103e-03,
            -2.7344414175e-03,
             6.4225402557e-04,
            -4.9629878624e-05
        };

        hist[pos] = x;
        pos = (pos + 1) % kTaps;
        phase ^= 1;
        if (phase != 0)
            return false;

        const int newest = (pos + kTaps - 1) % kTaps;
        double acc = 0.0;
        for (int k = 0; k < 17; ++k) {
            int idx = newest - delay[k];
            if (idx < 0) idx += kTaps;
            acc += hist[idx] * h[k];
        }
        y = acc;
        return true;
    }
};

// Full operator row:
// [freq, phase, amp, m, family, bandwidth]
// phase is initialization-rate and stored separately by the wrapper.
struct OpControl {
    double freq = 0.0;
    double amp = 0.0;
    double m = 0.0;
    double family = 0.0;
    double bandwidth = 0.0;
};

using Controls6 = std::array<OpControl, kNumOps>;
using Matrix6 = std::array<std::array<double, kNumOps>, kNumOps>;

struct PreparedElliptic {
    double m = 0.0;
    double q = 0.0;
    std::array<double, kMaxPartials> snCoeff{};
    std::array<double, kMaxPartials> sdCoeff{};
    // The Fourier series are truncated by the requested bandwidth.  These
    // tables normalize each truncated series to exactly +/-1 at quarter-cycle,
    // avoiding level jumps when the partial count changes with pitch.
    std::array<double, kMaxPartials + 1> snNorm{};
    std::array<double, kMaxPartials + 1> sdNorm{};
    int activePartials = 1;
};

inline void prepareElliptic(double mIn, PreparedElliptic& p) {
    // Values very close to one are useful musically, but m=1 itself has an
    // infinite real period and is not a periodic oscillator.
    const double m = clampd(mIn, 0.0, 0.999999);
    p.m = m;
    p.q = 0.0;
    p.snCoeff.fill(0.0);
    p.sdCoeff.fill(0.0);
    p.snNorm.fill(1.0);
    p.sdNorm.fill(1.0);
    p.activePartials = 1;

    if (m <= 1.0e-10) {
        // sn(u|0) = sin(u), and normalized sd has the same limit.
        p.snCoeff[0] = 1.0;
        p.sdCoeff[0] = 1.0;
        return;
    }

    const double k = std::sqrt(m);
    const double kPrime = std::sqrt(std::max(0.0, 1.0 - m));
    const double a = agm(1.0, kPrime);
    const double b = agm(1.0, k);

    // K(m) = pi / (2*AGM(1,sqrt(1-m)))
    // K'(m) = pi / (2*AGM(1,sqrt(m)))
    // q = exp(-pi*K'/K) = exp(-pi*a/b)
    const double q = std::exp(-kPi * a / b);
    p.q = q;
    const double scale = 4.0 * a / k; // 2*pi/(k*K)

    double r = std::sqrt(q);          // q^(n+1/2)
    double qOdd = q;                  // q^(2n+1)
    const double q2 = q * q;
    double snQuarter = 0.0;
    double sdQuarter = 0.0;

    for (int n = 0; n < kMaxPartials; ++n) {
        const double sign = (n & 1) ? -1.0 : 1.0;
        const double snDenom = std::max(1.0e-15, 1.0 - qOdd);
        const double sdDenom = std::max(1.0e-15, 1.0 + qOdd);

        // Frequency-normalized Jacobi sn, using zeta as oscillator phase.
        const double snC = scale * r / snDenom;

        // k' * sd has unit peak. Multiplying the DLMF sd series by k'
        // removes the 1/k' scale factor, leaving this coefficient sequence.
        const double sdC = sign * scale * r / sdDenom;

        p.snCoeff[n] = snC;
        p.sdCoeff[n] = sdC;
        if (std::max(std::abs(snC), std::abs(sdC)) > 1.0e-10)
            p.activePartials = n + 1;

        // sin((2n+1)*pi/2) alternates +/-1.
        snQuarter += sign * snC;
        sdQuarter += sign * sdC;
        p.snNorm[n + 1] = (std::abs(snQuarter) > 1.0e-15)
            ? (1.0 / snQuarter) : 1.0;
        p.sdNorm[n + 1] = (std::abs(sdQuarter) > 1.0e-15)
            ? (1.0 / sdQuarter) : 1.0;

        r *= q;
        qOdd *= q2;
    }
}

inline int partialCount(double freq,
                        double bandwidth,
                        double internalSampleRate) {
    const double af = std::abs(freq);
    if (af <= 1.0e-12)
        return kMaxPartials;

    const double nyquistGuard = internalSampleRate * 0.475;
    const double limit = (bandwidth > 0.0)
        ? std::min(std::abs(bandwidth), nyquistGuard)
        : nyquistGuard;

    if (limit < af)
        return 0;

    const int maxHarmonic = int(std::floor(limit / af));
    const int n = (maxHarmonic + 1) / 2; // odd harmonics 1,3,5,...
    return std::max(0, std::min(kMaxPartials, n));
}

inline double renderElliptic(double phase,
                             const PreparedElliptic& p,
                             int numPartials,
                             double family) {
    if (numPartials <= 0)
        return 0.0;

    const double f = clampd(family, 0.0, 1.0);
    const bool needSn = f < (1.0 - 1.0e-12);
    const bool needSd = f > 1.0e-12;

    const double s1 = fastSin01(phase);
    double sumSn = needSn ? (p.snCoeff[0] * s1) : 0.0;
    double sumSd = needSd ? (p.sdCoeff[0] * s1) : 0.0;

    if (numPartials > 1) {
        // One odd-harmonic recurrence services both elliptic families.
        const double s1sq = s1 * s1;
        const double cos2 = 1.0 - 2.0 * s1sq;
        double prev = s1;
        double cur = s1 * (3.0 - 4.0 * s1sq); // sin(3x)

        if (needSn) sumSn += p.snCoeff[1] * cur;
        if (needSd) sumSd += p.sdCoeff[1] * cur;

        for (int n = 2; n < numPartials; ++n) {
            const double next = 2.0 * cos2 * cur - prev;
            if (needSn) sumSn += p.snCoeff[n] * next;
            if (needSd) sumSd += p.sdCoeff[n] * next;
            prev = cur;
            cur = next;
        }
    }

    const double sn = needSn ? (sumSn * p.snNorm[numPartials]) : 0.0;
    const double sd = needSd ? (sumSd * p.sdNorm[numPartials]) : 0.0;
    if (!needSd) return sn;
    if (!needSn) return sd;
    return lerpd(sn, sd, f);
}

class Engine {
public:
    void configure(double hostSampleRate,
                   int oversample,
                   const std::array<double, kNumOps>& phaseRadians) {
        hostSampleRate_ = std::max(1.0, hostSampleRate);
        oversample_ = (oversample >= 4) ? 4 : ((oversample >= 2) ? 2 : 1);
        internalSampleRate_ = hostSampleRate_ * double(oversample_);

        for (int i = 0; i < kNumOps; ++i) {
            phase_[i] = wrap01(phaseRadians[i] / kTwoPi);
            lastM_[i] = std::numeric_limits<double>::quiet_NaN();
            prepareElliptic(0.0, prepared_[i]);
        }

        prevOut_.fill(0.0);
        for (auto& d : decim1_) d.reset();
        for (auto& d : decim2_) d.reset();
    }

    int oversample() const { return oversample_; }
    double internalSampleRate() const { return internalSampleRate_; }

    void processHostSample(const Controls6& ctl,
                           const Matrix6& modMatrix,
                           std::array<double, kNumOps>& out) {
        out.fill(0.0);

        struct RenderState {
            double dPhase = 0.0;
            double amp = 0.0;
            double family = 0.0;
            int numPartials = 0;
        };
        std::array<RenderState, kNumOps> rs{};

        for (int i = 0; i < kNumOps; ++i) {
            const double m = clampd(ctl[i].m, 0.0, 0.999999);
            if (!(m == lastM_[i])) {
                prepareElliptic(m, prepared_[i]);
                lastM_[i] = m;
            }
            rs[i].dPhase = ctl[i].freq / internalSampleRate_;
            rs[i].amp = ctl[i].amp;
            rs[i].family = ctl[i].family;
            rs[i].numPartials = std::min(
                partialCount(ctl[i].freq, ctl[i].bandwidth, internalSampleRate_),
                prepared_[i].activePartials);
        }

        struct Route { std::uint8_t dst; std::uint8_t src; double amount; };
        std::array<Route, kNumOps * kNumOps> routes{};
        int numRoutes = 0;
        for (int dst = 0; dst < kNumOps; ++dst) {
            for (int src = 0; src < kNumOps; ++src) {
                const double a = modMatrix[dst][src];
                if (a != 0.0) {
                    routes[numRoutes++] = Route{
                        std::uint8_t(dst), std::uint8_t(src), a
                    };
                }
            }
        }

        for (int sub = 0; sub < oversample_; ++sub) {
            std::array<double, kNumOps> pm{};
            std::array<double, kNumOps> next{};

            for (int r = 0; r < numRoutes; ++r) {
                const auto& route = routes[r];
                pm[route.dst] += route.amount * prevOut_[route.src];
            }

            for (int i = 0; i < kNumOps; ++i) {
                const double readPhase = wrap01(phase_[i] + pm[i] / kTwoPi);

                if (std::abs(rs[i].amp) <= 1.0e-15 || rs[i].numPartials <= 0) {
                    next[i] = 0.0;
                } else {
                    const double wave = renderElliptic(
                        readPhase, prepared_[i], rs[i].numPartials, rs[i].family);
                    next[i] = wave * rs[i].amp;
                }

                phase_[i] = wrap01(phase_[i] + rs[i].dPhase);
            }

            for (int i = 0; i < kNumOps; ++i) {
                if (oversample_ == 1) {
                    out[i] = next[i];
                } else if (oversample_ == 2) {
                    double y = 0.0;
                    if (decim1_[i].push(next[i], y))
                        out[i] = y;
                } else {
                    double y2 = 0.0;
                    if (decim1_[i].push(next[i], y2)) {
                        double y1 = 0.0;
                        if (decim2_[i].push(y2, y1))
                            out[i] = y1;
                    }
                }
            }

            prevOut_ = next;
        }
    }

private:
    double hostSampleRate_ = 48000.0;
    double internalSampleRate_ = 48000.0;
    int oversample_ = 1;

    std::array<double, kNumOps> phase_{};
    std::array<double, kNumOps> prevOut_{};
    std::array<double, kNumOps> lastM_{};
    std::array<PreparedElliptic, kNumOps> prepared_{};
    std::array<HalfbandDecimator2, kNumOps> decim1_{};
    std::array<HalfbandDecimator2, kNumOps> decim2_{};
};

} // namespace nome6
