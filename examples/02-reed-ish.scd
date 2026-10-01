// Nome6 -- reed-ish patch.
// High-m sn operators provide odd-harmonic structure before PM is applied.
(
SynthDef(\nomeReed, { |out = 0, freq = 146.83, amp = 0.16, gate = 1|
    var env = EnvGen.kr(Env.adsr(0.015, 0.18, 0.72, 0.8), gate, doneAction: 2);
    var bright = EnvGen.kr(Env([0.985, 0.78, 0.90], [0.08, 0.7], [-4, 2]));
    var ctl, pm, op, sig;

    ctl = [
        [freq,        0, env,        bright, 0.08, 15000],
        [freq * 2.00, 0, env * 0.30, 0.82,   0.35, 13000],
        [freq * 3.01, 0, env * 0.18, 0.70,   0.75, 11000],
        [freq * 4.00, 0, env * 0.08, 0.55,   0.20,  9000],
        [0, 0, 0],
        [0, 0, 0]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });
    pm[0][1] = 0.55;
    pm[0][2] = 0.18;
    pm[1][2] = 0.32;
    pm[0][0] = 0.08;

    op = Nome6.ar(ctl, pm, 2);
    sig = op[0] + (op[1] * 0.12) + (op[2] * 0.05);
    sig = LeakDC.ar(sig) * amp;
    Out.ar(out, sig ! 2);
}).add;
)

x = Synth(\nomeReed, [\freq, 146.83]);
// x.set(\gate, 0);
