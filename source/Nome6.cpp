#include "SC_PlugIn.h"
#include "Nome6DSP.hpp"

#include <array>
#include <cstdint>
#include <new>

static InterfaceTable* ft;

namespace {

constexpr int kCtlFields = 6;
constexpr int kCtlBase = 0;
constexpr int kModMatrixBase = nome6::kNumOps * kCtlFields; // 36
constexpr int kOversampleInput = kModMatrixBase + nome6::kNumOps * nome6::kNumOps; // 72
constexpr int kMaxAudioCtlRoutes = nome6::kNumOps * 5; // phase is init-rate
constexpr int kMaxAudioMatrixRoutes = nome6::kNumOps * nome6::kNumOps;

// ctl row: [freq, phase, amp, m, family, bandwidth]
enum CtlField {
    kFreq = 0,
    kPhase = 1,
    kAmp = 2,
    kM = 3,
    kFamily = 4,
    kBandwidth = 5
};

inline int ctlIndex(int op, CtlField field) {
    return kCtlBase + op * kCtlFields + int(field);
}

inline int matrixIndex(int dst, int src) {
    return kModMatrixBase + dst * nome6::kNumOps + src;
}

struct AudioCtlRoute {
    std::uint8_t op = 0;
    std::uint8_t field = 0;
    std::uint16_t input = 0;
};

struct AudioMatrixRoute {
    std::uint8_t dst = 0;
    std::uint8_t src = 0;
    std::uint16_t input = 0;
};

struct Nome6 : public Unit {
    nome6::Engine engine;
    std::array<AudioCtlRoute, kMaxAudioCtlRoutes> audioCtl{};
    std::array<AudioMatrixRoute, kMaxAudioMatrixRoutes> audioMod{};
    int numAudioCtl = 0;
    int numAudioMod = 0;
};

inline void readControlRowAtBlockStart(Nome6* unit,
                                       nome6::Controls6& ctl,
                                       int op) {
    ctl[op].freq      = IN0(ctlIndex(op, kFreq));
    ctl[op].amp       = IN0(ctlIndex(op, kAmp));
    ctl[op].m         = IN0(ctlIndex(op, kM));
    ctl[op].family    = IN0(ctlIndex(op, kFamily));
    ctl[op].bandwidth = IN0(ctlIndex(op, kBandwidth));
}

inline void setAudioControl(nome6::Controls6& ctl,
                            const AudioCtlRoute& route,
                            double value) {
    auto& c = ctl[route.op];
    switch (CtlField(route.field)) {
    case kFreq:      c.freq = value; break;
    case kAmp:       c.amp = value; break;
    case kM:         c.m = value; break;
    case kFamily:    c.family = value; break;
    case kBandwidth: c.bandwidth = value; break;
    case kPhase:     break;
    }
}

inline void readMatrixAtBlockStart(Nome6* unit,
                                   nome6::Matrix6& matrix) {
    for (int dst = 0; dst < nome6::kNumOps; ++dst) {
        for (int src = 0; src < nome6::kNumOps; ++src)
            matrix[dst][src] = IN0(matrixIndex(dst, src));
    }
}

inline void updateAudioMatrix(Nome6* unit,
                              nome6::Matrix6& matrix,
                              int sample) {
    for (int r = 0; r < unit->numAudioMod; ++r) {
        const auto& route = unit->audioMod[r];
        matrix[route.dst][route.src] = IN(route.input)[sample];
    }
}

void Nome6_next(Nome6* unit, int inNumSamples) {
    float* out[nome6::kNumOps] = {
        OUT(0), OUT(1), OUT(2), OUT(3), OUT(4), OUT(5)
    };

    nome6::Controls6 ctl{};
    nome6::Matrix6 modMatrix{};
    std::array<double, nome6::kNumOps> y{};

    for (int op = 0; op < nome6::kNumOps; ++op)
        readControlRowAtBlockStart(unit, ctl, op);
    readMatrixAtBlockStart(unit, modMatrix);

    for (int n = 0; n < inNumSamples; ++n) {
        for (int r = 0; r < unit->numAudioCtl; ++r) {
            const auto& route = unit->audioCtl[r];
            setAudioControl(ctl, route, IN(route.input)[n]);
        }
        updateAudioMatrix(unit, modMatrix, n);

        unit->engine.processHostSample(ctl, modMatrix, y);
        for (int op = 0; op < nome6::kNumOps; ++op)
            out[op][n] = float(y[op]);
    }
}

void Nome6_Ctor(Nome6* unit) {
    new (&unit->engine) nome6::Engine();
    unit->numAudioCtl = 0;
    unit->numAudioMod = 0;

    for (int op = 0; op < nome6::kNumOps; ++op) {
        for (int field = 0; field < kCtlFields; ++field) {
            if (field == kPhase)
                continue;
            const int idx = ctlIndex(op, CtlField(field));
            if (INRATE(idx) == calc_FullRate) {
                unit->audioCtl[unit->numAudioCtl++] = AudioCtlRoute{
                    std::uint8_t(op), std::uint8_t(field), std::uint16_t(idx)
                };
            }
        }
    }

    for (int dst = 0; dst < nome6::kNumOps; ++dst) {
        for (int src = 0; src < nome6::kNumOps; ++src) {
            const int idx = matrixIndex(dst, src);
            if (INRATE(idx) == calc_FullRate) {
                unit->audioMod[unit->numAudioMod++] = AudioMatrixRoute{
                    std::uint8_t(dst), std::uint8_t(src), std::uint16_t(idx)
                };
            }
        }
    }

    std::array<double, nome6::kNumOps> phaseRadians{};
    for (int op = 0; op < nome6::kNumOps; ++op)
        phaseRadians[op] = IN0(ctlIndex(op, kPhase));

    const float osIn = IN0(kOversampleInput);
    const int os = (osIn >= 3.0f) ? 4 : ((osIn >= 1.5f) ? 2 : 1);
    unit->engine.configure(SAMPLERATE, os, phaseRadians);

    SETCALC(Nome6_next);
    ClearUnitOutputs(unit, nome6::kNumOps);
}

void Nome6_Dtor(Nome6* unit) {
    unit->engine.~Engine();
}

} // namespace

PluginLoad(Nome6UGens) {
    ft = inTable;
    DefineDtorUnit(Nome6);
}
