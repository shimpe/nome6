// Nome6 -- feedback exploration.
// Diagonal matrix entries are one-internal-sample feedback, as in FM7-style
// operator matrices. Oversampling changes that internal feedback rate too.
(
{
    var f = 73;
    var fb = SinOsc.kr(0.035).range(0.1, 3.2);
    var ctl = [
        [f,          0, 0.65, 0.97, 0.10, 16000],
        [f * 1.503,  0, 0.55, 0.91, 0.85, 15000],
        [f * 2.071,  0, 0.40, 0.84, 0.35, 14000],
        [f * 0.667,  0, 0.25, 0.75, 0.70, 12000],
        [0, 0, 0],
        [0, 0, 0]
    ];
    var pm = Array.fill(6, { Array.fill(6, 0.0) });
    var op, sig;

    pm[0][0] = fb;
    pm[1][1] = fb * -0.71;
    pm[0][1] = 1.7;
    pm[1][2] = -1.2;
    pm[2][0] = 0.9;
    pm[2][3] = 0.6;

    op = Nome6.ar(ctl, pm, 4);
    sig = op[0] + (op[1] * 0.2) + (op[2] * 0.12);
    Limiter.ar(LeakDC.ar(sig * 0.13), 0.9) ! 2
}.play;
)
