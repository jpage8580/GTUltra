# Handover: bootstrap unit tests (sprout the palette-name helper)

Pick-up doc. Rationale in [testing-strategy.md](testing-strategy.md) (ladder #3/#4).
Motivated by issue #76 ([handover-issue-76.md](handover-issue-76.md)): the fix currently
has only an integration/ASan gate, no unit test. Fresh branch from `main`.

## Goal
Stand up the project's first unit-test harness and add a regression unit test for the
`setPaletteName` allocation contract (NUL-terminated, equal to input).

## Why a sprout is needed (not just "link the .o")
`setPaletteName` itself is trivial (touches only `char *paletteNames[16]` +
`strdup`/`free`), but `src/gpaletteeditor.o` has **~58 external non-libc dependencies**
(`drawbox`, `fliptoscreen`, `getColor`, `fileselector`, `getkey`, `mouse*`, editor
globals, ...). Linking it into a test would require stubbing all of them — fragile.

**Sprout instead** (testing-strategy ladder #4): extract the alloc/copy into a tiny pure
helper with no editor/SDL deps, e.g.

```c
/* palette-name storage: allocate a NUL-terminated copy into slot `index`. */
char *palette_name_dup(char **names, int index, const char *name);
```

Have `setPaletteName` call it. The helper links standalone into a unit test.

## Steps
1. Branch from `main`.
2. Add a single-header harness (**Unity** or **greatest** — minimal deps, matches project
   ethos; avoid GoogleTest/Criterion). Create `tests/unit/` + a `make test` target; wire
   into the three build workflows.
3. Sprout `palette_name_dup` (or similar) out of `setPaletteName`; keep `setPaletteName`
   as a thin caller so production behaviour is unchanged.
4. Unit test: call the helper with a known string; assert result is non-NULL, `strcmp`
   equal, and `strlen`+1 bytes allocated (NUL present). Build the unit test under
   `-fsanitize=address` so the old `malloc(strlen)` bug would fail it directly.
5. (Optional, ladder #2) add golden/round-trip tests for the CLI tools while the harness
   exists.
6. Update CHANGELOG `### Added`.

## Definition of done
- `make test` runs a green unit suite (incl. the palette-name regression) locally and in
  CI on at least Linux; test also passes under ASan.
