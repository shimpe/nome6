(
// FM7 vs Nome6 matched A/B comparison.
//
// Requirements:
// - FM7 available in SuperCollider
// - Nome6 installed and rebuilt from the fixed source
//
// The two SynthDefs use:
// - the same frequencies / ratios
// - the same operator amplitude envelopes
// - the same 6x6 phase-modulation matrix
// - the same output mix
//
// The only intentional synthesis difference is:
// FM7   -> sine operators
// Nome6 -> Jacobi elliptic operators
//
// Start with \m = 0 on Nome6: it should approach the FM7 version.
// Then try \m = 0.95 .. 0.99999 and \family = 0 .. 1.

~makeEnv = { |gate|
    [
        EnvGen.kr(Env.adsr(0.005, 0.18, 0.72, 0.8, curve: -4), gate),
        EnvGen.kr(Env.adsr(0.003, 0.11, 0.38, 0.45, curve: -4), gate),
        EnvGen.kr(Env.adsr(0.002, 0.08, 0.20, 0.30, curve: -5), gate),
        EnvGen.kr(Env.adsr(0.004, 0.15, 0.16, 0.55, curve: -4), gate),
        EnvGen.kr(Env.adsr(0.002, 0.07, 0.10, 0.25, curve: -5), gate),
        EnvGen.kr(Env.adsr(0.006, 0.22, 0.08, 0.65, curve: -4), gate)
    ]
};


// --------------------
// Conventional FM7
// --------------------

SynthDef(\fm7Matched, {
    |out = 0, freq = 110, amp = 0.12, gate = 1|

    var e = ~makeEnv.(gate);
    var ctl;
    var pm;
    var op;
    var sig;

    // Same ratios and amplitude envelopes as Nome6 below.
    ctl = [
        [freq * 1.00, 0, e[0] * 0.75],
        [freq * 2.00, 0, e[1] * 0.90],
        [freq * 3.01, 0, e[2] * 0.62],
        [freq * 0.50, 0, e[3] * 0.32],
        [freq * 5.03, 0, e[4] * 0.28],
        [freq * 1.41421356, 0, e[5] * 0.20]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });

    // destination row, source column
    pm[0][1] = 2.4;   // op2 -> op1
    pm[0][2] = 0.75;  // op3 -> op1

    pm[1][2] = 1.15;  // op3 -> op2
    pm[1][3] = 0.35;  // op4 -> op2

    pm[2][4] = 0.55;  // op5 -> op3

    // mild feedback
    pm[1][1] = 0.12;
    pm[2][2] = 0.08;

    op = FM7.ar(ctl, pm);

    // Same audible carriers as Nome6.
    sig =
        op[0]
        + (op[3] * 0.20)
        + (op[5] * 0.08);

    sig = LeakDC.ar(sig);
    sig = sig * amp;

    DetectSilence.ar(sig, 0.0001, 1.2, doneAction: 2);

    Out.ar(out, sig ! 2);
}).add;


// --------------------
// Matched Nome6
// --------------------

SynthDef(\nome6Matched, {
    |out = 0, freq = 110, amp = 0.12, gate = 1,
     m = 0.995, family = 0.0, bandwidth = 18000|

    var e = ~makeEnv.(gate);
    var ctl;
    var pm;
    var op;
    var sig;

    ctl = [
        [freq * 1.00, 0, e[0] * 0.75, m, family, bandwidth],
        [freq * 2.00, 0, e[1] * 0.90, m, family, bandwidth],
        [freq * 3.01, 0, e[2] * 0.62, m, family, bandwidth],
        [freq * 0.50, 0, e[3] * 0.32, m, family, bandwidth],
        [freq * 5.03, 0, e[4] * 0.28, m, family, bandwidth],
        [freq * 1.41421356, 0, e[5] * 0.20, m, family, bandwidth]
    ];

    pm = Array.fill(6, { Array.fill(6, 0.0) });

    // Exactly the same matrix as FM7.
    pm[0][1] = 2.4;
    pm[0][2] = 0.75;

    pm[1][2] = 1.15;
    pm[1][3] = 0.35;

    pm[2][4] = 0.55;

    pm[1][1] = 0.12;
    pm[2][2] = 0.08;

    // Keep oversampling fixed at SynthDef construction time.
    // Nome6 treats this as an initialization-time topology choice.
    op = Nome6.ar(ctl, pm, 2);

    sig =
        op[0]
        + (op[3] * 0.20)
        + (op[5] * 0.08);

    sig = LeakDC.ar(sig);
    sig = sig * amp;

    DetectSilence.ar(sig, 0.0001, 1.2, doneAction: 2);

    Out.ar(out, sig ! 2);
}).add;
)


// ============================================================
// Tests
// ============================================================


// A. FM7 reference
x = Synth(\fm7Matched, [\freq, 110, \amp, 0.10]);

// release
x.set(\gate, 0);


// B. Nome6 in sine limit.
// This should be quite close to the FM7 reference.
x = Synth(\nome6Matched, [
    \freq, 110,
    \amp, 0.10,
    \m, 0.0,
    \family, 0.0
]);

x.set(\gate, 0);


// C. Same patch, strongly elliptic sn operators.
x = Synth(\nome6Matched, [
    \freq, 110,
    \amp, 0.10,
    \m, 0.995,
    \family, 0.0
]);

x.set(\gate, 0);


// D. Push much closer to m = 1.
x = Synth(\nome6Matched, [
    \freq, 110,
    \amp, 0.08,
    \m, 0.999999,
    \family, 0.0
]);

x.set(\gate, 0);


// E. Same high-m state, move from sn toward sd.
x = Synth(\nome6Matched, [
    \freq, 110,
    \amp, 0.08,
    \m, 0.999999,
    \family, 1.0
]);

x.set(\gate, 0);


// ============================================================
// Automated A/B sequence
// ============================================================

(
Routine({
    var note = 110;

    "1/4  FM7 sine reference".postln;
    x = Synth(\fm7Matched, [
        \freq, note,
        \amp, 0.10
    ]);
    2.wait;
    x.set(\gate, 0);
    1.2.wait;

    "2/4  Nome6 m=0 (sine limit)".postln;
    x = Synth(\nome6Matched, [
        \freq, note,
        \amp, 0.10,
        \m, 0,
        \family, 0
    ]);
    2.wait;
    x.set(\gate, 0);
    1.2.wait;

    "3/4  Nome6 high-m sn".postln;
    x = Synth(\nome6Matched, [
        \freq, note,
        \amp, 0.08,
        \m, 0.999999,
        \family, 0
    ]);
    2.wait;
    x.set(\gate, 0);
    1.2.wait;

    "4/4  Nome6 high-m sd".postln;
    x = Synth(\nome6Matched, [
        \freq, note,
        \amp, 0.08,
        \m, 0.999999,
        \family, 1
    ]);
    2.wait;
    x.set(\gate, 0);
}).play;
)
