# GTUltra SID engine migration plan (authoritative)

Status: active execution plan for the `new-emulators` branch.
Companion tracker: `sid-engine-migration-tracking.md` (sequential step checklist).
This plan is **self-contained** (M4/M5 libresidfp detail and the API/source anchors were
folded in from the earlier analysis docs). Those prior docs - `resid-update-plan.md` (old
execution plan), `emulator-migration-proposal.md` and `new-emulator-versions.md` (analysis
records) - were removed from the tree; they remain in git history if the full "why" is needed.

Scope of the near-term work: introduce a future-proof engine wrapper, then bring the
new integer reSID (`_upstream/resid`, VICE mirror) in behind it. libresidfp is planned
but staged last. Every claim below was verified against the source in this repo and the
clones in `_upstream/` (see the review notes that produced this file).

---

## 0. Why this file exists (corrections to the earlier `resid-update-plan.md`)

The earlier `resid-update-plan.md` (now in git history) was directionally right on staging
but had defects that make it non-future-proof and non-building if followed literally:

1. **No wrapper.** It adds a third pointer family (`sidvice[4]`) and extends the
   `if (sid)… if (sidfp)…` ladder in `gsid.cpp`. That is O(engines x callsites) of
   hand-maintained conditionals (~6 clock sites in `sid_fillbuffer` alone,
   `gsid.cpp:307..426`, plus the whole `sid_init` block `gsid.cpp:68..262`). A 4th
   engine triples it again. This plan introduces `ISidEngine` **first** so each new
   engine is one new `.cpp` + one factory line.
2. **`version.o` breaks the build.** `version.cc` -> `siddefs.h` compiles
   `resid_version_string = VERSION;` (`_upstream/resid/siddefs.h.in:88`); `VERSION` is
   defined nowhere in this build, and `resid_version_string` is referenced nowhere in
   GTUltra. Drop the object.
