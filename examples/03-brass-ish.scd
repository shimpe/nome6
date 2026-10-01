// Nome6 -- brass-ish patch.
// The main operator's m follows the amplitude contour so the base waveform
// itself brightens during the attack, reducing how much PM is needed.
(
SynthDef(\nomeBrass, { |out = 0, freq = 110, amp = 0.15, gate = 1|
    var env = EnvGen.kr(Env.adsr(0.025, 0.22, 0.78, 0.55), gate, doneAction: 2);
    var attack = EnvGen.kr(Env([0, 1, 0.3], [0.035, 0.4], [4, -4]));
    var m = (0.38 + (attack * 0.60)).clip(0, 0.995);
    var ctl, pm, op, sig;

    ctl = [
        [freq,        0, env,        m,    0.05, 17000],
        [freq * 1.00, 0, env * 0.35, 0.72, 0.30, 15000],
        [freq * 2.00, 0, env * 0.18, 0.62, 0.65, 13000],
        [freq * 3.00, 0, env * 0.10, 0.50, 0.15, 11000],
        [0, 0, 0],
        [0, 0, 0]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });
    pm[0][1] = 0.35 + (attack * 0.8);
    pm[0][2] = attack * 0.25;
    pm[1][2] = 0.22;

    op = Nome6.ar(ctl, pm, 2);
    sig = op[0] + (op[1] * 0.10) + (op[3] * 0.03);
    sig = LeakDC.ar(sig) * amp;
    Out.ar(out, sig ! 2);
}).add;
)

x = Synth(\nomeBrass, [\freq, 110]);
// x.set(\gate, 0);
