// Nome6 -- shared noise excitation / phase roughness / ellipticity roughness.
//
// One noise source is shared by all six operators. Per-operator row fields 7-9
// route it to additive output noise, phase, and m respectively.
(
SynthDef(\nomeNoiseDemo, { |out = 0, freq = 146.83, amp = 0.16, gate = 1|
    var env = EnvGen.kr(Env.adsr(0.01, 0.18, 0.72, 0.7), gate, doneAction: 2);
    var breath = EnvGen.kr(Env([0, 1, 0.32, 0.18], [0.025, 0.28, 1.5], [4, -4, -2]));
    var ctl, pm, op, sig;

    ctl = [
        // freq, phase, amp, m, family, bw, noiseAmp, noisePhase, noiseM
        [freq,        0, env * 0.72, 0.965, 0.08, 16000,
            breath * 0.055, breath * 0.035, breath * 0.018],

        [freq * 2.00, 0, env * 0.30, 0.84, 0.35, 14000,
            breath * -0.018, breath * 0.018, breath * -0.010],

        [freq * 3.01, 0, env * 0.13, 0.72, 0.70, 12000,
            0, breath * 0.012, breath * 0.006],

        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });
    pm[0][1] = 0.48;
    pm[0][2] = 0.15;
    pm[1][2] = 0.28;

    // noiseColor = -0.4 gives a softer breath spectrum.
    op = Nome6.ar(ctl, pm, 2, -0.4, 5200);

    sig = op[0] + (op[1] * 0.10) + (op[2] * 0.035);
    Out.ar(out, (LeakDC.ar(sig) * amp) ! 2);
}).add;
)

x = Synth(\nomeNoiseDemo, [\freq, 146.83]);
// x.set(\gate, 0);
