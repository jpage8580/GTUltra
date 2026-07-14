# Headless render mode - scope + tracking (M0.1)

Active working doc for the first M0 task in `sid-engine-migration-tracking.md`:
a no-window, no-editor path that renders a `.sng` to WAV so golden fixtures and
engine A/B comparisons are automatable. Update this file as the work proceeds.

Status: SCOPING (no code yet). Owner: TBD. Branch: TBD (per user: not a separate PR for now).

---

## 0. Prerequisite (DONE, uncommitted): SDL_MAIN_HANDLED for gt2reloc

Resolves the deferred "CLI tools: headless SDL init on mac/Windows" follow-up named in
`tests/docs/known-bugs.md:19-22` and `handover-gt2reloc-bug2.md:23-26`. Root cause + fix in §3a.

Scope narrowed by evidence: among CLI tools, **only `gt2reloc` links SDL** (it includes
`bme.h`). `ins2snd2`/`mod2sng2`/`ss2stereo` link only `bme/bme_end.o` (verified in the mac
link lines) and never include `SDL_main.h`, so their `main` is not renamed - they were never
affected. `gtultra` is interactive and keeps the SDL/Cocoa path on purpose.

- [x] Add `#ifndef __WIN32__ #define SDL_MAIN_HANDLED #endif` before `bme.h` in `gt2reloc.c`.
- [x] No `SDL_SetMainReady` needed - `gt2reloc`/`greloc` never call `SDL_Init`.
- [x] `makefile.mac`/`makefile` (linux) use `sdl2-config --libs` = `-lSDL2` (no `SDL2main`);
      Windows links `-lSDL2main` -> that is why the define is `__WIN32__`-guarded.
- [x] Mac regression: rebuilds clean; packs fixture exit 0 (3431 bytes); `gt2reloc.o` now
      exports real `_main` (not `_SDL_main`).
- [~] Headless (no-GUI) validation: CANNOT reproduce in the dev shell - it runs inside the
      logged-in Aqua session, so gt2reloc already worked there (matches maintainer's Terminal
      experience). Real validation is mac CI (no GUI session). PENDING on CI.
- [ ] After CI proves it: drop `timeout-minutes:1` headless guard on mac CLI smoke; update
      `CHANGELOG.md` `### Fixed`; close the known-bugs follow-up.
- [ ] Windows headless (separate, not done): would need `SDL_MAIN_HANDLED` there too AND
      dropping `-lSDL2main` for the console tool; no Windows env to verify - defer.

---

## 1. Key finding: the render pipeline is already SDL-audio-free

Verified in source. The actual sample-producing path touches no audio device and no editor:

```
playroutine(gt)                       # gplay.c - writes SID registers into gt->sidreg[]
  -> ExportSIDToPCMFile(samples,norm) # gsound.c:785 - calls sid_fillbuffer, mixes 4 chips,
                                       #   writes interleaved S16 stereo to exportFileHandle
convertRAWToWAV(norm)                 # gsound.c:718 - wraps the raw PCM in a WAV header
```

`ExportSIDToPCMFile` only guards on `initted` and `samples <= MIXBUFFERSIZE`
(`gsound.c:787-788`) and pulls globals `mr`, `masterVolume`, `editorInfo.adparam`. It does
**not** call any SDL audio API. So the DSP path is headless already; the coupling is entirely
in how it is *reached* today (interactive editor) and in *engine init* (`sound_init` opens an
SDL audio device via `snd_init`, `gsound.c:227`).

## 2. Precedent: `gt2reloc.c` already drives the player headlessly

`gt2reloc` is a CLI tool that links the player + song model and runs it with **no
`initscreen`, no `sound_init`, no `SDL_Init`**:
- `io_openlinkeddatafile` -> `initchannels` -> `clearsong` -> `loadsong` (`gt2reloc.c:225-259`)
- drives `initsong` + `playroutine(gte)` in a loop (`gt2reloc.c:671-678`)
- defines its own copies of the app globals the linked objects need (it does not link
  `gt2stereo.o`), and sets `noSIDWrites = 1` to skip register writes it does not need.

The render tool is "`gt2reloc` but: wire real SID writes, init the engines offline, and in the
loop call `ExportSIDToPCMFile` then `convertRAWToWAV`."

## 3. Coupling to cut / provide (verified anchors)

- [ ] **Engine init without an audio device.** `sound_init` (`gsound.c:84`) allocates the
  `sid*buffer`s and calls `sid_init`, but also `snd_init` (SDL audio open, `:227`) and
  registers the realtime mixer (`snd_setcustommixer`, `:236`). For offline we want neither.
  -> Add `sound_init_offline(...)` in `gsound.c`: buffers + `playspeed=mr` + `sid_init` +
  `initted=1`, skipping `snd_init`/`snd_setcustommixer`/timer/thread. Small, clean, keeps the
  realtime path untouched.
