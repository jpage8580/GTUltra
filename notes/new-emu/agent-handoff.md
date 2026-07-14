# Agent handoff - SID engine migration (branch `new-emulators`)

Short orientation for the next agent picking up this work. Living detail is in the tracker;
this file is the "where do I start / what will bite me" summary.

## Start here
1. `notes/new-emu/sid-engine-migration-tracking.md` -> the **"Status snapshot (read this first)"**
   block (current state, commit hashes, loose ends, determinism results).
2. `notes/new-emu/sid-engine-migration-plan.md` -> authoritative, self-contained plan
   (M0-M7 + source-anchors appendix).
3. Memory `sid-emulator-migration.md` summarizes both. Corpus shortlist:
   `golden-corpus-candidates.md`. Editor-vs-tool fidelity: `gtultra-vs-sng2wav.md`.
   Render tool manual: `docs/sng2wav.md`.

## Done (committed, NOT pushed; tip `3c1a0c6`, 10 commits this session)
- **gt2reloc** mac headless-SDL hang fixed (`SDL_MAIN_HANDLED`, non-Windows).
- **sng2wav** - headless `.sng`->WAV renderer + recursive `--scan` box table. Built on
  `src/cli_common.c` (extracted from gt2reloc, verified byte-identical) + `sound_init_offline`
  in `gsound.c`; `songfilename` overflow/truncation fixed (benefits gt2reloc too).
- **Determinism proven** (`--seed` byte-identical; results table in the tracker).
- Docs self-contained + cross-linked; `tests/docs/warnings-tracking.md` updated.
- `make mac-rebuild` clean (exit 0); new code warning-free under `-Wall`.

## Next (finish M0, then M1)
1. Maintainer picks the golden corpus (shortlist in `golden-corpus-candidates.md`); capture
   golden WAVs with `sng2wav ... --seed`.
2. Stand up `tests/unit/` + a `make test` target (first unit suite for the project).
3. **M1** = introduce the `ISidEngine` wrapper (plan §M1) - highest-leverage step; must be
   **bit-identical** (verify with `sng2wav` before/after, same args/seed).

## Gotchas (read before building/running)
- **Headless runs go on Linux**, not a no-GUI mac shell (SDL Cocoa bootstrap; scope §3a).
  Building on mac is fine; *running* SDL binaries from an agent shell can hang on a dialog.
- **macOS warnings are pre-existing/vendored** - the table in `warnings-tracking.md` says which;
  the new engine/tool code is clean. Don't chase them as regressions.
- **Merge rules** (CLAUDE.md): squash only; PR body = `refer to CHANGELOG for details`; and
  **write CHANGELOG `[Unreleased]` entries at PR time** (gt2reloc fix, songfilename fix,
  sng2wav) - not yet written.
- **Never edit `_upstream/*`**; new-engine edits are carried as `patches/*.patch`.
- New engines are **compile-gated** (`WITH_RESID_VICE` / `WITH_RESIDFP_LIB`, default off) and
  **runtime-gated** (`sidbackend` cfg key) - keep default builds byte-identical (plan §3).
- **`--seed` is a no-op for most tunes**: its only live consumer in `sng2wav` is the pan-spread,
  which needs TrueStereo **and** a pan-range instrument; backend/interp don't matter. Also
  **glibc maps `srand(0)`->`srand(1)`**, so seeds 0 and 1 are identical - don't test seed
  sensitivity with that pair. Full trace in the tracker ("What `--seed` actually controls").

## Loose ends explicitly tracked
- CHANGELOG entries (above).
- `make test` harness + golden capture (M0 remainder).
- gt2reloc SDL fix not yet validated on a true no-GUI/CI mac runner.

Tree is clean, nothing pushed. The instrument (`sng2wav`) is ready; M1 is the real work.
