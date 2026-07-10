# Task: Build determinism & reproducibility of `goatdata.c`

Status: **tasks 1-3 implemented (on `resolve-issues-1`); cross-platform confirmation pending first CI run**
Related: [known-bugs.md](known-bugs.md) (the `dat2inc` fix below is a tool change; bug fixes touch tools too)

## Background

`src/goatdata.c` is the C64 SID player + UI resources embedded as a C byte array.
It is a **generated build artifact**, not authored source.

Generation pipeline (see `Makefile` `build-tools` target):

```
gt2stereo.seq  ──(bme/datafile)──▶  gt2stereo.dat  ──(bme/dat2inc)──▶  goatdata.c  ──(cc)──▶ linked into app
```

`gt2stereo.seq` lists 23 packed inputs: 8 player sources
(`player.s`, `altplayer.s`, `player3.s`, `altplayer3.s`, `player9.s`, `altplayer9.s`,
`player12.s`, `altplayer12.s`) + 15 resources
(`chargen.bin`, `palette.bin`, `bcursor.bin`, `cursor.bin`, `goattrk2.bmp`,
`0default.gtp`..`8default.gtp`, `goat32.png`).

## What we established (evidence)

Branch hygiene differs between the two lineages:

| | `goatdata.c` |
|---|---|
| `origin/master` (old upstream) | **Committed** (blob `bc4a33a`) |
| `origin/main` (2nd life) | **Not tracked**, gitignored (`.gitignore:12`) |

Measured on macOS/clang (`git hash-object`):

| Artifact | blob hash | note |
|---|---|---|
| Fresh regen from committed sources | `2dc1e3f` | |
| `main` on-disk `goatdata.c` | `2dc1e3f` | **matches fresh regen — correct/current** |
| `master` committed `goatdata.c` | `bc4a33a` | **does NOT match its own sources — STALE** |

Supporting numbers: sum of committed packed inputs = 495,754 bytes (identical on both
branches). Payload embedded in each file: main/regen = 496,245 bytes; master = 512,317
bytes (~16 KB extra). master's committed artifact was generated from an older, larger
input set and never regenerated.

All 23 packed inputs **and** both generator tools (`datafile.c`, `dat2inc.c`) are
byte-identical across `master` and `main`. The player itself is unchanged; only master's
committed generated file drifted.

### Conclusions
1. Generation is **deterministic on a single platform**: regen == on-disk (`2dc1e3f`).
2. `main`'s approach (gitignore the artifact) is correct. master's committed copy is a
   textbook case of a committed generated file silently going stale.
3. Cross-platform determinism is **not yet guaranteed** (see risks below).

## Cross-platform risks (why "just hash on 3 platforms" would mislead)

1. **No `.gitattributes`.** `datafile` packs inputs in binary mode
   (`datafile.c:141` `"rb"`), so it packs whatever bytes are on disk. On Windows with
   default `core.autocrlf=true`, git checks out text inputs (`*.s`, `*.seq`, `*.gtp`) as
   CRLF, changing the packed bytes → different (and possibly wrong) embedded player.
   This is a **build-correctness** risk, not just a test risk.
2. **`dat2inc` writes the `.c` in text mode** (`dat2inc.c:25` `"wt"`, emits `\n`).
   On Windows text mode rewrites `\n`→`\r\n`, so `goatdata.c` itself gets CRLF and hashes
   differently even when the payload is identical. Cosmetic for the compiled binary
   (compiler ignores line endings) but breaks a naive cross-platform hash comparison.
3. `gt2stereo.dat` is written binary (`datafile.c:54` `"wb"`); all 3 targets are
   little-endian, so the binary container is stable given identical inputs.

## Tasks (iterative)

- [x] **1. Add `.gitattributes`** — forces LF on packed text inputs (`*.s`, `*.seq`,
  `*.gtp`) and marks binaries `binary` (`*.bin`, `*.bmp`, `*.png`, plus `*.sng`/`*.prg`/
  `*.sid`/`*.dat`). Verified `git add --renormalize .` produces no diff (inputs were
  already LF), so no normalization churn. Protects the actual Windows build.
- [x] **2. `dat2inc.c` `"wt"` → `"wb"`** (`src/bme/dat2inc.c:25`) — emits LF-only
  `goatdata.c` on every OS. Verified on macOS: regeneration with the fixed tool still
  yields the same bytes (`sha256 b365a479…`), so no behavior change on Unix.
- [x] **3. CI regen-and-hash gate** — `tests/integration/check-goatdata.sh` compares the
  freshly built `src/goatdata.c` against the committed reference `tests/goatdata.sha256`
  (`b365a479…`). Wired into all three workflows (`build-{linux,macos,windows}.yml`) as a
  "Verify goatdata.c determinism" step between Build and smoke tests. Catches both
  platform divergence and master-style staleness. Self-tested locally: PASS.
- [x] **4. 3-platform confirmation** — PR #83 CI: all three platforms regenerated
  `goatdata.c` and the determinism step logged the same hash (`b365a479…`) on Linux,
  macOS, and Windows. Cross-platform determinism confirmed; the gate now guards it on
  every PR. (If Windows ever fails here, suspect a packed input arriving CRLF despite
  `.gitattributes` / a `core.autocrlf` interaction — investigate before relaxing the gate.)

## Reference-update procedure (when player sources legitimately change)
```
make build-tools
shasum -a 256 src/goatdata.c | awk '{print $1}' > tests/goatdata.sha256
```
Commit the new `tests/goatdata.sha256` alongside the source change.

## Design notes / limitations
- **The gate is a change-detector, not an independent oracle.** `tests/goatdata.sha256` is
  a committed baseline (lockfile-style). It reliably catches *accidental* drift and
  platform divergence, because all platforms compare to the same pinned value. It can be
  defeated only by regenerating the reference on a non-deterministic platform and
  committing that — but then the other two platforms fail against it on the next run, so
  even that surfaces quickly. When player sources change legitimately, the reference MUST
  be updated in the same commit (see procedure above) or CI correctly fails.
- **Alternative considered and rejected:** each OS uploads its generated `goatdata.c` as a
  CI artifact and a final job cross-diffs all three (no committed hash). This needs no
  human-maintained reference but does NOT catch staleness (drift from sources), which is
  the master-style failure we most want to prevent. The committed-hash approach catches
  both, so we kept it.
- **The build hard-depends on the generation pipeline; there is no committed fallback.**
  On `main`, `goatdata.c` is gitignored, so a fresh clone must regenerate it. If
  `datafile`/`dat2inc` ever fail to build on a platform, the whole build breaks (master
  carried the committed artifact as a fallback; main traded that for reproducibility).
  Keep the packer tools building on all three platforms.
- **Determinism is proven for the current GitHub Actions toolchains**, not guaranteed for
  every compiler/OS forever. The per-PR gate re-checks it, so a future toolchain change
  that breaks reproducibility fails on the offending platform rather than sneaking in.

## Suggested landing
Small self-contained change on a dedicated branch (e.g. `build-hardening`), separate from
PR-resolution history. Low risk; makes CI trustworthy before the bug-fix work.
