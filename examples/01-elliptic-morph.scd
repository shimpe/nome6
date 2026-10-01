// Nome6 -- elliptic morph
//
// With the Release-build coefficient-update bug fixed, this continuously
// moves through the strongly audible high-m region and between sn / sd.

(
{
    var sweep = LFTri.kr(0.08).range(0, 1);
    var m = sweep.linexp(0, 1, 0.75, 0.999999);
    var family = SinOsc.kr(0.047, 0.4).range(0, 1);
    var ctl = [
        [110, 0, 1, m, family, 18000],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
    ];
    var op = Nome6.ar(ctl, nil, 2);

    LeakDC.ar(op[0] * 0.14) ! 2
}.play;
)

// Static A/B sanity check:
// Change the first row between:
// [110, 0, 1, 0,        0, 18000]   // sine
// [110, 0, 1, 0.999999, 0, 18000]   // strongly flattened sn
// [110, 0, 1, 0.999999, 1, 18000]   // pointed normalized sd
