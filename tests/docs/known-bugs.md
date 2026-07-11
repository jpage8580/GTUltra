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

---

## Bug 2 — `gt2reloc` segfaults when packing a `.sng`

- **Where:** `gt2reloc` CLI tool (command-line packer/relocator).
- **Symptom:** on the fixture `tests/fixtures/Stereo_Pendejo.sng` it segfaults during
  packing (`EXIT: 139` on macOS). Documented in `tests/README.md`.
- **Status:** the functional smoke test is **kept blocking** so the regression stays
  visible in CI. Reproduced under ASan on Linux (Lima VM): still segfaults (exit 139).
- **ASan finding (partial, does NOT fix the segfault):** `clearsong` (`src/gsong.c:1703`)
  does `memset(loadedsongfilename, 0, sizeof loadedsongfilename)`. The header extern
  (`goattrk2.h:117`) and `gt2stereo.c:136` declare `loadedsongfilename[MAX_PATHNAME]`
  (256), but **`gt2reloc.c:84` declares it `[MAX_FILENAME]` (60)** — so in the gt2reloc
  build `clearsong` memsets 256 bytes into a 60-byte global → ASan `global-buffer-overflow`
  (`WRITE of size 256 ... 0 bytes after 'loadedsongfilename'`, via `gt2reloc.c:221`).
  **Verified:** changing `gt2reloc.c:84` to `MAX_PATHNAME` silences this overflow but the
  gt2reloc pack **still segfaults (139)** — so this is a real, separate defect but NOT the
  segfault root cause. Deferred to the Bug-2 / unused-result follow-up.
- **Next step:** get a full stack trace at the actual fault (build `-g -O0` +ASan), locate
  it, pin with a characterization test on the fixture (assert exit 0 + non-empty `.prg`),
  then fix. Also apply the `gt2reloc.c:84` size fix and re-widen the ASan job to the CLI
  usage checks (currently `SKIP_USAGE=1` there to avoid this pre-existing overflow).
  Full pick-up plan: [handover-gt2reloc-bug2.md](handover-gt2reloc-bug2.md).

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

## Note (not scheduled) — `ss2stereo` legacy limitation

- `ss2stereo` (original GoatTracker v2.76 stereo splitter) has a hardcoded
  `MAX_SONGLEN` / orderlist length ≤ 254 that modern GTUltra songs exceed:
  `ERROR: Orderlist-length of 254 exceeded!`
- Excluded from the functional smoke test. This is a legacy-tool limitation, not a
  GTUltra bug. Fix the limit before re-adding it to functional smoke tests.

---

## Highest-leverage cross-cutting move

Add **ASan/UBSan** to a CI build running the existing `tests/integration/smoke.sh`.
It would have caught Bug 1 automatically with a stack trace, and will help localize
Bug 2. See also the sanitizer recommendation referenced from the testing strategy.
