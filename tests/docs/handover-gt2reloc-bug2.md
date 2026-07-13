# RESOLVED: `gt2reloc` segfault (Bug 2 / issue #71) + `loadedsongfilename` overflow

**Status: RESOLVED** (branch `fix/gt2reloc-segfault-bug2`; closes issue #71).

**Root cause (Defect B, the segfault):** relocation-time playback runs on `gtEditorObject`
(`gt2reloc.c:653` `gte = &gtEditorObject` → `playroutine`). Its `sidreg[]` pointers are
never wired via `initSID` (only `gtObject` is). The main app tolerates this by setting
`gtEditorObject.noSIDWrites = 1` (`gt2stereo.c:723`), and `gplay.c:433` guards the SID write
with `if (!gt->noSIDWrites)`. `gt2reloc` omitted the flag → wrote through NULL `sidreg[i]`
at `gplay.c:435` (SEGV, addr 0x15). **Fix:** set `gtEditorObject.noSIDWrites = 1` in
`gt2reloc`'s setup.

**Defect A (overflow):** `loadedsongfilename` was `[MAX_FILENAME]` (60) in `gt2reloc.c:84`
vs the `[MAX_PATHNAME]` (256) extern; `clearsong`'s memset overflowed it. **Fix:** widened
to `MAX_PATHNAME`.

**Verified (Lima Linux VM, ASan/UBSan, `SANITIZE=1`):** fixture packs to both `.prg` and
`.sid` at exit 0, no sanitizer diagnostics; usage path (no args) clean (Defect A gone);
issue #71's exact command (`.sid` + `-EEnable -HEnable -S1 -W10`) also exit 0. Pack ~0.11s.

**CI:** re-enabled the `gt2reloc` functional smoke (both formats) by removing
`SKIP_GT2RELOC` from all three build workflows, and removed `SKIP_USAGE` from the Linux ASan
job. Smoke steps capped at `timeout-minutes: 1`. **Open item:** mac/Windows CI is unverified
locally (only the Linux VM was available); if the CLI tools hit a headless-SDL startup hang
there (a mac `NSAlert` dialog was seen locally), re-skip on those OSes and open a separate
"CLI tools: headless SDL init on mac/Windows" issue.

Original pick-up notes below, retained for context.

---

Pick-up doc. Bug 2 background in [known-bugs.md](known-bugs.md). Partly investigated while
adding the ASan gate for issue #76 ([handover-issue-76.md](handover-issue-76.md)). Fresh
branch from `main`.

## Goal
Make `gt2reloc` pack the fixture without crashing, and fix the ASan-found global overflow.

## Two related defects (both in the `gt2reloc` tool)

### Defect A — `loadedsongfilename` size mismatch (ASan global-buffer-overflow)
- `goattrk2.h:117`: `extern char loadedsongfilename[MAX_PATHNAME]` (256).
- `src/gt2stereo.c:136`: defines it `[MAX_PATHNAME]` (256) — correct (comment: "JP was
  MAX_FILENAME").
- **`src/gt2reloc.c:84`: defines it `[MAX_FILENAME]` (60)** — inconsistent with the extern.
- `clearsong` (`src/gsong.c:1703`) does `memset(loadedsongfilename, 0, sizeof
  loadedsongfilename)`; `sizeof` via the header is 256, so in the `gt2reloc` build it
  memsets 256 bytes into a 60-byte global.
- ASan (Linux, `SANITIZE=1`): `global-buffer-overflow WRITE of size 256 ... 0 bytes after
  global 'loadedsongfilename' ... gt2reloc.c:84 ... in clearsong gsong.c:1703`, reached
  from `gt2reloc.c:221` (main).
- **Fix:** change `src/gt2reloc.c:84` to `char loadedsongfilename[MAX_PATHNAME];` (match the
  extern; `MAX_FILENAME`=60, `MAX_PATHNAME`=256, both in `src/gfile.h`).

### Defect B — the segfault itself (Bug 2, still unexplained)
- `gt2reloc tests/fixtures/Stereo_Pendejo.sng out.prg` **segfaults (exit 139)** on Linux.
- **VERIFIED:** applying Defect A's fix (`gt2reloc.c:84` -> `MAX_PATHNAME`) and clean
  rebuilding, the pack **STILL segfaults (139)**. So Defect A is real but is **NOT** the
  segfault root cause. Bug 2 has a separate/additional fault deeper in the packing path.

## Steps
1. Branch from `main`. Apply Defect A fix (`gt2reloc.c:84`).
2. Build gt2reloc with `-g -O0 -fsanitize=address` and run the fixture pack to get a
   symbolized stack trace at the ACTUAL fault (the packing path, past `clearsong`, not
   startup). `make linux-rebuild SANITIZE=1` gives an ASan gt2reloc; also try `-O0`.
3. Locate and fix the fault. Pin with a characterization test: fixture in -> assert exit 0
   + non-empty `.prg`.
4. Re-enable coverage now that gt2reloc is healthy:
   - remove `SKIP_GT2RELOC: 1` from the build workflows (`.github/workflows/build-*.yml`);
   - remove `SKIP_USAGE: 1` from the Linux ASan job (added for #76 to dodge Defect A).
5. Update CHANGELOG `### Fixed`.

## Definition of done
- `gt2reloc` packs the fixture (exit 0, non-empty `.prg`); ASan clean on the CLI usage +
  functional checks; `SKIP_GT2RELOC` / ASan-job `SKIP_USAGE` removed; smoke green on 3 OSes.
