// Nome6 -- unmistakable elliptic A/B/C test.
//
// Every two seconds the first operator switches between:
//   0: ordinary sine          (m = 0,        family = sn)
//   1: strongly elliptic sn   (m = 0.999999, family = sn)
//   2: strongly elliptic sd   (m = 0.999999, family = sd)
//
// m/family are audio-rate stepped signals here on purpose. This makes the
// test independent of control-rate input handling, but because m only changes
// at the state transitions the expensive elliptic coefficient table is not
// recomputed continuously.
(
{
    var trig = Impulse.ar(0.5);
    var state = PulseCount.ar(trig) % 3;
    var m = Select.ar(state, [
        DC.ar(0.0),
        DC.ar(0.999999),
        DC.ar(0.999999)
    ]);
    var family = Select.ar(state, [
        DC.ar(0.0),
        DC.ar(0.0),
        DC.ar(1.0)
    ]);

    var ctl = [
        [110, 0, 1, m, family, 18000],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
    ];

    var op = Nome6.ar(ctl, nil, 2);

    // Print the active state twice per second so the audible change can be
    // correlated with the parameter state in the post window.
    Poll.kr(Impulse.kr(0.5), A2K.kr(state), \nome6State);

    LeakDC.ar(op[0] * 0.15) ! 2
}.play;
)
