# GTUltra editor vs sng2wav: audio-fidelity analysis

Precise, code-anchored answer to: "does `sng2wav` reproduce the GTUltra editor's audio
output?" Short version: **it runs the identical synthesis + mixing code, so it is
sample-for-sample identical to the editor's WAV *export* under matched conditions, but it is
NOT a bit-perfect capture of an arbitrary live editor session.** The exact shared code, and
every possible divergence, are enumerated below with source anchors (line numbers as of the
`new-emulators` branch, 2026-07-14).

---

## 1. The synthesis + mixing code is shared (not reimplemented)

All three paths converge on the same functions:

| Path | Entry | Per-block call |
|---|---|---|
| Editor real-time playback | `sound_mixer` -> `JPSoundMixer` (`gsound.c:557,567`) | `sid_fillbuffer(...)` (`gsound.c:618`) |
| Editor WAV export | `ExportAsPCM` (`gt2stereo.c:3329`) -> `ExportSIDToPCMFile` (`gsound.c:785`) | `sid_fillbuffer(...)` (`gsound.c:828`) |
| **sng2wav render** | `main` render loop (`sng2wav.c`) -> `ExportSIDToPCMFile` | `sid_fillbuffer(...)` (`gsound.c:828`) |

- `sid_fillbuffer` (`gsid.cpp:285`) is the single DSP entry: it clocks the reSID / reSID-FP
  engine objects, interleaving register writes with clocking in `SIDWAVEDELAY` / `residdelay`
  / `SIDWRITEDELAY` cycle chunks. Same code for all callers.
- The 4-chip stereo mix is identical in both non-real-time consumers: sum the four left
  half-buffers and four right half-buffers, scale by `masterVolume * 0.25f`
  (`ExportSIDToPCMFile` `gsound.c:845`; `JPSoundMixer` `gsound.c:645`).
- `convertRAWToWAV` (`gsound.c:718`) writes the WAV header + PCM identically for editor export
  and sng2wav.

**Consequence:** for identical inputs and identical RNG/volume/rate state, the produced samples
are bit-identical by construction - it is literally the same object code, not a port.

The old engines are also **internally deterministic**: `src/resid` (integer) and `src/resid-fp`
(float) contain no `rand()`/noise source (verified). (Note: the *new* VICE `filter8580new`
does inject `rand()` noise - relevant only once M2+ lands, not for today's legacy engines.)

---

## 2. sng2wav's loop vs the editor's export loop

Editor `ExportAsPCM` (`gt2stereo.c:3329-3400`):
1. `stopsong` + `playUntilEnd(songNumber)` (pre-roll), `bypassPlayRoutine = 1`, `SDL_Delay(50)`.
2. `GenerateExportFileName` + `OpenExportFileNameForWriting`.
3. `initsong(sng, PLAY_BEGINNING, gt)`, `loopEnabledFlag = 0`, `followplay = 1`.
4. loop: (every 100 iters `SDL_Delay(10)`/`getkey()`/`displayupdate()`), `playroutine(gt)`,
   `ExportSIDToPCMFile(samplesToExport, doNormalize)`, until every channel has looped.
5. `ExportCloseFileHandle`, `convertRAWToWAV`.

sng2wav (`sng2wav.c` render path):
1. `sound_init_offline(...)` (sets `bypassPlayRoutine = 1`, `initted = 1`, `sid_init`), `initSID`.
2. `initsong(sng, PLAY_BEGINNING, gt)`, `loopEnabledFlag = 0`, `followplay = 1`.
3. `GenerateExportFileName` + `OpenExportFileNameForWriting`.
4. loop: `playroutine(gt)`, `ExportSIDToPCMFile(samplesPerFrame, normalize)`, for a fixed frame
   count (from `--seconds`/`--frames`), or until `songinit == PLAY_STOPPED`.
5. `ExportCloseFileHandle`, `convertRAWToWAV`.

`samplesPerFrame` uses the same formula as the editor: `(mr*2)/100`, `*2` if `multiplier==0`,
else `/multiplier` (`ExportAsPCM` `gt2stereo.c:3358-3362`; `sng2wav.c`). The
`SDL_Delay`/`getkey`/`displayupdate` calls in the editor loop happen *between* fills and do not
touch the sample buffers, so they do not change audio content.

---

## 3. Every possible divergence (exhaustive)

### 3.1 Master volume  (LEVEL difference)
`ExportSIDToPCMFile` scales by the global `masterVolume` (`gsound.c:845`, `mvf = masterVolume*0.25f`).
- Editor: user's current `masterVolume` (`gt2stereo.c:130`, runtime-adjustable).
- sng2wav: fixed `1.0f` (`cli_common.c`), **no `--volume` flag**.
-> Identical only when the editor's master volume is 1.0. Otherwise a pure gain difference.

### 3.2 RNG state  (rare CONTENT difference)
`rand()` is used in exactly two places on the play/mix path:
- `gplay.c:567,569` - the pan-spread play command (`panMin/panMax += rand()%range`). Only fires
  for songs that use that command.
- `gsid.cpp:293` - `badline = rand() % NUMSIDREGS`, a random register-write delay slot, but it
  only has effect inside `if ((badline==c) && (residdelay))` (`gsid.cpp:331`). With
  `residdelay == 0` the `rand()` result is unused.
State:
- Editor: never calls `srand` (verified - no `srand` in `gt2stereo.c`), so it starts from libc
  default seed 1, but by export time an unknown number of `rand()` calls have occurred during
  interactive playback / `playUntilEnd` pre-roll -> RNG state at export is not predictable.
- sng2wav: `srand(seed)` at startup (`--seed`, default 0) -> fully deterministic.
- sng2wav default `residdelay` is 0 (`cli_common.c`), so the badline `rand()` is inert;
  pan-spread is the only remaining RNG-driven content, and only for songs using it.
-> Identical when: song uses no pan-spread command AND `residdelay` matches (both 0), OR the
  RNG sequences happen to align. sng2wav is deterministic run-to-run; the editor generally is
  not (for affected songs / nonzero residdelay).

### 3.3 Sample rate  (PITCH/RESAMPLE difference)
`sid_init` is given `playspeed` as the output sample rate.
- Editor `sound_init`: `playspeed = snd_mixrate` (`gsound.c:231`) - the rate the SDL audio
  device actually *obtained*, which may differ from the requested `mr`.
- sng2wav `sound_init_offline`: `playspeed = rate` clamped to `[MINMIXRATE, MAXMIXRATE]`
  (requested value used directly; no device involved).
-> Identical when the device's obtained rate equals the requested rate (typically true at
  44100, not guaranteed).

