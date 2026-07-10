# GTUltra
C64 SID tracker (GoatTracker Stereo fork). ~32k LOC own logic; rest vendored/generated.

## Index
- Build/run: `Makefile` (`{mac,linux,win}-build`, `mac-run`)
- CI: `.github/workflows/build-{linux,macos,windows}.yml`
- Tests: `tests/README.md`
- Build determinism + `goatdata.c` rules: `tests/docs/build-determinism.md`
- Known bugs / fixes: `tests/docs/known-bugs.md`
- Testing strategy: `tests/docs/testing-strategy.md`
- Main app (`gtultra`): `src/gt2stereo.c`; app logic: `src/g*.c`
- CLI tools: `src/{gt2reloc,ins2snd2,mod2sng2,ss2stereo}.c`
- 6510 player: `src/player*.s`, `src/altplayer*.s` (assembler: `src/asm/`)

## Rules
- `src/goatdata.c` is generated: never commit/edit; change packed inputs → follow `tests/docs/build-determinism.md`.
- Do not modify vendored: `src/resid/`, `src/resid-fp/`, `src/asm/`, `src/RtMidi.*`, `src/bme/SDL/`.
- Commits: single-line message; CHANGELOG carries the detail, updated only **after** the PR is ready to merge.
- Merge to `main` by **squash** only.
- Do not bloat this file, leverage `## Index` above

