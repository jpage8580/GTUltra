# Handover / tracking: PR #13 — `-Wpointer-to-int-cast` in `src/asm/`

Tracking doc for GitHub **PR #13** (@drfiemost, "Fix pointer-to-int-cast warnings",
**OPEN**, not merged). Surfaced by the warnings inventory
([warnings-tracking.md](warnings-tracking.md)). Related: [known-bugs.md](known-bugs.md).

## What PR #13 does
Fixes all **11** `-Wpointer-to-int-cast` warnings, in **`src/asm/expr.c` (9)** and
**`src/asm/parse.c` (2)**. It replaces `(u32)ptr` casts in `LOG()` debug-dump statements
with `%p` + `(void*)ptr`:
```c
- LOG(level, ("expr 0x%08X symref %s\n", (u32)e, e->type.symref));
+ LOG(level, ("expr %p symref %s\n",      (void*)e, e->type.symref));
```
Sites: `expr_dump()` (expr.c) and `dump_sym_entry()` (parse.c).

## Why it warns, and how serious it is
On 64-bit, `(u32)e` **truncates** a 64-bit pointer to 32 bits — the warning is legitimate.
But every site is inside a **debug `LOG()` dump helper**, so the only real-world effect is
a wrong/truncated pointer value in debug log output. No program logic depends on the cast.
- Practical severity: **LOW** (debug-only, cosmetic).
- Fix correctness: **HIGH** and trivial (`%p`/`(void*)` is the canonical form).

## Blocker: vendored-code policy (decision needed before adopting)
`src/asm/` is listed in `CLAUDE.md` under **"Do not modify vendored"**. PR #13 patches it
directly, so it cannot simply be merged without reconciling that rule:
- **If `src/asm/` is an owned fork** (not tracking an upstream) → adopt PR #13 as-is,
  credit @drfiemost, mark the pointer-to-int-cast row in `warnings-tracking.md` fixed.
- **If it genuinely tracks upstream** → send the change upstream instead of merging here;
  keep the vendored rule intact and leave these 11 warnings as accepted noise.

This doc does **not** adopt PR #13 — it records it pending that call.

## Steps to adopt (once the policy is decided)
1. Branch from `main`.
2. Apply PR #13 (fetch @drfiemost's branch, or apply the two-file diff).
3. Rebuild; confirm the 11 `-Wpointer-to-int-cast` warnings are gone and no new warnings.
4. CHANGELOG `### Fixed`: note the fix, "Thanks @drfiemost (PR #13)".
5. Update [warnings-tracking.md](warnings-tracking.md) (11 → 0) and resolve this doc.

## Definition of done
0 `-Wpointer-to-int-cast` in the build; PR #13 closable; the `src/asm/` vendored-policy
question explicitly resolved (owned-fork vs upstream).
