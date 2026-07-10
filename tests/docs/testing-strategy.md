# Testing strategy

How to test GTUltra to a modern-but-realistic bar. Rationale captured so it is not
re-derived. Related: [build-determinism.md](build-determinism.md), [known-bugs.md](known-bugs.md),
[../README.md](../README.md).

## Current state
- **No unit tests.** What exists: `tests/integration/smoke.sh` (binary/usage/functional
  smoke) and `tests/integration/check-goatdata.sh` (determinism gate), both run in CI on
  Linux/macOS/Windows.

## Codebase shape (this drives what is worth testing)
- ~32k LOC of **own** logic: `src/g*.c` + CLI tools. Everything else is **vendored**
  (`src/resid/`, `src/resid-fp/`, `src/asm/`, `src/RtMidi.*`, `src/bme/SDL/`) or
  **generated** (`src/goatdata.c`). Do NOT write tests for vendored/generated code.
- The `g*.c` modules share global/`extern` state via `goattrk2.h` and are coupled to the
  SDL/BME runtime. That coupling makes broad unit coverage low-ROI without refactoring.

## Principle
Target **defects and tool I/O**, not a line-coverage percentage. "Modern quality" for a
real-time C64 audio tool means: CI green on 3 platforms + sanitizers + golden/round-trip
tests + characterization tests around bugs. It does NOT mean 80% unit coverage, and it
does NOT mean a rewrite.

## Priority ladder (highest ROI first)
1. **ASan/UBSan CI build** running `smoke.sh`. Compile with
   `-fsanitize=address,undefined`. Would have caught issue #76 automatically with a stack
   trace. ~1h of Makefile/CI work; do this first.
2. **Golden-file / round-trip tests** on the non-interactive CLI tools
   (`gt2reloc`, `mod2sng2`, `ins2snd2`): feed a fixture, assert stable output
   (hash/size). Add a `.sng` load->save->load byte-stability check. No refactor needed.
3. **Characterization test before each bug fix** (pin current behavior, then fix). See
   `known-bugs.md`.
4. **Extract pure domain functions** (reloc math in `greloc.c`, table-index/undo logic,
   packing helpers) out of the god-modules via sprout/wrap, and unit-test those.

## Framework
Single-header C harness (**Unity** or **greatest**). Add `tests/unit/` + a `make test`
target, wire into the existing workflows. Avoid heavy deps (GoogleTest/Criterion) — the
project prizes minimal dependencies.

## Testable seams vs integration-only
- **Unit-testable:** CLI tools (`file -> file`), reloc math, data conversion, table/undo
  logic, `.sng` (de)serialization.
- **Integration-only (never unit):** the editor UI, SDL/BME runtime, and global-state-
  driven editor flow. This split is normal; do not force-unit the render/IO edge.

## Why not a rewrite (to another language)
- The value is the 6510 asm player (embedded via `goatdata.c`) + reSID + exact `.sng`
  format compatibility. A rewrite must preserve all three bit-for-bit, i.e. re-implement
  the hardest ~32k and discard 25 years of accumulated edge-case fixes.
- Justified only if the goal is a fundamentally different product (web/WASM, VST/AU
  plugin, modern GUI). Even then prefer the **strangler-fig** path: carve the domain core
  (song model, file I/O, reloc, packing) into a testable library the existing app links
  against, rather than a big-bang rewrite.
