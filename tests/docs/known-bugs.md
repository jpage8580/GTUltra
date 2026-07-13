# Task: Known functional bugs & their fixes

This file tracks **open** functional bugs. Fixed ones move out (record lives in
`CHANGELOG.md` + any `handover-*.md`). Related: [build-determinism.md](build-determinism.md).

Approach for each: **pin behavior with a characterization/regression test FIRST**, then
fix, so the fix is provably correct and stays fixed.

## Resolved (record only, no longer tracked here)
- **issue #76** — heap buffer overflow in `setPaletteName()` (`src/gpaletteeditor.c`).
  Fixed (`strdup`) + Linux ASan/UBSan CI gate. Details/credit: `CHANGELOG.md` and
  [handover-issue-76.md](handover-issue-76.md). The same PR fixed 3 `-Wformat-security`
  sites; the `-Wunused-result` sweep was deferred → [handover-unused-result.md](handover-unused-result.md).
- **Bug 2 / issue #71** — `gt2reloc` segfault when packing a song (NULL `sidreg[i]` in
  `playroutine`, `gplay.c:435`) plus a `loadedsongfilename` `global-buffer-overflow`. Fixed
  by marking `gtEditorObject.noSIDWrites = 1` in `gt2reloc` and widening the declaration to
  `MAX_PATHNAME`. Functional smoke (`.prg` + `.sid`) and Linux ASan usage checks re-enabled.
  Details: `CHANGELOG.md`, [handover-gt2reloc-bug2.md](handover-gt2reloc-bug2.md).
  Note: fully verified on Linux (VM, ASan-clean, both formats). `SKIP_GT2RELOC` was removed
  from all three build workflows to check mac/Windows on CI; smoke steps are capped at
  `timeout-minutes: 1` so a possible headless-SDL hang on those runners fails fast rather
  than stalling. If mac/Windows misbehave, re-skip there and open the headless-SDL follow-up.

---

## Bug 3 — high idle CPU on macOS Apple Silicon (GitHub issue #85)

- **Where:** `gtultra` interactive editor (main loop), macOS on Apple Silicon (M-series).
- **Symptom:** "can easily take 40%+ on an idle editor" — high CPU with no user activity.
- **Status:** OPEN, root cause not yet confirmed.
- **Likely cause (to verify):** a busy render/input loop with no blocking wait or frame
  throttle. The editor loops `fliptoscreen(); getkey();` (e.g. `gt2stereo.c` main loop,
  `gpaletteeditor.c:275`) — if input is polled non-blocking and the frame is redrawn every
  iteration without vsync or an `SDL_Delay`/`SDL_WaitEvent`, the thread spins at 100% of a
  core. Retina/HiDPI redraw cost on Apple Silicon may amplify it.
- **Next step:** profile idle with `sample`/Instruments (Time Profiler) to find the hot
  loop; check the BME/SDL event path (`src/bme/bme_*.c`) for polling vs blocking and
  whether vsync / a frame cap is applied. Fix by yielding when idle (blocking event wait
  or a small delay) without harming playback timing. Pin with a manual CPU-usage check;
  this is not currently covered by smoke tests (interactive).

---

## Bug 4 — potential heap overflow in the special-note-name loop (`gt2stereo.c:2972`)

- **Where:** special-note-name build loop (`gt2stereo.c` ~2958-2980).
- **Symptom (latent):** `name = malloc(4)` holds 2 note chars + octave digits + NUL. Fits a
  single-digit octave (2 + 1 + 1 = 4), but a **two-digit `oct` (>= 10) overflows** via
  `strcpy(name + 2, octave)` (2 + 2 + 1 = 5 > 4). Same class as issue #76.
- **Status:** OPEN. Surfaced by `-Wstringop-truncation` at `:2972`, **kept loud on purpose**
  (not silenced) so the signal stays visible — see [warnings-tracking.md](warnings-tracking.md).
- **Next step (own PR, ASan-verified):** determine whether `oct` can reach 10 (outer
  `while (i < 93)` loop, `specialnotenames` size, custom tunings / `equaldivisionsperoctave`);
  if reachable, size the allocation with `snprintf`/proper bound and fix the copy; if provably
  unreachable, document the bound then silence. Verify under the Linux `SANITIZE=1` ASan job.
  Full pick-up: [handover-owncode-warnings.md](handover-owncode-warnings.md).

## Note (not scheduled) — `ss2stereo` legacy limitation

- `ss2stereo` (original GoatTracker v2.76 stereo splitter) has a hardcoded
  `MAX_SONGLEN` / orderlist length ≤ 254 that modern GTUltra songs exceed:
  `ERROR: Orderlist-length of 254 exceeded!`
- Excluded from the functional smoke test. This is a legacy-tool limitation, not a
  GTUltra bug. Fix the limit before re-adding it to functional smoke tests.

---

## Highest-leverage cross-cutting move — DONE

The Linux **ASan/UBSan** CI job (`asan:` in `.github/workflows/build-linux.yml`, added with
the issue #76 fix) runs `tests/integration/smoke.sh` under `-fsanitize=address,undefined`.
It gated #76 and is what localized Bug 2 (the symbolized `playroutine` NULL-write trace).
