# GTUltra
C64 SID tracker (GoatTracker Stereo fork). ~32k LOC own logic; rest vendored/generated.

## Index
- Build/run: `Makefile` (`{mac,linux,win}-build`, `mac-run`)
- CI: `.github/workflows/build-{linux,macos,windows}.yml`
- Tests: `tests/README.md`
- Build determinism + `goatdata.c` rules: `tests/docs/build-determinism.md`
- Known bugs / fixes: `tests/docs/known-bugs.md`
- Testing strategy: `tests/docs/testing-strategy.md`
- Handover / follow-ups: `tests/docs/handover-*.md` (issue-76, unused-result, gt2reloc-bug2, unit-tests, asm-pointer-cast, owncode-warnings)
- Compiler warnings inventory: `tests/docs/warnings-tracking.md`
- Main app (`gtultra`): `src/gt2stereo.c`; app logic: `src/g*.c`
- CLI tools: `src/{gt2reloc,ins2snd2,mod2sng2,ss2stereo,sng2wav}.c` (headless `.sng`->WAV + `--scan`: `docs/sng2wav.md`); shared headless boilerplate `src/cli_common.c`
- 6510 player: `src/player*.s`, `src/altplayer*.s` (assembler: `src/asm/`)

## Rules
- `src/goatdata.c` is generated: never commit/edit; change packed inputs → follow `tests/docs/build-determinism.md`.
- Do not modify vendored: `src/resid/`, `src/resid-fp/`, `src/asm/`, `src/RtMidi.*`, `src/bme/SDL/`.
- Commits: single-line message; CHANGELOG carries the detail, updated only **after** the PR is ready to merge. New entries go under `[Unreleased]` — do not assign a version number until release.
- Version bump (at release): update **both** together, `CHANGELOG.md` (promote `[Unreleased]` → `[X.Y.Z] - DATE`) **and** `src/version.h` (`GTULTRA_VERSION`). They must stay in sync.
- Do not bloat this file, leverage `## Index` above

## Running SDL tools headless (agents/CI)
- `gtultra` and most CLI tools (`gt2reloc`, ...) link SDL and open a window. From a **non-GUI shell on macOS** they hang on an SDL startup dialog — `SDL_VIDEODRIVER=dummy` does **not** fix it there; don't run those mac builds from an agent shell. (A real mac Terminal / GUI session runs them fine.)
- **Exception — `sng2wav` is windowless by design** (offline `sound_init_offline` path, no editor/device) and **runs fine locally on macOS from an agent shell** — no hang. Prefer the local `mac/sng2wav` (build `make mac-build`) over Lima/Linux for `.sng`→WAV renders and `--scan`; it operates on the working tree directly (no file sync). Verified via `--scan`; the render path uses the same windowless code.
- To run/verify the *windowed* tools headless, use a **Linux** build (CI, or a local Linux VM): `make linux-build [SANITIZE=1]`, then run with `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ASAN_OPTIONS=detect_leaks=0`. Fixture: `tests/fixtures/Stereo_Pendejo.sng`.

## Merge rules
- Merge to `main` by **squash** only: `gh pr merge {PR} --squash --delete-branch`.
- By default the **PR body** and the **squash merge commit body** are exactly the single line `refer to CHANGELOG for details` — never duplicate CHANGELOG content in either.
- No `Co-Authored-By` trailer on commits; no generated-by footer on PRs.
- **Exception — bulk-included contributions:** when a squash folds other contributors' work into a larger change (e.g. adopting or superseding their PRs), the squash-merge commit body extends beyond that single line to add (a) a concise one-line note of what was included, and (b) correct attribution — the commit `Author` for the primary contributor and/or `Co-authored-by:` trailers for each. Trailers are used only in this case.