- [ ] **SID register wiring.** Writes only reach the engine if `gt->sidreg[i]` points at the
  `gsid.cpp` `sidreg*[]` arrays. The editor does this via `initSID(&gtObject)` with
  `noSIDWrites=0` (`gt2stereo.c:722-727`); `gplay.c` gates every write on `!noSIDWrites`
  (`gplay.c:191,433,555,...`). -> tool must call `initSID` and keep `noSIDWrites=0` (unlike
  gt2reloc).
- [ ] **App globals.** Linked `gsound.o`/`gsid.o`/`gplay.o` reference `mr`
  (`gt2stereo.c:107`), `masterVolume` (`:130`), `editorInfo`, etc., which live in
  `gt2stereo.o` (not linked). -> tool defines its own, as `gt2reloc.c` does. Enumerate the
  full extern set during implementation.
- [ ] **Editor-only calls in the current export loop.** `ExportAsPCM` (`gt2stereo.c:3329`)
  interleaves `getkey()`, `displayupdate(gt)`, `SDL_Delay` (`:3371-3373`). -> do NOT reuse
  `ExportAsPCM`; write a tiny render loop in the tool that omits those. Reuse only
  `ExportSIDToPCMFile` + `convertRAWToWAV` (both editor-free).
- [ ] **Output filenames.** `convertRAWToWAV` uses globals `rawFileName`/`wavfilename` set by
  `GenerateExportFileName()`. -> set output path explicitly (add a setter or a param).
- [ ] **SDL init at all?** `gt2reloc` links `bme` but never `SDL_Init`s audio/video. Confirm
  the offline path needs no `SDL_Init` (likely only `SDL_LogSetOutputFunction`, as
  `gt2reloc.c:220`). If `snd_init` is fully avoided, no audio subsystem is needed.

## 3a. KNOWN BLOCKER: mac headless-SDL startup hang (do not re-derive)

There is a documented, still-open issue that directly constrains this tool:
**any SDL-linked CLI tool run from a non-GUI macOS shell hangs on an SDL startup dialog**
(a mac `NSAlert` was seen locally with `gt2reloc`). `SDL_VIDEODRIVER=dummy` does NOT fix it.
Sources: `CLAUDE.md:24-26`, `tests/docs/handover-gt2reloc-bug2.md:23-26`,
`tests/docs/known-bugs.md:19-22` (CI caps smoke at `timeout-minutes:1` so it fails fast),
`tests/README.md:39`. A deferred follow-up is named: "CLI tools: headless SDL init on
mac/Windows."

Mechanics (CONFIRMED, root cause + fix known): `SDL_main.h:117-118` does
`#define main SDL_main`, guarded by `#ifndef SDL_MAIN_HANDLED` (line 33); macOS sets
`SDL_MAIN_AVAILABLE` (line 40). Nothing in the repo defines `SDL_MAIN_HANDLED`, so every
tool's `main` is renamed to `SDL_main` and SDL runs a **Cocoa app bootstrap** before our
code. That bootstrap needs an **Aqua (GUI login) session**: it succeeds in Terminal.app while
logged into the desktop (gt2reloc works there - confirmed by maintainer), and blocks on an
`NSAlert` from an ssh/agent/CI shell with no GUI session. This also explains why
`SDL_VIDEODRIVER=dummy` never helped: the dialog fires pre-`main`, before any `SDL_Init` reads
the hint. (`SDL_Init(VIDEO|AUDIO|JOYSTICK)` at `bme/bme_win.c:80` via `win_open` is a
separate, later concern.)

