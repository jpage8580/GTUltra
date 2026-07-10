# Task: Known functional bugs & their fixes

Status: **identified, not yet fixed**
Related: [build-determinism.md](build-determinism.md)

Approach for each: **pin behavior with a characterization/regression test FIRST**, then
fix, so the fix is provably correct and stays fixed.

---

## Bug 1 — Buffer overflow in `setPaletteName()` (GitHub issue #76)

- **Where:** `src/gpaletteeditor.c:616` `setPaletteName()`, defect at line 622.
- **Defect:**
  ```c
  paletteNames[index] = malloc(strlen(paletteName));  // 1 byte too few (no NUL)
  strcpy(paletteNames[index], paletteName);           // writes strlen+1 bytes
  ```
  A 1-byte heap overflow. glibc FORTIFY (`_FORTIFY_SOURCE`, active under `-O3`) detects it
  and aborts: `*** buffer overflow detected ***: terminated` — the crash reported in #76.
- **Status on `main`:** STILL PRESENT (verified). Both `main` and `resolve-PRs` at `d3bf412`.
- **Fix:** replace with `strdup(paletteName)` (allocates strlen+1 and copies).
  This is exactly **PR #73** ("Fix buffer overflow in setPaletteName()"), which is
  **still OPEN / unmerged**; its fix did not make it into the 2nd-life tree.
- **Test idea:** call `setPaletteName()` with a known string, assert the stored string is
  NUL-terminated and equal; run the suite under ASan to catch the overflow directly.

### Sub-item — format-security warnings (same issue #76 report)
- `src/gt2stereo.c:1198` and `:1205`: `sprintf(infoTextBuffer, keyOffsetText);`
  → `-Wformat-security` (format not a string literal). Fix: `sprintf(infoTextBuffer, "%s", keyOffsetText);`
- Unchecked `chdir`/`fgets` return values (`-Wunused-result`) also still present.
- Cosmetic (non-fatal) but cheap to clean up while touching the file.

---

## Bug 2 — `gt2reloc` segfaults when packing a `.sng`

- **Where:** `gt2reloc` CLI tool (command-line packer/relocator).
- **Symptom:** on the fixture `tests/fixtures/Stereo_Pendejo.sng` it segfaults during
  packing (`EXIT: 139` on macOS). Documented in `tests/README.md`.
- **Status:** the functional smoke test is **kept blocking** so the regression stays
  visible in CI.
- **Next step:** reproduce, get a stack trace (build with `-fsanitize=address` /
  `-g -O0`), locate the fault, pin with a characterization test on the fixture
  (assert exit 0 + non-empty `.prg`), then fix.

---

## Note (not scheduled) — `ss2stereo` legacy limitation

- `ss2stereo` (original GoatTracker v2.76 stereo splitter) has a hardcoded
  `MAX_SONGLEN` / orderlist length ≤ 254 that modern GTUltra songs exceed:
  `ERROR: Orderlist-length of 254 exceeded!`
- Excluded from the functional smoke test. This is a legacy-tool limitation, not a
  GTUltra bug. Fix the limit before re-adding it to functional smoke tests.

---

## Highest-leverage cross-cutting move

Add **ASan/UBSan** to a CI build running the existing `tests/integration/smoke.sh`.
It would have caught Bug 1 automatically with a stack trace, and will help localize
Bug 2. See also the sanitizer recommendation referenced from the testing strategy.
