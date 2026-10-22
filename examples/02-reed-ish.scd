// Nome6 -- reed-ish patch with shared breath noise.
(
SynthDef(\nomeReed, { |out = 0, freq = 146.83, amp = 0.16, gate = 1|
    var env = EnvGen.kr(Env.adsr(0.015, 0.18, 0.72, 0.8), gate, doneAction: 2);
    var attack = EnvGen.kr(Env([0, 1, 0.28], [0.035, 0.55], [4, -4]));
    var bright = EnvGen.kr(Env([0.985, 0.78, 0.90], [0.08, 0.7], [-4, 2]));
    var ctl, pm, op, sig;

    ctl = [
        [freq,        0, env,        bright, 0.08, 15000,
            attack * 0.045, attack * 0.025, attack * 0.015],
        [freq * 2.00, 0, env * 0.30, 0.82,   0.35, 13000,
            0, 0.010, 0.006],
        [freq * 3.01, 0, env * 0.18, 0.70,   0.75, 11000,
            0, 0.006, 0.004],
        [freq * 4.00, 0, env * 0.08, 0.55,   0.20,  9000,
            0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });
    pm[0][1] = 0.55;
    pm[0][2] = 0.18;
    pm[1][2] = 0.32;
    pm[0][0] = 0.08;

    op = Nome6.ar(ctl, pm, 2, -0.45, 4800);
    sig = op[0] + (op[1] * 0.12) + (op[2] * 0.05);
    sig = LeakDC.ar(sig) * amp;
    Out.ar(out, sig ! 2);
}).add;
)

x = Synth(\nomeReed, [\freq, 146.83]);
// x.set(\gate, 0);