3. **`-Isrc/resid-vice` is wrong and unsafe.** The build runs in `src/` (objects are
   `resid/sid.o`, flags are `-Ibme -Iasm`). No include path is needed at all
   (`gsid.cpp` already reaches the old engine via `#include "resid/sid.h"`, and
   intra-engine `#include "voice.h"` resolves via GCC's automatic same-dir search).
   Adding `-Iresid-vice` globally would also shadow one of the three `sid.h` files.
4. **`resid-config.h` missing.** `_upstream/resid/sid.h:25` includes it; the plan only
   mentioned `siddefs.h`.
5. **"Near-1:1 filter port" is overstated.** With `NEW_8580_FILTER 1` the patch target
   is `filter8580new.h/.cc` (state `Vhp,Vbp,Vbp_x,Vbp_vc,Vlp,Vlp_x,Vlp_vc,ve,v1..v3`,
   `clock(int,int,int)`), not the classic `filter.h` the old `src/resid` patched with a
   float `LR` path. It is a re-implementation onto a different filter. Two independent
   `Filter` objects is the clean approach and is what this plan uses.

---

## 1. Verified engine boundary (what the wrapper must cover)

The only things that cross out of `gsid.cpp` are the C API in `gsid.h`:
`sid_init`, `sid_fillbuffer`, `sid_getorder`, `sid_debug`, plus globals `sidreg*[]`
and `filterparams`. Nothing outside `gsid.cpp` references a `SID`/`SIDFP` type
(verified: `read`, `write`, `get_filter`, `debugCount` have zero hits outside the
engine dirs). So `ISidEngine` is a **private refactor inside `gsid.cpp` with zero blast
radius** on `gsound.c` or the rest of the app.

Key invariants the boundary imposes:

- **Write-timing stays above the interface.** `SIDWAVEDELAY` / `residdelay` /
  `SIDWRITEDELAY` chunked clocking (`gsid.cpp:304..385`) is app logic. The interface is
  clocked in deltas via a by-reference cycle count; it must not absorb the timing loop.
- **Split-buffer layout.** Each chip writes L into `buf[0..n-1]` and R into
  `buf[bufferHalfSize..]` (the `MIXBUFFERSIZE` half split; `gsound.h`). `clockStereo`
  must honour this exactly.
- **Equal-sample-count assumption.** The fill loop advances all four buffers by one
  `result` (`gsid.cpp:322..326, 380..384`). This holds only if all four chips run the
  **same backend with identical sampling params**. The factory must enforce that (see
  M1.4); do not allow mixed backends across `chip[0..3]`.
- **`sid_debug()`** returns `sid->debugCount`, a GTUltra field on the old integer engine
  only. It must move behind the interface (`debug()`), else it fails to compile once
  the old engine is gone.

---

## 2. Repository layout

```
src/
  engines/                    # NEW wrapper layer (own code, unit-testable)
    isid_engine.h             # interface; includes NO engine headers
    engine_factory.h/.cpp     # backend enum -> ISidEngine*; enforces single backend
    engine_resid_old.cpp      # wraps global ::SID     (M1)
    engine_residfp_old.cpp    # wraps global ::SIDFP   (M1)
    engine_resid_vice.cpp     # wraps reSID::SID       (M2/M3, compile-gated)
    engine_residfp_lib.cpp    # wraps reSIDfp::SID     (M4/M5, compile-gated)
  resid/                      # old integer reSID   (kept; default)
  resid-fp/                   # old float reSID-FP  (kept; default)
  resid-vice/                 # new integer reSID   (M2; built only if WITH_RESID_VICE=1)
  residfp-lib/                # new float libresidfp (M4; built only if WITH_RESIDFP_LIB=1)
tests/
  unit/                       # NEW: first unit suite (see Testing)
    harness/                  # single-header C/C++ harness (Unity or greatest)
    test_sid_engine.cpp       # wrapper contract, pan math, center==mono
    Makefile / rule in makefile.common ("make test")
  fixtures/                   # *.sng + golden-*.wav / golden-*.sha256
  integration/                # existing smoke.sh, check-goatdata.sh (+ headless render)
patches/
  resid-vice-truestereo.patch
  libresidfp-truestereo.patch
notes/new-emu/                # this plan + analysis records
```

Rationale: the wrapper layer is **own code** and lives in `src/engines/`, separate from the
vendored engine dirs. The vendored `resid*/` dirs stay individually replaceable and are
never unit-tested internally (see Testing). `resid-vice/` and `residfp-lib/` are physically
present but excluded from the default build via compile gates (§3).

## 3. Rollout: new engines as opt-in, unstable, slow-cook features

Requirement: keep the old engines as the shipped default; bring reSID-vice in slowly and
release it initially as unstable. Two independent gates, layered, plus a maturity ladder.

**M1 is NOT gated.** The `ISidEngine` refactor wraps the *existing* engines and must be
bit-identical (§M1.5). It becomes the stable base everything else sits on. Only the *new*
engines are gated.

### 3.1 Compile-time gate (primary isolation)

Mirror the existing `SANITIZE` pattern in `src/makefile.common`:

```makefile
ifeq ($(WITH_RESID_VICE),1)
CXXFLAGS += -DWITH_RESID_VICE
RESID_VICE_OBJS = resid-vice/sid.o resid-vice/voice.o resid-vice/wave.o \
  resid-vice/envelope.o resid-vice/filter8580new.o resid-vice/dac.o \
  resid-vice/extfilt.o resid-vice/pot.o engines/engine_resid_vice.o
endif
```

Link `$(RESID_VICE_OBJS)` into the `EXE` recipes; `engine_factory.cpp` guards the
reSID-vice branch with `#ifdef WITH_RESID_VICE`. Consequences:
- Default `make {mac,linux,win}-build` compiles and links **nothing** new. Release binaries
  are byte-for-byte unaffected by in-progress engine work. "Unstable" is literally absent
  from shipped builds.
- Opt-in build: `make linux-build WITH_RESID_VICE=1` (the flag flows to the sub-make exactly
  as `SANITIZE=1` does).
- CI gets one **dedicated job** that builds `WITH_RESID_VICE=1` (+ `SANITIZE=1`) so the new
  code keeps compiling and is sanitized, without touching the three default-build jobs.

### 3.2 Runtime gate (once compiled in)

Even in a `WITH_RESID_VICE=1` binary the new engine is not reachable unless opted in:
- Append a new positional `.cfg` key `sidbackend` (append-only; loader tolerates absence and
  keeps the `interpolate>>1` default). See M6.
- `engine_factory` maps `sidbackend` -> backend. If the requested backend was **not compiled
  in**, fall back to the legacy engine and warn (never corrupt config, never hard-fail).
- Surface the active backend as "experimental/unstable" in the status area (M6) so users know.

### 3.3 Maturity ladder

- **Phase A (default OFF, opt-in build):** `WITH_RESID_VICE` off in releases; opt-in builds +
  CI gated job keep it alive. Golden/statistical tests run in the gated job only.
- **Phase B (compiled in, runtime OFF):** flip default compile to on, but `sidbackend`
  defaults to legacy; UI labels reSID-vice "experimental". Ships to users as opt-in.
- **Phase C (runtime default for 8580):** once corpus passes, make reSID-vice the default for
  8580; old engines remain selectable.
- **Phase D (M7):** remove old engines.

libresidfp follows the same ladder with its own `WITH_RESIDFP_LIB` gate, staged after
reSID-vice.

## M0 - Test scaffolding (before any engine touch)

Objective: make change observable. No engine change.
1. Add headless render mode (Testing §Integration): load `.sng`, select
   `{backend, chip, stereoMode}`, render N frames via the existing
   `sid_fillbuffer`/`ExportSIDToPCMFile` path, write a WAV, no window/timer.
2. Stand up `tests/unit/` harness + `make test` (first unit suite for the project).
3. Capture golden WAVs/metrics for the current build (both legacy backends, full corpus).
This is independently useful (regression fixtures) and unblocks M1's bit-identical check.

## M1 - Introduce `ISidEngine` (do this first, no new engine yet)

Objective: `gsid.cpp` drives `ISidEngine* chip[4]` through wrappers around the two
**existing** engines. Behaviour-identical, verified against golden WAVs. Highest
leverage, low risk. This is the future-proofing spine.

### M1.1 Interface (header includes NO engine headers)

```cpp
// src/engines/isid_engine.h
class ISidEngine {
public:
  enum Model    { MODEL_6581, MODEL_8580 };
  enum Sampling { FAST, INTERPOLATE, RESAMPLE };
  virtual ~ISidEngine() = default;
  virtual void reset() = 0;
  virtual void setChipModel(Model) = 0;
  virtual void setSamplingParams(double clockHz, Sampling, double sampleHz) = 0;
  virtual void write(int reg, unsigned char value) = 0;
  // L -> buf[0..n-1], R -> buf[bufferHalfSize..]; delta decremented; returns samples.
  virtual int  clockStereo(int& delta, short* buf, int n, int bufferHalfSize) = 0;
  virtual void enableFilter(bool) = 0;
  virtual int  debug() { return 0; }            // replaces sid->debugCount
  // libresidfp-only tuning; no-op elsewhere (do NOT add legacy distortion setters):
  virtual void setFilter6581Curve(double) {}
  virtual void setFilter6581Range(double) {}
  virtual void setFilter8580Curve(double) {}
  virtual void setCombinedWaveforms(int) {}
};
```

Rules that keep it future-proof:
- **Neutral enums** (`Model`, `Sampling`), each wrapper maps to its engine's own enum.
  Never pass an engine's raw `SAMPLE_*` value through the interface.
- **Header firewall.** Only the `.cpp` wrappers include `resid/…`, `resid-fp/…`,
  `resid-vice/…`, `residfp-lib/…`. All three engine headers are named `sid.h`/`SID.h`;
  keeping them out of `isid_engine.h` avoids collisions and firewalls compile times.
- **No legacy `FILTERPARAMS` in the interface.** The old-FP wrapper reads the global
  `filterparams` in its own setup (transitional). The interface exposes only the
  principled setters, so dead concepts never enter the abstraction.

### M1.2 Wrappers for the two existing engines

```
src/engines/
  isid_engine.h
  engine_resid_old.cpp    # wraps global ::SID   (maps FAST/INTERPOLATE; RESAMPLE->INTERPOLATE)
  engine_residfp_old.cpp  # wraps global ::SIDFP (applies filterparams in ctor/setup)
```

Each `clockStereo` forwards to the engine's existing patched
`clock(delta, buf, n, bufferHalfSize)` (both old engines already emit the split
buffer). `debug()` on the old integer wrapper returns `debugCount`.

### M1.3 Collapse `gsid.cpp`

`sid_fillbuffer`'s six laddered clock blocks become one loop:

```cpp
short* buf[4] = { lptr, rptr, lptr2, rptr2 };
for (int i = 0; i < 4; i++) {
    int d = tdelta2;                                  // per-chip copy, as today
    result = chip[i]->clockStereo(d, buf[i], samples, bufferHalfSize);
}
```

`sid_init` becomes: pick backend -> factory builds `chip[0..3]` -> loop
`reset()/setChipModel()/setSamplingParams()/enableFilter()`. `sid_debug()` returns
`chip[0]->debug()`.

### M1.4 Factory + single-backend constraint

```c
enum SidBackend { BACKEND_LEGACY_RESID, BACKEND_LEGACY_RESIDFP,
                  BACKEND_VICE_RESID,  BACKEND_LIBRESIDFP };
```

Factory builds four instances of **one** backend (never mixed) so the equal-count
assumption in the fill loop holds. Decode today's `interpolate` into
(backend, sampling): `interpolate>>1` -> legacy engine; `interpolate&1` -> sampling.

### M1.5 Verify

Golden-WAV compare against pre-refactor build must be **bit-identical** (pure refactor,
same engines). This validates the seam before any engine changes.

---

## M2 - VICE reSID, mono, behind the wrapper

Objective: add `src/resid-vice/`, build it, wrap it with center-pan output. Prove the
measured-8580 model vs golden 8580 tunes. No TrueStereo yet.

### M2.1 Source tree
- Copy `_upstream/resid/*` to `src/resid-vice/` (keep `.cc`). `_upstream/resid` stays
  pristine reference.

### M2.2 Generated + configured files (determinism-safe, no Perl in CI)
- Run `samp2src.pl` once, commit the eight `{...}` fragments
  `wave{6581,8580}{__ST,_P_T,_PS_,_PST}.h`. They are `#include`d into
  `model_wave[2][8][1<<12]` inside `wave.cc:49..70`, so only `wave.o` is built (no
  separate `wave6581_PST.o`).
- From `siddefs.h.in` commit `src/resid-vice/siddefs.h` with:
  `RESID_INLINING 1`, `RESID_INLINE inline`, `RESID_BRANCH_HINTS 1`, `NEW_8580_FILTER 1`,
  `HAVE_BOOL 1`, `HAVE_BUILTIN_EXPECT 1`, `HAVE_LOG1P 1`. (Keep the file's typedefs,
  enums, branch-hint macros and `namespace reSID` block; only the `@...@` slots are
  substituted. Note `filter8580new.cc:233` `#undef HAS_LOG1P` and ships its own, so the
  `HAVE_LOG1P` value is effectively moot.)
- Commit `src/resid-vice/resid-config.h` (one-line wrapper including `siddefs.h`).

### M2.3 Build integration (`src/makefile.common`)
- **No include path.** `.cc -> .o` uses make's built-in C++ rule with inherited
  `-MMD -MP`, so `.d` files are produced.
- Add to `-include`: `resid-vice/*.d`.
- Add to `clean`: `resid-vice/*.o resid-vice/*.d`.
- Object list (both `gtultra` and `gt2reloc` link resid):
  `resid-vice/sid.o resid-vice/voice.o resid-vice/wave.o resid-vice/envelope.o
   resid-vice/filter8580new.o resid-vice/dac.o resid-vice/extfilt.o resid-vice/pot.o`
  Exclude classic `filter.cc` (one filter only). **Do not** list `version.o` (see 0.2).

### M2.4 Wrapper `engine_resid_vice.cpp`
- Include `resid-vice/sid.h`.
- `clockStereo`: call mono `s.clock(delta, buf, n, /*interleave*/1)`, then duplicate
  each produced sample into `buf[bufferHalfSize + i]`. Handles resample short-counts
  via the returned count.
- Map `Sampling`: `FAST->SAMPLE_FAST`, `INTERPOLATE->SAMPLE_INTERPOLATE`,
  `RESAMPLE->SAMPLE_RESAMPLE`. Map `Model`: `MODEL_8580->MOS8580`.

### M2.5 Verify
- `make linux-build` and `make mac-build` link and play.
- `make SANITIZE=1 linux-build` (UBSan on the new filter math).
- Compare 8580 tunes vs golden; expect improvement, not equality.

---

## M3 - VICE reSID, TrueStereo (two-filter patch)

Objective: forward-port per-voice panning as a **filter-input split**, preserving
hard-sync and ring-mod (both computed in `wave.synchronize()` before the filter).

Patch `src/resid-vice/` (carry as `patches/resid-vice-truestereo.patch`):

1. **Pan storage** (`wave.h`/`wave.cc`): add `reg8 pan; reg8 getPan();`;
   `writePW_HI` sets `pan = (pw_hi >> 4) & 0xf;`; `reset()` sets `pan = 0;`.
2. **Two filter chains** (`sid.h`): `Filter filter[2];`, `ExternalFilter extfilt[2];`
   (two objects, not `[2]` internal state, because `filter8580new` state is rich). Fan
   `set_chip_model/enable_filter/enable_external_filter/set_voice_mask/reset/input` to
   both `[0]` and `[1]`.
3. **Pan split in `SID::clock()` (both overloads)**: after `set_waveform_output`, for
   each voice compute the exact legacy mapping (`src/resid/sid.cpp:683..692`):
   `pan = getPan(); if (pan==0xf) pan--; pan %= 15; fpan = pan/14.0f;
    gL = 1-fpan; gR = fpan;` then feed
   `filter[0].clock((int)(s0*gL),(int)(s1*gL),(int)(s2*gL))` and the `gR` set into
   `filter[1]`, then `extfilt[0].clock(filter[0].output())`, same for `[1]`.
   Note: `filter8580new::clock` takes `int` and adds its own DC/noise per channel, so a
   centered tune is close-but-not-bit-identical to mono. Acceptance is feature
   preservation, not sample equality.
4. **Stereo `output(short* L, short* R)`** and split-buffer
   `clock(delta, buf, n, bufferHalfSize, interleave=1)`: reimplement
   `clock_fast/interpolate/resample` to write `buf[s]`=L and `buf[bufferHalfSize+s]`=R;
   for resample add second ring buffer `sampleR`.
5. Point `engine_resid_vice.cpp::clockStereo` at the new split `clock`.

Verify: pan positions preserved; a hard-sync + ring-mod tune still correct; unit checks
per M-tests below.

---

## M4 - libresidfp, mono, behind the wrapper (compile-gated `WITH_RESIDFP_LIB`)

Objective: add `src/residfp-lib/`, build it, wrap it center-pan. Prove the 6581 analog
character vs golden 6581 tunes. Staged after reSID-vice (harder engine).

### M4.1 Source + config (no generator, simpler than reSID)
- Copy `_upstream/libresidfp` -> `src/residfp-lib/`. Release v1.1.1, GPL-2.0-or-later,
  standalone (no libsidplayfp deps at build), needs a C++11 compiler.
- No build-time generation: combined waveforms (`WaveformCalculator`), filter model
  (`FilterModelConfig*`), and DAC (`Dac`) are all computed at runtime. Only synthesize
  `siddefs-fp.h` from `siddefs-fp.h.in` (5 substitutions: `RESIDFP_BRANCH_HINTS`,
  `HAVE_BUILTIN_EXPECT`, `RESIDFP_INLINING`, `RESIDFP_INLINE`, `PACKAGE_VERSION`) and compile
  `src/*.cpp` + `src/resample/*.cpp`. No Perl.

### M4.2 Verified upstream API (`_upstream/libresidfp/src/SID.h`)
- `setChipModel`, `setCombinedWaveforms`, `reset`, `input`, `peek`/`read`/`write`,
  `setSamplingParameters(clock, method, sample)`, `clock(unsigned cycles, int16_t* buf)` and
  `clock(int16_t* buf, int bufSize)`, `clockSilent`, `setFilter6581Curve`,
  `setFilter6581Range`, `setFilter8580Curve`, `enableFilter`, `enableOld6581caps`, `setPaddle`.
- Enums (`src/residfp/residfp_defs.h`): `ChipModel { MOS6581, CSG8580 }` (note **8580 = CSG8580**),
  `CombinedWaveforms { AVERAGE, WEAK, STRONG }`, `SamplingMethod { DECIMATE, RESAMPLE, NONE }`.
- Per-cycle core (`SID.h:166`): `filter->clock(voice[0], voice[1], voice[2])` where
  `Filter::clock(Voice&,Voice&,Voice&)` (`Filter.h:216`) **sums the voices internally** via
  normalized-DAC lookup tables; state lives in `Vhp/Vbp/Vlp` plus `Integrator` objects;
  `Voice::output()` is public.

### M4.3 Build gate + wrapper
- Gate `WITH_RESIDFP_LIB` (mirror `WITH_RESID_VICE`); objects = `residfp-lib/*.o` +
  `residfp-lib/resample/*.o` + `engines/engine_residfp_lib.o`. Verify `-fpermissive` is NOT
  needed (clean C++11) - prefer building it without, to keep new-engine warnings visible.
- Wrapper `engine_residfp_lib.cpp` (mono, center-pan). Map `Model`: `MODEL_8580 -> CSG8580`.
  Map `Sampling`: `FAST`/`INTERPOLATE -> DECIMATE`, `RESAMPLE -> RESAMPLE`. Optionally expose
  the tuning setters via the interface's `setFilter6581Curve` etc.
- Factory branch under `#ifdef WITH_RESIDFP_LIB`; fallback + warn if requested but not compiled.

### M4.4 Verify
- Gated build links & plays; 6581 tunes vs golden (tolerance, not equality).

---

## M5 - libresidfp, TrueStereo (a rewrite, not a port)

libresidfp's filter is fundamentally different from reSID's: `Filter::clock(Voice&,Voice&,
Voice&)` pulls `voice.output()` itself and mixes via `getNormalizedVoice` /
`getNormalizedMixerVoice` lookup tables; state lives in `Vhp/Vbp/Vlp` (in `Filter`) with
`Integrator6581`/`Integrator8580` doing the per-sample solve (`Integrator*::solve()` is
`const`, so those heavy objects are **stateless per sample** - only `Vhp/Vbp/Vlp` and the
external filter carry state). So TrueStereo is a targeted rewrite, carried as
`patches/libresidfp-truestereo.patch`:

1. **Pan storage.** Add `pan[3]` to `SID` (or to each `WaveformGenerator`), extracted in
   `SID::write` for the PW-HI offsets `0x03 / 0x0a / 0x11`, high nibble.
2. **Dual filter chain.** Give `SID` a second `Filter` (L and R), a second `ExternalFilter`,
   and a **second `Resampler`**. The filters can share the immutable `FilterModelConfig*`
   singleton, so this is cheap in memory.
3. **Pan-aware filter clock.** Add `Filter::clock(Voice&,Voice&,Voice&, float g1,float g2,
   float g3)` that scales each voice's normalized contribution (**both** the summer-input path
   and the unfiltered mixer path) by the per-voice gain - physically a linear post-DAC gain,
   which is what pan is. The existing 3-arg `clock` becomes `clock(...,1,1,1)`.