### 3.4 Engine + tuning parameters
- Engine/sampling: both from the packed `interpolate` value; sng2wav builds it as
  `(backend<<1)|interp` (default 3 = reSID-FP + interpolate = the app default `gt2stereo.c:111`).
- reSID-FP `FILTERPARAMS` (`gsid.cpp:34,192-262`): applied identically - both link the same
  `gsid.cpp` which applies the global `filterparams` (default table in `gsid.cpp:34`) to the FP
  engine at `sid_init`. sng2wav does not alter them, so they match the editor's defaults. (If a
  user changed `filterparams` via `.cfg`, sng2wav would need that same `.cfg`; it currently uses
  the built-in defaults.)
- Chip model / stereoMode / NTSC / multiplier / adparam(HR): sng2wav loads them from the
  `.sng` 0x1f block (or CLI overrides); the editor uses the same loaded values (or user tweaks).
  Match for a saved song with no unsaved edits.

### 3.5 Song register-write order
`sid_getorder` (`gsid.cpp:265`) picks `altsidorder` when `adparam >= 0xf000`, else `sidorder`.
Driven by `editorInfo.adparam` in both -> identical when adparam matches (it does, from the .sng).

### 3.6 Length / loop handling  (LENGTH difference, not per-sample)
- Editor export: plays until every channel's `loopCount != 0` (one full pass).
- sng2wav: fixed frame count (`--seconds`/`--frames`), or until `PLAY_STOPPED`.
-> Different total length; the overlapping frames are identical (given 3.1-3.5 match).

### 3.7 Init/pre-roll side effects
- Editor `ExportAsPCM` does `stopsong` + `playUntilEnd` before `initsong`; sng2wav does not.
  `initsong(PLAY_BEGINNING)` resets channel/song play state, so post-init playback is equivalent
  - but the pre-roll advances the editor's `rand()` state (see 3.2) and is the main reason the
  editor's export RNG state is unpredictable.

### 3.8 Things that do NOT differ
- The SDL audio device / real-time mixer callback: during editor export `bypassPlayRoutine=1`
  makes `sound_mixer` skip `JPSoundMixer` (`gsound.c:560`), so the device never touches the
  export buffers. sng2wav opens no device at all. No effect on exported samples either way.
- `SIDWAVEDELAY`/`SIDWRITEDELAY` write timing, 4-chip split-buffer layout, normalize logic,
  WAV header - all shared code.

---

## 4. Exact parity condition

`sng2wav <song> out.wav` is **sample-for-sample identical** to the editor's WAV export when ALL hold:
1. same engine + sampling (`interpolate` / `--backend`+`--interp`),
2. same chip model, stereoMode, NTSC/PAL, speed multiplier, adparam(HR), channel count, subsong,
3. `masterVolume == 1.0` in the editor (sng2wav is fixed at 1.0),
4. device obtained rate == requested rate (both effectively `mr`),
5. `residdelay` equal (sng2wav = 0) AND the song uses no pan-spread `rand()` command (or the RNG
   sequences otherwise coincide),
6. same `filterparams` (sng2wav uses the built-in defaults),
7. comparison restricted to the overlapping frame range.

Under those, equality is guaranteed because the sample-producing code is byte-for-byte the same
object code.

## 5. What this means for the migration

- For **golden references and A/B engine comparison** (the M0/M1 purpose), sng2wav is the correct
  instrument: same DSP, and with `--seed` + fixed flags it is fully reproducible. The legacy
  engines have no internal RNG, so M1's "bit-identical before/after the `ISidEngine` refactor"
  check is meaningful and achievable (compare sng2wav output with identical args/seed).
- It is **not** a bit-perfect recording of a specific live editor session; do not claim that.

## 6. Follow-ups implied by this analysis

- Add a `--volume` flag to sng2wav (remove divergence 3.1).
- Optionally add `--residdelay` and a way to load a `.cfg` `filterparams` block, for exact
  parity with a specific user configuration (3.2 / 3.4).
- To *prove* parity empirically: in the editor set master volume = 1.0 and `residdelay = 0`,
  export a song with no pan-spread command; render the same song+settings+rate with sng2wav for
  the same frame count; the PCM should match. (Not yet run - see tracker loose ends.)
