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
| `-Wpointer-to-int-cast` | 11 | vendored `src/asm/` (`expr.c` ×9, `parse.c` ×2). Existing fix: **PR #13** — see [handover-asm-pointer-cast.md](handover-asm-pointer-cast.md) |
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
(`pointer-to-int-cast`, `uninitialized`, `maybe-uninitialized`, `parentheses`,
`unused-function`, `unused-value`). Those are upstream engine/parser warnings; leave them.

**Own code** (`src/g*.c`, CLI tools) warnings are almost entirely `-Wunused-result`
(deferred sweep). The only two own-code, non-`unused-result` warnings are:

1. **`src/greloc.c:156` — `-Wunused-variable`** (`char temppackedsongname[MAX_FILENAME];`
   declared, never used). Trivial: delete the declaration. Cosmetic.
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
   allocation properly. Track/verify like the other memory bugs (ASan).

## Priority
1. `src/gt2stereo.c:2972` — verify the octave range; fix if a 2-digit octave is reachable
   (potential heap overflow, same class as issue #76).
2. `-Wunused-result` sweep — [handover-unused-result.md](handover-unused-result.md).
3. `src/greloc.c:156` — delete the unused variable (cosmetic).
4. Vendored warnings — out of scope (do not modify vendored trees), EXCEPT the 11
   `-Wpointer-to-int-cast` in `src/asm/`, which have a ready fix in PR #13 pending a
   vendored-policy call → [handover-asm-pointer-cast.md](handover-asm-pointer-cast.md).

## Note on this iteration
The issue-#76 PR **removed** 3 `-Wformat-security` warnings and **adds none**; the counts
above are pre-existing on `main` (not introduced here).
