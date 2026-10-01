// Nome6 -- ordinary sine PM when m = 0.
// Three-field rows are expanded to [freq, phase, amp, 0, 0, 0].
(
{
    var f = 110;
    var ctl = [
        [f,     0, 0.45],
        [f * 2, 0, 0.75],
        [f * 3, 0, 0.30],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
    ];
    var pm = Array.fill(6, { Array.fill(6, 0.0) });
    var op;

    pm[0][1] = 2.4;
    pm[1][2] = 0.9;
    pm[1][1] = 0.12;

    op = Nome6.ar(ctl, pm, 2);
    LeakDC.ar(op[0] * 0.18) ! 2
}.play;
)
