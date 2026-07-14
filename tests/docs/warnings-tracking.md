# Compiler warnings tracking

Inventory of build warnings, to triage and burn down over time. **Not fixed in the
issue-#76 iteration** — this doc just records the current state so it is not lost.
Related: [handover-unused-result.md](handover-unused-result.md),
[testing-strategy.md](testing-strategy.md), [known-bugs.md](known-bugs.md).

## Source / how to regenerate
From a full Linux build under `-Wall -O3` (GCC 16, as in CI `build-linux.yml`):
```
make linux-rebuild 2>&1 | tee /tmp/build.log
grep -oE '\[-W[a-z0-9-]+\]' /tmp/build.log | sort | uniq -c | sort -rn   # by type
grep -n 'warning:' /tmp/build.log                                        # full list
```
Snapshot below is from the GCC-16 CI build (one leg): ~159 `warning:` lines.

## By type
| warning | count | notes |
|---------|-------|-------|
| `-Wunused-result` | 83 | `fread`/`fgets`/`write`/`chdir`/`getcwd` return values ignored. Dedicated sweep: [handover-unused-result.md](handover-unused-result.md) |
| `-Wpointer-to-int-cast` | 0 | **Fixed** in `src/asm/{expr,parse}.c` — backported Exomizer upstream `%p`/`(void*)` formatting (was `(u32)` truncating 64-bit pointers). See [handover-asm-pointer-cast.md](handover-asm-pointer-cast.md) (supersedes PR #13, @drfiemost) |
| `-Wparentheses` | 6 | vendored |
| `-Wunused-function` | 3 | vendored |
| `-Wmaybe-uninitialized` | 2 | vendored (`resid`/`bme`) |
| `-Wuninitialized` | 2 | vendored |
| `-Wunused-variable` | 1 | own: `src/greloc.c:156` (`temppackedsongname` unused) — cosmetic |
| `-Wunused-value` | 1 | vendored |
| `-Wstringop-truncation` | 1 | own: `src/gt2stereo.c:2972` — see below |

## Own-code vs vendored
Vendored (**do NOT modify** per `CLAUDE.md`: `src/asm/`, `src/resid/`, `src/resid-fp/`,
`src/bme/`, `src/RtMidi.*`) accounts for **every** potentially-serious class
(`uninitialized`, `maybe-uninitialized`, `parentheses`,
`unused-function`, `unused-value`). Those are upstream engine/parser warnings; leave them.
(`pointer-to-int-cast` was in this set but is now fixed via an Exomizer-upstream backport.)

**Own code** (`src/g*.c`, CLI tools) warnings are almost entirely `-Wunused-result`
(deferred sweep). The only two own-code, non-`unused-result` warnings are:

1. **`src/greloc.c:156` — `-Wunused-variable`** — **NOT "never used"; do NOT just delete.**
   `temppackedsongname` is used at `greloc.c:2033`/`2036`, but only inside the `#else`
   (non-`GT2RELOC`) branch of `#ifdef GT2RELOC` (`greloc.c:1884-2078`). So it is used in the
   `gtultra` build and unused only in the `-DGT2RELOC` gt2reloc build (where CI's GCC-16 leg
   flags it). Deleting line 156 would break the gtultra build. Fix: guard the declaration
   with `#ifndef GT2RELOC`. Cosmetic. Full analysis + verified fix:
   [handover-owncode-warnings.md](handover-owncode-warnings.md).
2. **`src/gt2stereo.c:2972` — `-Wstringop-truncation`** — and this one deserves a look, it
   may be a latent heap overflow:
   ```c
   name = malloc(4);
   strncpy(name, specialnotenames + j, 2);   // 2 bytes, no NUL
   sprintf(octave, "%d", oct);
   strcpy(name + 2, octave);                 // writes strlen(octave)+1 at name+2
   ```
   The 4-byte buffer fits a single-digit octave (2 note chars + 1 digit + NUL = 4), but a
   **two-digit `oct` (>= 10) overflows** (2 + 2 + 1 = 5 > 4). Confirm the octave range
   (custom tunings / `equaldivisionsperoctave` may exceed 9); if reachable, size the
   allocation properly. Track/verify like the other memory bugs (ASan). Kept loud on
   purpose (silencing the warning would hide the overflow signal). Pick-up:
   [handover-owncode-warnings.md](handover-owncode-warnings.md), Bug 4 in
   [known-bugs.md](known-bugs.md).

## Priority
1. `src/gt2stereo.c:2972` — verify the octave range; fix if a 2-digit octave is reachable
   (potential heap overflow, same class as issue #76) →
   [handover-owncode-warnings.md](handover-owncode-warnings.md).
2. `-Wunused-result` sweep — [handover-unused-result.md](handover-unused-result.md).
3. `src/greloc.c:156` — guard the declaration with `#ifndef GT2RELOC` (do NOT delete;
   cosmetic) → [handover-owncode-warnings.md](handover-owncode-warnings.md).
4. Vendored warnings — out of scope (do not modify vendored trees). The 11
   `-Wpointer-to-int-cast` in `src/asm/` are **DONE** (backported from Exomizer upstream;
   `src/asm/` is Exomizer's embedded assembler) →
   [handover-asm-pointer-cast.md](handover-asm-pointer-cast.md).

## macOS / clang rebuild, and the SID-migration work (branch `new-emulators`, 2026-07-14)

The snapshot above is the **GCC-16 / Linux** build. A **macOS `make mac-rebuild`** (Apple
clang) is clean (exit 0, all binaries) but clang emits some **extra warning classes GCC does
not** - all in **pre-existing / vendored** code, none from the migration work. Listed here so a
next agent does not mistake them for new regressions:

| Where | clang warning | own/vendored |
|---|---|---|
| `src/resid/wave.cpp` (71/76/81/86), `resid/filter.cpp` (229/235) | `-Wbitwise-op-parentheses` (`&` within `|`) | vendored (do not touch) |
| `src/resid/sid.cpp:128/134` | `-Wtautological-constant-out-of-range-compare` | vendored |
| `src/RtMidi.cpp:1576` | `-Wvla-cxx-extension` | vendored |
| `src/asm/lexyy.c` (1715/1761/2139) | `-Wunused-function` / `-Wunneeded-internal-declaration` | generated/vendored |
| `src/gdisplay.c:700/1143` | `-Wtautological-constant-out-of-range-compare` | own/editor, pre-existing |
| `src/gt2stereo.c:3282/3284` | `-Wparentheses-equality` | own/editor, pre-existing |
| `src/greloc.c:156` | `-Wunused-variable` (`temppackedsongname`) | own, pre-existing (see above) |

**The SID-migration additions are warning-free.** `src/sng2wav.c` and `src/cli_common.c`
compile with **zero warnings under `-Wall`** (verified on clang; the only things `-Wextra`
surfaces are `-Wunused-parameter` in the intentional editor **stubs** in `cli_common.c`, which
`-Wall` does not enable). Extracting `cli_common.c` out of `gt2reloc.c` added **no new
warnings** to `gt2reloc` - its lone warning (`greloc.c:156`) predates this work and lives in
`greloc.c`, not the tool. `src/gsound.c` (`sound_init_offline`) and the `songfilename`
`MAX_PATHNAME` fix add none.

Bottom line for a picker-upper: if you build on macOS and see warnings, they are the
vendored/editor ones in the table above (or the GCC list earlier) - **not** from the new
`engines/`, `sng2wav`, or `cli_common` code.

## Note on this iteration
The issue-#76 PR **removed** 3 `-Wformat-security` warnings and **adds none**; the counts
above are pre-existing on `main` (not introduced here).
