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
   `-fsanitize=address,undefined`. **DONE** (issue #76): `make <plat>-build SANITIZE=1`
   (`src/makefile.common`) + a Linux job in `build-linux.yml`. Caught #76 with a symbolized
   trace. See [handover-issue-76.md](handover-issue-76.md) and the coverage note below.
2. **Golden-file / round-trip tests** on the non-interactive CLI tools
   (`gt2reloc`, `mod2sng2`, `ins2snd2`): feed a fixture, assert stable output
   (hash/size). Add a `.sng` load->save->load byte-stability check. No refactor needed.
3. **Characterization test before each bug fix** (pin current behavior, then fix). See
   `known-bugs.md`.
4. **Extract pure domain functions** (reloc math in `greloc.c`, table-index/undo logic,
   packing helpers) out of the god-modules via sprout/wrap, and unit-test those.

## Sanitizer platform coverage (Linux-only gate, and why)
The ASan/UBSan gate runs on **Linux only**. This was decided from evidence, not
convenience; do not "add the other platforms" without re-reading this.
- **Linux (gcc):** full ASan+UBSan, and it reproduces #76 exactly (glibc FORTIFY abort +
  symbolized ASan trace, headless). This is the real gate.
- **macOS (Apple clang):** ASan+UBSan exist and a standalone control catches the exact
  `malloc(strlen)+strcpy` pattern, BUT the instrumented `gtultra` reaches the Cocoa event
  loop without firing (startup overflow path not exercised the way Linux does; root cause
  undetermined — likely the SDL/Cocoa `main` shim or allocator path). Also no FORTIFY and a
  16-byte min malloc bucket, so a normal build never crashes. **A mac ASan job would be
  GREEN with the bug present → not a valid gate.** macOS stays normal build+smoke.
- **Windows:** MinGW-w64 GCC (current CI) ships **no libasan** (`--disable-libsanitizer`;
  `cannot find -lasan`); UBSan only in diagnostic-less trap mode. MSVC has ASan but **never
  UBSan**, and needs a full Makefile→MSBuild + source port. MSYS2 CLANG64 has both but
  forces a whole-toolchain migration (libc++/UCRT ABI rebuild of all C++, fix
  `-fpermissive`-masked code, drop `-static`+`strip`, PATH the asan runtime DLL). All are
  costly/lower-coverage. Windows stays normal build+smoke.
- Cheap optional extra (not done): `-fsanitize=undefined -fsanitize-trap=undefined
  -fno-sanitize-recover=all` on the existing MinGW job gives diagnostic-less UB-crash
  coverage with no runtime.

Bottom line: ASan/UBSan catch language-level, platform-independent defects, so one Linux
build catches the overwhelming majority. Spend effort on Linux depth, not OS breadth.

## Framework

**Decision (2026-07-14): `greatest` (single vendored header) for the C core now; `doctest`
reserved for the C++ layer if/when the Renoise-style GUI revamp lands.** Rationale recorded
here because this is a foundational, hard-to-reverse choice for the whole test scope.

### Language split that drives the choice (measured, own code only)
- **C: 26 `.c` files, ~30k LOC** - all domain/editor/IO/CLI logic (`gsong, gorder, gpattern,
  ginstr, gtable, gplay, greloc, gsound, gdisplay, gchareditor, gconsole, ginfo, gfile,
  gfkeys, ghelp, gundo, gpaletteeditor, gtabledisplay, gmidiselect`, the CLI tools,
  `cli_common`). This is the bulk of what is unit-testable. (`goatdata.c` ~49k is generated -
  never tested.)
- **C++: 2 files, 559 LOC** - `gsid.cpp` (438, SID glue) + `gmidi.cpp` (121). Tiny today.
- **Vendored (never test):** C++ `resid/` (16), `resid-fp/` (8), `RtMidi.cpp`; C `bme/` (10),
  `asm/` (11); 6510 `.s` player.
- Build already links mixed C/C++ via `g++` (reSID is C++); 3 pure-C CLI tools link with `cc`.
- **Ratio today ≈ 98% C / 2% C++** in own code.

### Why `greatest` now (not Unity / Criterion / GoogleTest / doctest)
- Codebase is **C-dominant**, so a C++-only framework (GoogleTest / Catch2 / doctest) is the
  wrong spine now - it would force C++ wrappers around every C function under test.
- `greatest` is **one ISC-licensed header, C *and* C++ from the same file, zero link step,
  no install** - so it works identically on Linux/macOS/Windows with none of the toolchain
  tax that a linked lib (Criterion) or an MSVC/MinGW C++ framework would add (cf. the
  sanitizer-coverage section above on how painful Windows toolchains already are).
- Features that scale to a large suite: test filtering (`-t`), suites, `SKIP`, TAP output,
  `ASSERT_EQ_FMT`, `ASSERT_STR_EQ`, `ASSERT_IN_RANGE` (covers the pan-gain/filter float
  tolerances in the SID unit tests #1/#3).
- **Unity** was the runner-up (nicer typed float asserts) but ships a separate `unity.c` TU
  and leans on a Ruby generator for ergonomic discovery - more wiring, a dependency smell.
- The one-time network fetch is not a recurring dep: `tests/unit/greatest.h` is **vendored
  once and committed**, offline forever after (same posture as `src/` vendored code).

### Future: the Renoise-style GUI revamp
A Renoise-like revamp is a large **new C++** surface, so the C++ share will grow. But:
- GUI render/event code stays **integration-only, never unit** (see next section). What
  becomes unit-testable is any **carved-out C++ core** (view-models, command/undo, selection
  state) if the revamp is architected MVVM-style.
- At that point add **`doctest`** (single MIT header, C++-native, fastest-compiling, no link
  step - same zero-friction posture as greatest) **for the C++ side only**.
- greatest and doctest coexist cleanly because they compile into **disjoint translation
  units** (C tests vs C++ tests), each linked into its own `make test` sub-binary - no
  shared-runner conflict. This is the *only* sanctioned "two harnesses" split: partition by
  language, never overlap on the same code.
- **Do not** stand up doctest until there is real unit-testable C++ to point it at
  (scaffolding-ahead-of-code is waste).

### Mechanics
Add `tests/unit/` (`greatest.h` + `test_*.c`) + a `make test` target, built under
`-fsanitize=address,undefined` (ladder #1), wired into the CI workflows. Avoid heavy deps
(GoogleTest/Criterion) - the project prizes minimal dependencies.

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
