# Handover / tracking: own-code warnings (greloc unused-var, gt2stereo stringop-truncation)

Pick-up doc for the **two own-code, non-`unused-result` warnings** left OPEN after the
Exomizer pointer-cast backport ([handover-asm-pointer-cast.md](handover-asm-pointer-cast.md)).
Both were investigated and root-caused here but **deliberately not fixed in that PR** (kept
Exomizer-only). Inventory: [warnings-tracking.md](warnings-tracking.md).
Related memory-bug tracking: [known-bugs.md](known-bugs.md).

---

## 1. `src/greloc.c:156` — `-Wunused-variable` (`temppackedsongname`)

### Root cause (NOT "just delete it")
`char temppackedsongname[MAX_FILENAME];` is declared unconditionally at `greloc.c:156`, but
its only uses are at `greloc.c:2033` and `:2036`:
```c
memcpy(&temppackedsongname, &packedsongname, MAX_FILENAME);
...
memcpy(&packedsongname, &temppackedsongname, MAX_FILENAME);
```
Both uses sit inside the **`#else` (non-`GT2RELOC`) branch** of the
`#ifdef GT2RELOC ... #else ... #endif` at `greloc.c:1884-2078` (the interactive save path).

So the variable is:
- **used** in the `gtultra` build (compiled **without** `GT2RELOC`) → no warning (this is
  why local macOS builds are clean), and
- **unused** in the `gt2reloc` CLI build (compiled **with** `-DGT2RELOC`, that `#else` code is
  excluded) → the `-Wunused-variable` that shows up in the GCC-16 Linux CI leg.

**An unconditional delete of line 156 would break the `gtultra` build** (it removes a variable
the non-`GT2RELOC` path uses). The old warnings-tracking note ("Trivial: delete the
declaration") was wrong and has been corrected.

### Fix (verified locally, not yet applied on a branch)
Guard the declaration with the same condition as its uses:
```c
#ifndef GT2RELOC
	char temppackedsongname[MAX_FILENAME];   /* only used by the non-GT2RELOC save path below */
#endif
```
Verified: `cc ... -Wall -fsyntax-only greloc.c` is clean **both** with and without
`-DGT2RELOC`; full `make mac-build` succeeds. Severity: **cosmetic** (warning only).

---

## 2. `src/gt2stereo.c:2972` — `-Wstringop-truncation` + latent heap overflow

### The warning and the real bug behind it
In the special-note-name loop (`printnotetable()`-style, `gt2stereo.c` ~2958-2980):
```c
name = malloc(4);
strncpy(name, specialnotenames + j, 2);   // 2 note chars, no NUL  -> -Wstringop-truncation
sprintf(octave, "%d", oct);
strcpy(name + 2, octave);                 // writes strlen(octave)+1 bytes at name+2
```
The `strncpy`-truncation warning is the surface symptom. The **real risk** is the 4-byte
allocation: it fits a single-digit octave (2 note chars + 1 digit + NUL = 4) but a
**two-digit `oct` (>= 10) overflows the heap** (2 + 2 + 1 = 5 > 4) — same class as issue #76.

**Deliberately NOT silenced yet:** swapping `strncpy`→`memcpy` clears the warning without
fixing the overflow, which would hide the signal pointing at the bug. Left loud on purpose.

### Steps to fix (own PR, ASan-verified)
1. Determine whether `oct` can actually reach 10: trace the outer `while (i < 93)` loop, the
   `specialnotenames` table size, and custom tunings / `equaldivisionsperoctave` (a
   microtonal scale could produce many "octaves").
2. If reachable: size the allocation from the formatted length (e.g. `snprintf` into a sized
   buffer, or `malloc(strlen)`-style with the octave measured) and fix the copy; then the
   `memcpy`/`strncpy` truncation can be cleaned up safely.
3. If provably unreachable: document the bound with a comment and only then silence.
4. Verify under ASan (reuse the Linux `SANITIZE=1` job that gated issue #76).

Tracked as **Bug 4** in [known-bugs.md](known-bugs.md).

---

## Definition of done
- `greloc.c:156` guarded with `#ifndef GT2RELOC`; 0 `-Wunused-variable` in **both** build
  configs.
- `gt2stereo.c:2972` allocation correctly sized (or bound proven); 0 `-Wstringop-truncation`;
  no heap overflow under ASan for the reachable octave range.
- `warnings-tracking.md` counts updated; this doc resolved.