4. **Stereo clock loop.** In `SID::clock`, clock the shared wave/env generators once per cycle,
   then run `filterL->clock(...panL...)` -> `extFiltL` -> `resL` and `filterR->clock(...panR...)`
   -> `extFiltR` -> `resR`. Emit into the `buf[0…]` / `buf[bufferHalfSize…]` split layout.
5. Optionally expose the new tuning setters (`setFilter6581Curve` etc.) through the wrapper.

Pan mapping: identical to the legacy formula (`src/resid/sid.cpp:683..692`): nibble 0..15;
if `==0xf` decrement; `pan %= 15`; `fpan = pan/14`; `gL = 1 - fpan`, `gR = fpan`. (The formula
makes an unset pan of 0 hard-left, but GTUltra always writes a pan nibble; replicate it exactly.)

Second resampler is acceptable, not a regression: the OLD FP engine already runs dual
`sampleL`/`sampleR` ring buffers (`src/resid-fp/sidfp.h:93-94`), so dual resampling is the
status-quo cost (CPU roughly doubles vs mono libresidfp - inherent to TrueStereo, already paid).

Risk: moderate-high (this touches the most numerically sensitive, recently-rewritten part of
libresidfp). Mitigations: keep the patch minimal and localized; gate it so the stock mono path
is unchanged when all pans are centered; carry it as a re-appliable patch file.

