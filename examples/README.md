# Nome6 examples

- `00-fm7-compatible.scd` - ordinary sine PM using three-field FM7-style rows.
- `01-elliptic-morph.scd` - isolates `m` and the `sn` to normalized-`sd` family morph.
- `02-reed-ish.scd` - odd-rich elliptic operators used as a reed-like spectral prior.
- `03-brass-ish.scd` - envelopes the elliptic parameter with the note attack.
- `04-inharmonic-bell.scd` - irrational ratios plus elliptic operators and PM.
- `05-feedback.scd` - self-feedback and cyclic feedback at 4x internal oversampling.

For real-time polyphony, start with 2x oversampling. Use 4x when feedback or modulation is aggressive and CPU allows it.

`m` is best treated as a scalar/control-rate timbral parameter. Audio-rate `m` works, but changing it requires rebuilding elliptic Fourier coefficients and is therefore significantly more expensive than audio-rate modulation of `freq`, `amp`, `family`, or the PM matrix.
