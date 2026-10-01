(
// Nome6 ratio experiment
//
// Goal:
// Determine whether the periodic "accent" heard at high m comes from
// interactions between rich elliptic spectra and operator frequency ratios.
//
// Three otherwise-identical synths are compared:
//
// 1. integer ratios
// 2. slightly detuned ratios
// 3. irrational ratios
//
// Keep FreqScope open while running the sequence.

~makeNomeRatioTest = { |name, ratios|

    SynthDef(name, {
        |out = 0, freq = 110, amp = 0.10, gate = 1,
         m = 0.999999, family = 0.0|

        var e;
        var ctl;
        var pm;
        var op;
        var sig;

        // Same envelopes in all three versions.
        e = [
            EnvGen.kr(Env.adsr(0.005, 0.20, 0.75, 0.8, curve: -4), gate),
            EnvGen.kr(Env.adsr(0.003, 0.12, 0.45, 0.5, curve: -4), gate),
            EnvGen.kr(Env.adsr(0.002, 0.09, 0.28, 0.4, curve: -5), gate),
            EnvGen.kr(Env.adsr(0.004, 0.16, 0.20, 0.6, curve: -4), gate),
            EnvGen.kr(Env.adsr(0.002, 0.08, 0.12, 0.3, curve: -5), gate),
            EnvGen.kr(Env.adsr(0.006, 0.22, 0.09, 0.7, curve: -4), gate)
        ];

        ctl = [
            [freq * ratios[0], 0, e[0] * 0.75, m, family, 18000],
            [freq * ratios[1], 0, e[1] * 0.90, m, family, 18000],
            [freq * ratios[2], 0, e[2] * 0.62, m, family, 18000],
            [freq * ratios[3], 0, e[3] * 0.32, m, family, 18000],
            [freq * ratios[4], 0, e[4] * 0.28, m, family, 18000],
            [freq * ratios[5], 0, e[5] * 0.20, m, family, 18000]
        ];

        pm = Array.fill(6, {
            Array.fill(6, 0.0)
        });

        // Exactly the same PM topology in all cases.
        pm[0][1] = 2.4;
        pm[0][2] = 0.75;

        pm[1][2] = 1.15;
        pm[1][3] = 0.35;

        pm[2][4] = 0.55;

        // Mild operator feedback.
        pm[1][1] = 0.12;
        pm[2][2] = 0.08;

        op = Nome6.ar(
            ctl,
            pm,
            2
        );

        // Same audible carriers.
        sig =
            op[0]
            + (op[3] * 0.20)
            + (op[5] * 0.08);

        sig = LeakDC.ar(sig) * amp;

        DetectSilence.ar(
            sig,
            amp: 0.0001,
            time: 1.2,
            doneAction: 2
        );

        Out.ar(out, sig ! 2);
    }).add;
};


// ------------------------------------------------------------
// 1. Integer ratios
// ------------------------------------------------------------

~makeNomeRatioTest.(
    \nomeInteger,
    [
        1.0,
        2.0,
        3.0,
        0.5,
        5.0,
        4.0
    ]
);


// ------------------------------------------------------------
// 2. Slightly detuned ratios
// ------------------------------------------------------------

~makeNomeRatioTest.(
    \nomeDetuned,
    [
        1.0,
        2.003,
        3.011,
        0.497,
        5.019,
        4.007
    ]
);


// ------------------------------------------------------------
// 3. Irrational / strongly incommensurate ratios
// ------------------------------------------------------------

~makeNomeRatioTest.(
    \nomeIrrational,
    [
        1.0,
        2.sqrt,
        (1 + 5.sqrt) / 2,   // golden ratio
        3.sqrt / 2,
        5.sqrt,
        2.sqrt + 1
    ]
);

)


// ============================================================
// Play individually
// ============================================================


// Integer ratios
x = Synth(\nomeInteger, [
    \freq, 110,
    \m, 0.999999,
    \family, 0,
    \amp, 0.08
]);

x.set(\gate, 0);


// Slightly detuned
x = Synth(\nomeDetuned, [
    \freq, 110,
    \m, 0.999999,
    \family, 0,
    \amp, 0.08
]);

x.set(\gate, 0);


// Irrational
x = Synth(\nomeIrrational, [
    \freq, 110,
    \m, 0.999999,
    \family, 0,
    \amp, 0.08
]);

x.set(\gate, 0);


// ============================================================
// Automated A/B/C sequence
// ============================================================

(
Routine({

    "1/3 INTEGER RATIOS".postln;

    x = Synth(\nomeInteger, [
        \freq, 110,
        \m, 0.999999,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

    1.5.wait;


    "2/3 SLIGHTLY DETUNED RATIOS".postln;

    x = Synth(\nomeDetuned, [
        \freq, 110,
        \m, 0.999999,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

    1.5.wait;


    "3/3 IRRATIONAL RATIOS".postln;

    x = Synth(\nomeIrrational, [
        \freq, 110,
        \m, 0.999999,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

}).play;
)


// ============================================================
// Useful control experiment:
// repeat at m = 0 (sine limit)
// ============================================================

(
Routine({

    "SINE LIMIT: INTEGER".postln;

    x = Synth(\nomeInteger, [
        \freq, 110,
        \m, 0,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

    1.5.wait;


    "SINE LIMIT: DETUNED".postln;

    x = Synth(\nomeDetuned, [
        \freq, 110,
        \m, 0,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

    1.5.wait;


    "SINE LIMIT: IRRATIONAL".postln;

    x = Synth(\nomeIrrational, [
        \freq, 110,
        \m, 0,
        \family, 0,
        \amp, 0.08
    ]);

    4.wait;
    x.set(\gate, 0);

}).play;
)
