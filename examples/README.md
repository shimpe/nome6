# Nome6 examples

- `00-fm7-compatible.scd` - sine-limit compatibility using three-field rows.
- `01-elliptic-morph.scd` - direct `m` and `sn`/`sd` family morph.
- `02-reed-ish.scd` - reed-like patch with shared breath excitation, phase jitter, and subtle `m` roughness.
- `03-brass-ish.scd` - brass-like patch with a restrained noisy attack.
- `04-inharmonic-bell.scd` - irrational ratios and elliptic PM for struck-metal spectra.
- `05-feedback.scd` - PM/self-feedback exploration.
- `06-noise-excitation.scd` - focused demo of the three shared-noise routing modes.

Full operator row:

```supercollider
[freq, phase, amp, m, family, bandwidth, noiseAmp, noisePhase, noiseM]
```

The global call is:

```supercollider
Nome6.ar(ctl, pm, oversample, noiseColor, noiseCutoff)
```

Old three-field and six-field rows still work; their noise routes default to zero.