**Fix (one line in the tool's source):**
```c
#define SDL_MAIN_HANDLED   /* BEFORE including bme.h / SDL */
#include "bme.h"
```
`main` is no longer renamed -> no Cocoa bootstrap -> runs headless as a plain console binary.
Call `SDL_SetMainReady()` before any `SDL_Init`. Layer with `sound_init_offline` (never reach
`win_open`) so no `SDL_INIT_VIDEO`; init at most `SDL_INIT_AUDIO` with `SDL_AUDIODRIVER=dummy`.
Build caveat: with `SDL_MAIN_HANDLED` do NOT link `SDL2main` (sdl2-config --libs is usually
just `-lSDL2`; confirm), else a duplicate/renamed `main`.

Consequences / decisions:
- With the fix, `sng2wav` can run headless on mac too; without it, render automation is
  Linux-only. Either way, **primary golden capture + A/B compare run on LINUX** (dummy
  drivers), matching the ASan-Linux-only gate philosophy.
- Applying `SDL_MAIN_HANDLED` to ALL CLI tools (`gt2reloc`/`ins2snd2`/`mod2sng2`/`ss2stereo`)
  is the deferred "CLI tools: headless SDL init on mac/Windows" follow-up; it would let
  mac/Windows CI drop the `timeout-minutes:1` headless-hang guard.

Two `gt2reloc`-derived gotchas that also transfer (both already understood):
- Declare `loadedsongfilename[MAX_PATHNAME]` (NOT `MAX_FILENAME`) - the size-mismatch
  overflow fixed in `gt2reloc.c:84` (`handover-gt2reloc-bug2.md` Defect A).
- We want `noSIDWrites = 0` + a real `initSID` wiring. That is exactly the path whose absence
  NULL-derefed `gt2reloc` (`gplay.c:435`, issue #71 Defect B); getting the wiring right is
  mandatory, not optional.

## 4. Approach decision

- **Chosen: new CLI tool** (working name `sng2wav`), modeled on `gt2reloc.c`, linking the same
  object set + the two export functions. Rationale: matches the existing CLI-tool pattern,
  keeps SDL video/editor entirely out, easiest to sanitize and to call from tests.
- **Rejected: a `--export` flag on `gtultra`.** Still drags `initscreen()` and the interactive
  loop; harder to keep SDL video out; muddier to test.

## 5. Proposed CLI surface (draft, refine on implementation)

```
sng2wav <in.sng> <out.wav>
        [--subsong N] [--seconds S | --frames F]
        [--model 6581|8580] [--stereo 0|1|2]
        [--backend legacy-resid|legacy-residfp|vice-resid]
        [--rate 44100] [--normalize] [--seed K]
```

- Default render length: **fixed** (`--seconds`/`--frames`) for deterministic fixtures;
  optional "play until all channels loop" mode mirroring `ExportAsPCM`'s end-detect.
- `--backend` maps to the migration's backend enum (legacy today; new ones as they land).
- `--seed` sets `srand(K)` before init (see Determinism).

## 6. Determinism notes (for golden fixtures)

- `rand()` is used in `gplay.c:567` (pan spread), `gsid.cpp:293` (badline), and later in the
  new `filter8580new` noise buffer. Set `srand(--seed)` once at startup so legacy-engine
  fixtures are reproducible run-to-run.
- Legacy engines are otherwise deterministic; new reSID-vice will still not be bit-exact once
  its `rand()`-seeded filter noise is in play -> new-engine goldens stay tolerance-based (see
  main plan Testing).

## 7. Task checklist  (DONE except CI wiring - committed `1647356`)

- [x] Extern-global set handled by factoring `src/cli_common.c` (shared boilerplate from
      gt2reloc; gt2reloc verified byte-identical after the extraction).
- [x] Added `sound_init_offline` to `gsound.c` + prototype in `gsound.h`.
- [x] Output path handled via the `wavfilename` global + `GenerateExportFileName` (no separate
      setter needed).
- [x] Wrote `src/sng2wav.c` (arg parse, load, `initSID`, offline init, render loop, WAV out,
      plus `--scan` survey with recursive dir walk + box-table renderer).
- [x] Added `sng2wav` to `makefile.common` (EXE list + link rule, gt2reloc object set minus greloc.o).
- [x] Smoke: renders `Stereo_Pendejo.sng` -> valid non-silent stereo WAV (L!=R confirms TrueStereo).
      NOTE: the `--seed` byte-identical before/after check was implemented but not formally run to
      completion - do this before trusting goldens.
- [ ] Wire into `tests/integration` on **Linux** (dummy drivers); mac headless is out (§3a). (future CI)
- [x] Updated `sid-engine-migration-tracking.md` M0.

## 8. Open questions

- [ ] End-detect (`chn[i].loopCount`, `editorInfo.maxSIDChannels`) vs fixed-length only for v1?
      (Lean: fixed-length for tests; add loop-detect later.)
- [ ] Reuse `gt2reloc`'s global-definition block verbatim, or factor a shared
      `cli_globals.c`? (Factoring reduces drift but is extra scope.)
- [x] Does any linked object require a real `SDL_Init`? -> `bme/bme_win.c:80` inits
      VIDEO|AUDIO|JOYSTICK via `win_open` (unreached here), but mac still hangs via the SDL
      Cocoa main shim (§3a). Net: render is Linux-only for automation.
- [ ] Can `sound_init_offline` fully avoid `SDL_Init(AUDIO)` on Linux too, so `sng2wav` needs
      zero SDL subsystems there? (Would simplify CI; confirm by building.)
</content>
