# Nome6

`Nome6` is a six-operator SuperCollider UGen inspired by the routing model of `FM7`, but its operators are frequency-normalized Jacobi elliptic oscillators instead of sine-only oscillators.

Each full operator row is:

```supercollider
[freq, phase, amp, m, family, bandwidth, noiseAmp, noisePhase, noiseM]
```

- `freq` - frequency in Hz.
- `phase` - initialization phase in radians.
- `amp` - operator amplitude; this scaled signal also feeds the modulation matrix.
- `m` - Jacobi parameter, `0 .. 0.999999`. `0` is the sine limit; values toward `1` produce increasingly strong odd-harmonic structure.
- `family` - `0` = frequency-normalized `sn`, `1` = peak-normalized `sd`; intermediate values morph between them.
- `bandwidth` - base elliptic-waveform harmonic bandwidth in Hz. `<= 0` uses the internal anti-alias ceiling.
- `noiseAmp` - additive amount of the shared filtered noise source at this operator output. Because operator outputs feed the PM matrix, this can also inject noise into downstream modulation paths.
- `noisePhase` - shared-noise phase jitter in radians.
- `noiseM` - bipolar perturbation depth around `m`. Nome6 prepares the two endpoint elliptic spectra once per host sample and interpolates their Fourier coefficients at the internal rate, avoiding AGM/nome recalculation at the noise rate.

Three-field FM7-style rows `[freq, phase, amp]` and legacy six-field elliptic rows are accepted; missing noise fields default to zero.


## Release-build coefficient update fix

Earlier builds used a NaN sentinel to force the first Jacobi coefficient update.
Because Release builds enable `/fp:fast` on MSVC and `-ffast-math` elsewhere,
the compiler could legally assume NaNs did not occur and leave every operator
stuck on the sine-limit coefficient table. Nome6 now uses an explicit validity
flag instead; the test suite includes an engine-level `m=0` versus `m≈1`
regression compiled with the same Release floating-point policy.

## Shared noise source

The extended call is:

```supercollider
Nome6.ar(ctlMatrix, modMatrix, oversample: 2, noiseColor: 0, noiseCutoff: 8000)
```

All six operators receive the same internally generated noise sample, with independent signed routing amounts in their rows. `noiseColor` ranges from `-1` (low-passed / brown-ish) through `0` (white) to `+1` (high-passed / bright). `noiseCutoff` sets the one-pole crossover/cutoff in Hz. The noise generator and its routing run at the selected internal oversampling rate.

The three routes are intentionally different:

- `noiseAmp` adds excitation/noise directly to the operator output.
- `noisePhase` creates phase/pitch roughness.
- `noiseM` creates spectral roughness by moving through ellipticity space.

Because the source is shared, signed routes can create correlated or anti-correlated excitation across operators.

## Modulation matrix

`modMatrix` is 6x6. Rows are destinations, columns are sources, and entries are radians of phase modulation. Diagonal entries are self-feedback. Operator-to-operator feedback uses the previous **internal** sample, so the complete matrix loop runs at the selected oversampling rate.

```supercollider
var pm = Array.fill(6, { Array.fill(6, 0.0) });
pm[0][1] = 2.0;   // operator 2 -> phase of operator 1
pm[1][1] = 0.2;   // operator 2 self-feedback
```

## Why "Nome6"?

The implementation renders Jacobi elliptic waveforms from their odd-harmonic Fourier series. Their coefficients are conveniently expressed using the Jacobi nome `q`. The nome and coefficient tables are prepared only when an operator's `m` changes.

This avoids a full special-function evaluation for every audio sample. The odd harmonics themselves are generated with one sine approximation plus a recurrence.

The `sn` and peak-normalized `sd` families use separate band-limited coefficient tables and share the same harmonic recurrence, so the `family` morph is inexpensive.

## Oversampling

```supercollider
Nome6.ar(ctlMatrix, modMatrix, 1)
Nome6.ar(ctlMatrix, modMatrix, 2) // recommended default
Nome6.ar(ctlMatrix, modMatrix, 4)
```

The full PM/feedback network is evaluated at the internal rate. 2x and 4x are decimated using 31-tap half-band FIR stages.

The base elliptic waveform is already explicitly band-limited. Oversampling is most useful for sidebands created by strong PM and feedback.

## Performance notes

- `m` at scalar/control rate is recommended. A changed base `m` requires an AGM/nome/coefficient update. Internal `noiseM` does not recalculate the nome at audio rate; it interpolates precomputed endpoint spectra.
- `family` is cheap to modulate, including at audio rate.
- Silent operators skip waveform evaluation.
- The PM matrix is collapsed to active routes once per host sample and reused across oversampling substeps.
- Release builds use `/O2 /fp:fast` with MSVC and `-O3 -ffast-math` elsewhere.

## Build

A SuperCollider source tree is required for the plug-in headers.

```text
cmake -S . -B build -DSC_PATH=C:/path/to/supercollider
cmake --build build --config Release
```

On Windows and macOS the server plug-in is named `Nome6.scx`; Linux uses `Nome6.so`.

For a normal extension install, keep the package structure:

```text
Nome6/
  Classes/Nome6.sc
  HelpSource/Classes/Nome6.schelp
  examples/
  plugins/Nome6.scx       # Windows/macOS
```

After copying/rebuilding the extension, recompile the SuperCollider class library and reboot the server.

## Tests

The standalone DSP tests cover:

- the sine limit at `m = 0`,
- quarter-cycle normalization,
- audible distinction between the `sn` and normalized-`sd` families,
- finite and level-consistent 1x/2x/4x rendering,
- shared additive/phase/ellipticity noise routing,
- shared-source correlation,
- a six-operator feedback benchmark.
