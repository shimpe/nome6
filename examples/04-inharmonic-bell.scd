// Nome6 -- inharmonic bell / struck-metal family.
// Elliptic operators already contain odd harmonic ladders; irrational ratios
// and PM turn those ladders into dense inharmonic clusters.
(
{
    var f = 164.81;
    var hit = EnvGen.kr(Env.perc(0.002, 4.5));
    var fast = EnvGen.kr(Env.perc(0.001, 0.35));
    var ctl = [
        [f,          0, 0.50 * hit,  0.88, 0.20, 17000],
        [f * 1.4142, 0, 0.65 * fast, 0.94, 0.85, 18000],
        [f * 2.756,  0, 0.42 * fast, 0.90, 0.55, 18000],
        [f * 4.107,  0, 0.22 * hit,  0.78, 0.10, 16000],
        [f * 5.431,  0, 0.15 * hit,  0.96, 0.95, 18000],
        [f * 7.013,  0, 0.10 * hit,  0.70, 0.40, 15000]
    ];
    var pm = Array.fill(6, { Array.fill(6, 0.0) });
    var op, sig;

    pm[0][1] = 2.3;
    pm[0][2] = -1.1;
    pm[1][2] = 1.4;
    pm[3][4] = 0.7;
    pm[0][0] = 0.18;

    op = Nome6.ar(ctl, pm, 4);
    sig = op[0] + (op[3] * 0.35) + (op[4] * 0.20) + (op[5] * 0.12);
    sig = LeakDC.ar(sig * 0.18);
    sig ! 2
}.play;
)
