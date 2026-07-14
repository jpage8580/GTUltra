# Changelog

All notable changes to GTUltra will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed

- `gt2reloc` segfault when packing a song (issue #71): the relocation-time playback runs on `gtEditorObject`, whose `sidreg[]` pointers are intentionally never wired via `initSID` (only `gtObject` is). The main app marks that object silent (`noSIDWrites = 1`) so `playroutine` skips SID register writes; `gt2reloc` omitted the flag, so it dereferenced NULL `sidreg[i]` (`gplay.c:435`). Now sets `gtEditorObject.noSIDWrites = 1`, matching `gt2stereo.c`. Also fixes an ASan `global-buffer-overflow`: `loadedsongfilename` was declared `[MAX_FILENAME]` (60) in `gt2reloc.c` vs the `[MAX_PATHNAME]` (256) extern, overflowed by `clearsong`'s memset. Re-enables the `gt2reloc` functional smoke test (packs the fixture to both `.prg` and `.sid` with the fixture's canonical flags `-AFF00 -B2 -H1 -U6`, exercising the stereo/2-SID and buffered-write player paths) and the Linux ASan usage checks. Reported by @moraff. Also silenced injected debug output in the vendored assembler (`src/asm/asmtab.c`: forced `yydebug = 1` plus stray `Assemble checkN` / `Starting parse` prints) that flooded the logs on every pack; this realigns with Exomizer 3 upstream, which sets `yydebug = 0` and has none of those prints. Also gave `gt2reloc` a custom `SDL_LogSetOutputFunction` so its `INFO:` lines are identical on Linux/macOS/Windows (macOS otherwise routes `SDL_Log` through `NSLog`, prepending a timestamp/process prefix); the same inconsistency in the other apps is tracked in `known-bugs.md` (Bug 5). Removed an unconditional `debug_0.s` dump (`greloc.c`) that wrote the generated player assembly into the current directory on every relocation/pack (also an unchecked `fopen`); this affected both `gt2reloc` and the `gtultra` export path.
- `-Wpointer-to-int-cast` (11 warnings) in the vendored Exomizer assembler `src/asm/{expr,parse}.c`: debug `LOG()` dump helpers truncated 64-bit pointers via `(u32)` casts. Backported the `%p`/`(void*)` formatting from current Exomizer upstream (each site license-marked). Supersedes PR #13; thanks @drfiemost for the report.

### Changed

- Release workflow: node24 artifact actions, shallow single-tag release checkouts (avoids legacy mixed-case tag collision on case-insensitive runners)

## [1.5.7] - 2026-07-13

### Added

- GitHub Actions release workflow (`workflow_dispatch`) with version/overwrite inputs, per-platform builds (Linux x86_64 + aarch64, macOS arm64, Windows x86_64), Windows runtime DLL bundling, source archive, and SHA256 checksums; pre-releases are auto-detected from a `-rc` tag suffix (no separate input)
- Linux `aarch64` build leg in CI (`build-linux.yml`), matrixed alongside `x86_64` on native ARM runners
- `SANITIZE=1` build switch in `src/makefile.common` (`make <plat>-build SANITIZE=1`): adds `-fsanitize=address,undefined` to compile and link, disables `_FORTIFY_SOURCE`, and skips `strip` for symbolized reports
- Linux ASan/UBSan CI job (`.github/workflows/build-linux.yml`) that builds with `SANITIZE=1` and runs the startup smoke test; a regression gate for the issue #76 class of memory bug (Linux only: MinGW GCC has no libasan and macOS does not exercise the startup path)
- Cross-platform `goatdata.c` determinism gate: `tests/integration/check-goatdata.sh` compares the regenerated player data against the committed reference `tests/goatdata.sha256`, wired into all three build workflows (confirmed byte-identical on Linux/macOS/Windows)
- `.gitattributes` enforcing LF on packed player inputs (`*.s`, `*.seq`, `*.gtp`) and binary handling for packed resources, so generation is identical on every OS
- `CLAUDE.md` agent guide (index of key files and rules) with `AGENTS.md` as a symlink to it
- Project knowledge base under `tests/docs/`: `build-determinism.md`, `testing-strategy.md`, `known-bugs.md`, `handover-issue-76.md`, and follow-up pick-up docs `handover-unused-result.md`, `handover-gt2reloc-bug2.md`, `handover-unit-tests.md`

### Changed

- Linux CI build now uses GCC 16 (via `ubuntu-toolchain-r/test` PPA), superseding GCC 15
- `tests/integration/smoke.sh` now treats a startup `SIGABRT` (exit 134) and any ASan/UBSan/FORTIFY diagnostic as a crash (previously only `139`/`138`), closing the gap that let issue #76 pass CI

### Fixed

- Heap buffer overflow in `setPaletteName()` (`src/gpaletteeditor.c`) that crashed GTUltra at startup under glibc FORTIFY (`*** buffer overflow detected ***`, issue #76): `malloc(strlen(...))` + `strcpy` (one byte short of the NUL) replaced with `strdup`. Fix by @fgaz (PR #73); reported and root-caused by @lunadog (#76)
- `-Wformat-security` warnings from non-literal `sprintf` format strings at `src/gt2stereo.c:1198`, `:1205` and `src/ginfo.c:469`, now `sprintf(buf, "%s", ...)`. Thanks @OPNA2608 (PR #22)
- `src/bme/dat2inc.c` now writes `goatdata.c` in binary mode (`"wt"` → `"wb"`) so the generated file is byte-identical (LF) across Linux, macOS, and Windows

## [1.5.6] - 2026-07-10

### Added

- Cross-platform CI (initial): GitHub Actions workflows building GTUltra on Linux, macOS, and Windows
- Makefile now covers all platforms
- Cross-platform CI smoke tests in `tests/integration/smoke.sh`, run after each build
- Environment-variable toggles to skip smoke-test categories (`SKIP_BINARIES`, `SKIP_USAGE`, `SKIP_GTULTRA`, `SKIP_GT2RELOC`)

### Changed

- Linux CI build now uses GCC 15
- GitHub Actions pinned to commit SHAs for supply-chain safety
- `gt2reloc` functional smoke test is skipped in CI until its segfault is fixed

### Fixed

- Incomplete `checkForMouseInDetailed*` prototypes in `src/goattrk2.h` that broke the Linux build under GCC 15. Thanks @IFEware and @jansalleine

## [1.5.5] - 2026-07-08

- booker/MSL becomes new maintainer
- no functional changes: starting off with a clean slate

### Added

- Root `Makefile` with `clean`, `mac-build`, `mac-rebuild`, and `mac-run` targets
- `mac-run` uses local `HOME` so config lives in `mac/.goattrk/` instead of `~/.goattrk/`
- `mac-rebuild` target for clean + full rebuild
- `src/version.h` as single source of truth for version string
- `.gitignore` for build artifacts, generated source, compiled build tools, and `.d` dependency files
- This changelog

### Changed

- Cleaned repository history: single commit with no binaries, object files, or generated files. 21.78MB down to ~1.23MB+
- Removed committed binaries (`win32/*.exe`, `win32/*.dll`, `linux/*`, `mac/*`)
- Removed committed object files (`*.o`)
- Removed generated source (`src/goatdata.c`, `src/gt2stereo.dat`) from tracking
- Removed unused vendored SDL1 headers (`src/SDL/`)
- Removed committed build tool binaries (`src/bme/dat2inc.exe`, `src/bme/datafile.exe`, `src/datafiles/*.exe`)
- `src/gt2stereo.c` and `src/gt2reloc.c` now use `version.h` instead of hardcoded version strings
- `src/makefile.common`: added `-MMD -MP` flags for automatic header dependency tracking

### Fixed

- `src/makefile.mac`: removed UTF-8 BOM that caused `missing separator` error
- `src/makefile.mac`: fixed broken backtick (`'` → `` ` ``) on CFLAGS line preventing `sdl2-config` from being evaluated
