# RESOLVED: `-Wpointer-to-int-cast` in `src/asm/` (PR #13)

**Status: RESOLVED** (branch `fix/asm-pointer-cast-exomizer-backport`). Backported the
Exomizer-upstream `%p`/`(void*)` formatting into `src/asm/{expr,parse}.c`; build is now
0 `-Wpointer-to-int-cast`. Supersedes GitHub **PR #13** (@drfiemost) — the PR proposed the
identical change, which turned out to already be present in current Exomizer upstream.

Original tracking notes below, retained for context.

---

Tracking doc for GitHub **PR #13** (@drfiemost, "Fix pointer-to-int-cast warnings").
Surfaced by the warnings inventory
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

## Provenance: `src/asm/` is Exomizer's embedded assembler
Resolved the vendored-policy question by identifying the source. `src/asm/` is the
embedded assembler from **Exomizer** (Magnus Lind, © 2005; official source on Bitbucket
`magli143/exomizer`, current release v3.1.0). Filenames map 1:1: `asmtab.c`←`asm.tab.c`
(yacc), `lexyy.c`←`lex.yy.c` (flex), plus `expr.c`/`parse.c`/`pc.c`/`membuf`/`chnkpool`/
`vec`. So it **genuinely tracks a live upstream** — but our copy is a 2005-era snapshot far
behind current Exomizer. One local edit already exists and is license-marked
(`src/asm/log.c:30`, "Modified for GoatTracker2").

Checked current upstream `expr_dump()`: it **already uses `%p`/`(void*)`** — identical to
what PR #13 proposed. (Upstream also still has the `vNEG` fall-through with no `break`, so
that quirk is upstream behavior, left as-is to stay aligned.)

## What was done (backport, not a hand-written fix)
Backported the upstream `%p`/`(void*)` formatting into the two files, each marked per the
Exomizer license (clause 2, "altered source versions must be plainly marked"), mirroring
`log.c:30`:
- `src/asm/expr.c` `expr_dump()` — 6 `LOG()` lines (9 casts)
- `src/asm/parse.c` `dump_sym_entry()` — 1 `LOG()` line (2 casts)

Verified: pre-patch 9 + 2 = **11** `-Wpointer-to-int-cast`; post-patch **0**; full
`make mac-build` succeeds, binary produced, no new warnings.

## Severity recap
All sites are inside **debug `LOG()` dump helpers**, so the only pre-fix effect was a
truncated pointer value in debug output. Practical severity **LOW**; fix correctness
**HIGH** (canonical, and matches upstream).

## Follow-up (optional, separate effort)
A full re-sync of `src/asm/` from Exomizer v3.1.0 would modernize the whole assembler but
is higher risk (20y of drift, assembler API consumed by GTUltra's build/relocation path,
the local `log.c` edit to re-apply). Deferred — the targeted backport captures this fix.
