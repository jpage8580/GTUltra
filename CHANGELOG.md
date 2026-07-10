# Changelog

All notable changes to GTUltra will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Cross-platform `goatdata.c` determinism gate: `tests/integration/check-goatdata.sh` compares the regenerated player data against the committed reference `tests/goatdata.sha256`, wired into all three build workflows (confirmed byte-identical on Linux/macOS/Windows)
- `.gitattributes` enforcing LF on packed player inputs (`*.s`, `*.seq`, `*.gtp`) and binary handling for packed resources, so generation is identical on every OS
- `CLAUDE.md` agent guide (index of key files and rules) with `AGENTS.md` as a symlink to it
- Project knowledge base under `tests/docs/`: `build-determinism.md`, `testing-strategy.md`, `known-bugs.md`, `handover-issue-76.md`

### Fixed

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
