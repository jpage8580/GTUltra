# Handover: `-Wunused-result` cleanup sweep

Pick-up doc. Surfaced while fixing issue #76 (see [known-bugs.md](known-bugs.md),
[handover-issue-76.md](handover-issue-76.md)). Do on a fresh branch cut from `main`.

## Goal
Silence the 83 `-Wunused-result` warnings without regressing file I/O behaviour.

## Background
glibc marks `fread`/`fgets`/`fwrite`/`chdir`/`getcwd`/`write` with
`__attribute__((warn_unused_result))` under `_FORTIFY_SOURCE` (active at `-O2`+, so at our
`-O3`). Ignoring their return values warns. Counted on Linux (gcc 15/16):

| function | count |
|----------|-------|
| `fread`  | 56    |
| `fgets`  | 11    |
| `write`  | 8     |
| `chdir`  | 6     |
| `getcwd` | 2     |
| **total**| **83**|

By file: `gsong.c` 36, `gsound.c` 9, `ss2stereo.c` 9, `gfile.c` 7, `ins2snd2.c` 5,
`gconsole.c` 4, `io.c` 3, `gt2reloc.c` 2, `gpaletteeditor.c` 1.

## Why this was deferred from #76
Most of the count is `fread`/`write` inside **save/load paths** (`gsong.c`). Wrapping them
wrong can silently corrupt or mask real I/O errors. It is a cross-cutting janitorial sweep,
not a one-line fix, so it does not belong in a security-fix PR.

## Approach (mechanical, do NOT bulk cast-to-void)
- **A `(void)` cast does NOT silence `warn_unused_result`** in GCC — it still warns. So you
  must actually consume the value.
- Prefer **meaningful handling** where cheap: check `fread`/`fgets` element/char counts and
  bail or log on short read; check `chdir`/`getcwd`/`write` for `-1` and handle.
- Where a genuine ignore is intended, use a deliberate idiom that compiles clean, e.g.
  `if (fread(...) != n) { /* handle/short-read */ }` or a small checked wrapper.
- Do this **file by file**, rebuild after each, and diff behaviour. `gsong.c` (song
  (de)serialisation) is the highest-risk file — pair with a golden/round-trip test
  (testing-strategy ladder #2) before touching it if possible.

## Steps
1. Branch from `main`.
2. Reproduce the list: `make linux-rebuild 2>&1 | grep -n unused-result` (or build under
   FORTIFY on any platform). 83 expected.
3. Fix per file, rebuild, confirm the count drops; run `tests/integration/smoke.sh`.
4. For `gsong.c`, ideally add a `.sng` load->save->load byte-stability check first.
5. Update CHANGELOG `### Fixed`.

## Definition of done
- 0 `-Wunused-result` warnings on Linux build; smoke tests green; no I/O regression
  (round-trip stable).
