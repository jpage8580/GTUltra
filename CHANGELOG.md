# Changelog

All notable changes to GTUltra will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v1.5.5] - 2026-07-08

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