Verify: pan preserved; hard-sync + ring-mod tune correct; feature-preservation vs golden.

---

## M6 - Config / UI
- Append-only `.cfg` key `sidbackend` (positional `.cfg`; new keys MUST append, loader
  tolerates absence). Overrides the engine bits of `interpolate` when present.
- Append libresidfp tuning keys (`filter6581curve/range`, `filter8580curve`,
  `combinedwaveforms`, `old6581caps`) with defaults matching stock libresidfp.

## M7 - Deprecate old engines
Once M3 (+ M5) pass the full corpus, remove `src/resid/`, `src/resid-fp/`, their
makefile objects, the legacy `filterparams` application, and their wrappers. `gsid.h`
C API is unchanged, so nothing above `gsid.cpp` is touched.

---

## Testing

Aligns with `tests/docs/testing-strategy.md`: target defects and I/O contracts, not
coverage %. There are **no unit tests today** (only `tests/integration/smoke.sh` +
`check-goatdata.sh`). This migration is a rare high-ROI place to add the first unit suite,
because the wrapper is a genuinely testable seam.

### What is and is not in scope
- **Test our code:** the `ISidEngine` wrappers, pan extraction/mapping, the TrueStereo
  split-buffer contract, the factory/gating fallback. This is *our* logic layered on the
  engines.
