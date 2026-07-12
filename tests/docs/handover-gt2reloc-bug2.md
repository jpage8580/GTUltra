# Handover: `gt2reloc` segfault (Bug 2) + `loadedsongfilename` overflow

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
