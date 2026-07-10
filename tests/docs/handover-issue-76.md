# Handover: issue #76 — palette-name buffer overflow

Pick-up doc for the next work unit. Background/analysis in
[known-bugs.md](known-bugs.md). Do this on a fresh branch cut from `main` after PR #83
merges.

## Goal
Fix the heap buffer overflow that crashes GTUltra at startup
(`*** buffer overflow detected ***: terminated`, GitHub issue #76), and verify it.

## Credit (put in CHANGELOG when merged)
- Fix by **@fgaz** (PR #73 "Fix buffer overflow in setPaletteName()", branch
  `fix-buffer-overflow`).
- Reported and root-caused (spotted #73 fixes it) by **@lunadog** (issue #76).

## The fix
`src/gpaletteeditor.c:622` in `setPaletteName()`:
```c
paletteNames[index] = malloc(strlen(paletteName));  // 1 byte short: no NUL
strcpy(paletteNames[index], paletteName);
```
Replace both lines with:
```c
paletteNames[index] = strdup(paletteName);          // allocates strlen+1, copies
```
This is exactly PR #73. Prefer **adopting/crediting** that PR (cherry-pick or apply with
attribution) over reimplementing.

## Steps
1. Branch from `main` (e.g. `resolve-issues-2` or `fix/issue-76-palette-overflow`).
2. **Add an ASan/UBSan CI build** running `tests/integration/smoke.sh` compiled with
   `-fsanitize=address,undefined` (testing-ladder item #1). This is the verification
   mechanism: it reproduces the overflow before the fix and proves it gone after.
   See [testing-strategy.md](testing-strategy.md).
3. Confirm the overflow reproduces under ASan (characterization: red before fix).
4. Apply the `strdup` fix.
5. Confirm ASan is clean (green after fix).
6. (Optional, same report) clean the `-Wformat-security` warnings:
   `src/gt2stereo.c:1198` and `:1205` — `sprintf(infoTextBuffer, keyOffsetText);` →
   `sprintf(infoTextBuffer, "%s", keyOffsetText);`. Plus the unchecked `chdir`/`fgets`
   `-Wunused-result` warnings if in scope.
7. Update CHANGELOG (`### Fixed`) with the credit above, then squash-merge.

## Definition of done
- ASan build in CI, green with the fix, and demonstrably red without it.
- issue #76 closable (crash gone; warnings addressed or explicitly deferred).