- **Do NOT test emulation internals** of `resid*/` (vendored; upstream's job). We assert
  behavioural properties (no missing voices, pan preserved, sync/ring-mod audible), not
  that reSID models the 6581 correctly.

### Why unit testing is unusually tractable here
Unlike the `g*.c` god-modules (58+ external deps, SDL/global coupling), each engine is a
self-contained namespace that links standalone, and `ISidEngine` has **no SDL/global
dependency** (only `sidreg`/`filterparams`, which the test supplies). So a test can:
instantiate a wrapper, `write()` registers, `clockStereo()` into a buffer, and inspect the
samples - no editor, no `initscreen()`, no sprout needed. This is the seam the testing
strategy asks for (ladder #4), handed to us for free.

### Determinism caveat (verified, important)
`filter8580new.h:654-668` fills a 1024-entry noise buffer via `rand()` in the `Filter`
constructor with **no `srand`**, and GTUltra calls `rand()` elsewhere (`gsid.cpp:293`,
`gplay.c:567`). Therefore reSID-vice output is **not bit-reproducible** across runs/builds.
Consequences:
- New-engine golden tests must be **tolerance/statistical** (RMS, DC, cross-correlation,
  spectral centroid, click detector), never a byte hash.
- For deterministic unit assertions, seed `srand(fixed)` immediately before constructing
  the engine under test, or assert only structural properties (buffer layout, non-silence,
  L/R balance) that noise does not perturb.
- The OLD classic filter has no such noise (grep confirms), so **M1.5 bit-identical still
  holds** for the legacy-engine wrappers - use an exact hash there.

### Unit suite (`tests/unit/`, compiled as C++ TU)
Single-header harness (Unity or greatest, per strategy §Framework), `make test` target
wired into the gated CI job. Tests, smallest first:
1. **Pan mapping:** for nibble 0..15, assert `getPan()` and the exact legacy gains
   (`pan==0xf`->14; `pan%=15`; `gL=1-pan/14`, `gR=pan/14`).
2. **Split-buffer contract:** `clockStereo` returns `n`; L written to `[0..n-1]`, R to
   `[bufferHalfSize..]`; nothing written past the halves.
3. **Center == mono (tolerance):** all voices centered -> L and R within tolerance of the
   stock mono path summed to both channels (validates the dual-filter patch did not perturb
   the mono tone). Seed rand for stability.
4. **Shared front-end / sync-ring-mod:** a known register sequence exercising hard-sync and
   ring-mod renders non-trivially and matches stock-mono in mono mode (validates the split
   is at the filter input, downstream of `wave.synchronize()`).
5. **Factory fallback:** requesting a non-compiled backend returns the legacy engine and
   does not corrupt state (§3.2).
6. **M1 bit-identical:** legacy-engine wrapper output hashes equal to a captured
   pre-refactor reference.

Build the unit suite under `-fsanitize=address,undefined` (strategy ladder #1); the filter
math (int and later float) is a good UBSan target.

### Integration (golden WAV) - prerequisite: headless render
No headless render exists today (WAV export needs the SDL editor, `gsound.c:718-862`). Add a
batch path (CLI flag or CLI tool) decoupled from `initscreen()` **before M1** so golden WAVs
are automatable. Capture golden references with the current build first (M0). Corpus: 6581 +
8580 tunes, songs using ring-mod / hard-sync / filter sweeps, across mono / SID-stereo /
TrueStereo, both legacy backends. Store as `tests/fixtures/golden-*.wav` (or metrics +
hashes). Gate M1 on **bit-identical**; gate new engines on **feature preservation within
tolerance**, never sample equality.

### CI wiring
- Default jobs (Linux/macOS/Windows): unchanged - build + smoke + goatdata determinism.
- New gated job (Linux): `make linux-build WITH_RESID_VICE=1 SANITIZE=1` + `make test` +
  headless golden compare. Keeps the unstable engine compiling and sanitized without
  affecting default builds.

## Files to touch
- `src/engines/*` (new: interface + factory + 4 wrappers, added incrementally).
- `src/gsid.cpp`, `src/gsid.h` (drive `chip[4]`; `sid_debug()` -> `chip[0]->debug()`).
- `src/resid-vice/*` (M2; patched in M3; compile-gated `WITH_RESID_VICE`).
- `src/residfp-lib/*` (M4/M5; compile-gated `WITH_RESIDFP_LIB`).
- `src/makefile.common` (gated objects, `-include`, `clean`, `make test`; no `-I`, no
  `version.o`).
- `tests/unit/*` (new harness + `test_sid_engine.cpp`), `tests/integration/*` (headless
  render), `tests/fixtures/golden-*`.
- `.github/workflows/*` (one new gated Linux job; default jobs unchanged).
- `patches/resid-vice-truestereo.patch`, `patches/libresidfp-truestereo.patch`.

## Open decisions for the maintainer
1. Confirm M1 (wrapper) before any new engine. (Recommended.)
2. Runtime `sidbackend` vs build-time selection. (Proposal: runtime.)
3. Delete old engines after M7 vs keep as selectable backends permanently.
4. Golden corpus `.sng` picks (need 6581/8580 x sync/ring-mod/filter sweep). Shortlist in
   `golden-corpus-candidates.md`.
5. Upstream commit pinning + re-sync cadence.

---

## Appendix - source anchors (line numbers approximate; verify before editing)

Absorbed from the earlier analysis docs so this plan stands alone.

**This repo - engine boundary & config**
- `src/gsid.cpp`: `sid_init` (:55), `sid_fillbuffer` (:285), `sid_getorder` (:265),
  badline `rand()` (:293).
- `src/gsound.c`: `sound_init` (:84), `sound_init_offline` (new), `JPSoundMixer` (:567),
  `ExportSIDToPCMFile` (:785), `convertRAWToWAV` (:718), 4-chip mix `masterVolume*0.25` (:645/:845).
- Config/CLI: `interpolate` default `gt2stereo.c:111`, load `:311`, `-I` `:457`;
  `filterparams` `gsid.h:12` + `gsid.cpp:34` + apply `:192-262`; `MIXBUFFERSIZE` `gsound.h:29`;
  `NUMSIDREGS` `gsid.h:4`; positional `.cfg` (append-only).

**This repo - old patched TrueStereo (reference for the port)**
- `src/resid/sid.cpp:676..702` (pan split + per-channel filter), `:1082..1085` (dead buggy
  integer resample path); `src/resid/wave.cpp:84` (pan extraction from PW-HI hi-nibble),
  `wave.h` `getPan()`.
- `src/resid-fp/filterfp.h` (LR filter clock, `Vhp[2]/Vbp[2]/Vlp[2]`), `sidfp.h:93` (dual
  `sampleL`/`sampleR` ring buffers - proof dual resampling is already the status quo).

**Upstream (`_upstream/`, git-ignored)**
- reSID (`_upstream/resid`): `sid.h` public API + inline per-cycle clock
  (`filter.clock(v0,v1,v2)` -> `extfilt.clock(filter.output())`); `siddefs.h.in` enums
  (`chip_model`, `sampling_method`); `Makefile.am` `.dat->.h` Perl rule; `wave.cc:49..70`
  includes generated tables into `model_wave[2][8][1<<12]`; `filter8580new.*` (selected by
  `NEW_8580_FILTER`), noise buffer `filter8580new.h:654-668`.
- libresidfp (`_upstream/libresidfp`): `src/SID.h` public API + inline clock (`SID.h:166`);
  `Filter.h:216` `clock(Voice&,Voice&,Voice&)`; `Voice.h` `output()`; `ExternalFilter.h`;
  `Integrator6581.h` `solve() const`; `src/residfp/residfp_defs.h` enums; `siddefs-fp.h.in`
  config macros; `src/resample/*` resampler.
- Re-clone: `git clone --depth 1 https://github.com/libsidplayfp/{libresidfp,resid}.git` into `_upstream/`.

Licensing: GTUltra, both old engines, and both new engines are all GPL-2.0-or-later. No conflict.
</content>
</invoke>
