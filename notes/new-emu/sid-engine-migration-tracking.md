# SID engine migration - step tracking

Sequential checklist for `sid-engine-migration-plan.md` (the authoritative plan; read it
for the "why" and detail). Work top to bottom: each milestone is independently shippable
and gates the next. Tick `[x]` when done; keep notes inline.

Legend: `[ ]` todo, `[~]` in progress, `[x]` done, `[-]` skipped (note why).
Rule: do not start a milestone until the previous one's "verify" box is `[x]`.

---

## Status snapshot (2026-07-14) - read this first

All work so far is on branch `new-emulators` (not pushed, not yet a PR).

Done + committed:
- **gt2reloc headless-SDL fix** (`SDL_MAIN_HANDLED`, non-Windows) - commit `2b5ad0a`.
  Root cause + detail: `headless-render-scope.md` §3a. Verified: builds, packs identically,
  object exports real `_main`. NOT yet validated on a true no-GUI runner (mac CI) - see §0.
- **sng2wav headless `.sng`->WAV renderer + recursive `--scan` box table** - commit `1647356`.
  Also in that commit: `src/cli_common.c` (shared CLI boilerplate, extracted from gt2reloc,
  which was verified byte-identical after), `sound_init_offline()` in `gsound.c`, and a
  `songfilename` buffer fix (`MAX_FILENAME`->`MAX_PATHNAME`: truncation + latent overflow;
  also benefits gt2reloc). Manual: `docs/sng2wav.md`. Builds clean on mac; render + scan work.
- **Plan / tracker / notes** - commits `038633b`, `6869846`. The old analysis docs were folded
  into `sid-engine-migration-plan.md` (M4/M5 + appendix) and then removed (kept in git history).
- **Golden-corpus candidates surveyed** - `golden-corpus-candidates.md` (pick still pending).

Next up (finish M0): pick the golden corpus, capture golden WAVs, stand up `tests/unit/` +
`make test`. Then M1 (`ISidEngine` wrapper).

**CI policy for this branch (decided 2026-07-14):** keep firing CI **on push** to
`new-emulators` for the whole migration - the 3 workflows carry `push: [new-emulators]` +
`workflow_dispatch:` (added alongside the existing `pull_request:[main]`). **Defer the PR to
`main` until the very end.** At that final PR: strip the branch-specific `push` trigger (and any
other branch-only scaffolding) so `main`'s workflows stay PR-only, and squash-merge per the
CLAUDE.md rules. (`workflow_dispatch` only becomes dispatchable once it reaches `main`.)

Loose ends to handle at PR time:
- **CHANGELOG** `[Unreleased]` entries not yet written: gt2reloc headless-SDL fix, songfilename
  overflow/truncation fix, sng2wav tool. (Project rule: CHANGELOG updated when PR is ready.)
- **Remove the `push: [new-emulators]` CI trigger** from the 3 workflows before the final merge.
- **Determinism check DONE (2026-07-14)** - `sng2wav` is fully reproducible on the legacy
  engines, so the M1 bit-identical strategy is sound. (Ad-hoc `/tmp` run, not yet a committed
  test.) Results (10s renders):

  | Check | Result |
  |---|---|
  | same tune + same `--seed`, twice | IDENTICAL (`md5 1b34c171…`) |
  | no `--seed` given, twice | IDENTICAL (legacy engines have no internal noise) |
  | In-clouds (heavy sync/ring), same seed | IDENTICAL |
  | `legacy-resid` vs `legacy-residfp` | DIFFER (`e89a84ea…` vs `1b34c171…`) - engine switch works |

  Cross-check: default backend output = explicit `--backend legacy-residfp` (`1b34c171…`),
  confirming the documented default.
- **What `--seed` actually controls (traced 2026-07-14, Linux/Lima aarch64).** The
  "no internal noise" line above is imprecise: there IS a `rand()`-driven path, it is just
  dormant for the current fixtures. Mechanically:
  - **Two `rand()` sites exist**, and the SID **backend is irrelevant** to both (resid and
    resid-fp share the `gsid.cpp` write loop and the `gplay.c` player):
    1. `gplay.c:567/569` pan-spread - fires only when `stereoMode == 2` (TrueStereo) **AND** a
       played instrument has a pan **range** (hi nibble ≠ lo nibble of `instr.pan`). The picked
       pan lands in reg `0x3` hi-nibble (`gplay.c:1239`); resid reads it (`wave.cpp:87
       pan=(pw_hi>>4)&0xf`) and per-voice L/R-mixes it (`sid.cpp:683-692`/`787-796`). Live under
       **both** `--interp 0` and `--interp 1`.
    2. `gsid.cpp:293` badline delay - gated by `residdelay`, which is hardcoded `0` in
       `cli_common.c:64` and has **no `sng2wav` flag** -> the branch never fires (the `rand()`
       call runs but its result is unused). Dormant in `sng2wav`.
  - **=> `--seed` changes `sng2wav` output ONLY for a TrueStereo tune with pan-range instruments.**
    For mono / SID-stereo, or TrueStereo with fixed pans, `--seed` is a genuine no-op.
  - `Stereo_Pendejo.sng`: all 63 instruments `pan=0x77` (7..7, no range) -> seed inert for it
    (confirmed by dumping loaded `instr[].pan`).
  - **glibc caveat:** `srand(0)` is remapped to `srand(1)`, so **seeds 0 and 1 are identical by
    design** - never use that pair to test seed sensitivity. Verified with a forced pan range:
    seed 2 vs 42 DIFFER (interp 0 and 1); seed 42 twice IDENTICAL (deterministic); native
    fixed-pan fixture seed 2 vs 42 IDENTICAL.
  - **Golden-corpus implication:** a `--seed`-pinned golden is only meaningful for a **TrueStereo
    tune with pan ranges**; pick at least one such tune to exercise the seed path, else note that
    `--seed` is inert for the chosen set. (Repo untouched; traced via env-gated debug patches in
    the Lima VM copy, since reverted.)
- **Editor-vs-sng2wav parity** is analysed precisely in `gtultra-vs-sng2wav.md` (same DSP code;
  divergences = master volume, RNG state, device rate). Empirical parity proof not yet run.
- mac/Windows CI: the `timeout-minutes:1` headless guard can be reconsidered once the
  SDL_MAIN_HANDLED fix is proven on those runners.

---

## M0 - Test scaffolding (no engine change)
- [x] Prereq: `SDL_MAIN_HANDLED` in `gt2reloc.c` for mac headless-SDL hang (commit `2b5ad0a`;
      only gt2reloc links SDL among CLI tools; true no-GUI/CI validation still pending). §0/§3a.
- [x] Headless render mode: `sng2wav` (commit `1647356`) drives `sid_fillbuffer`/
      `ExportSIDToPCMFile` via `sound_init_offline`, no editor/SDL device. Song-derived settings
      + CLI overrides; recursive `--scan` box table. Scope: `headless-render-scope.md`; manual: `docs/sng2wav.md`.
- [ ] Stand up `tests/unit/` + single-header harness (Unity or greatest) + `make test`.
- [~] Pick golden corpus `.sng` set (6581/8580 x sync/ring-mod/filter-sweep; mono/SID-stereo/TrueStereo).
      Candidates surveyed + shortlist recommended: `golden-corpus-candidates.md` (maintainer pick pending).
- [ ] Capture golden WAVs/metrics for the CURRENT build, both legacy backends -> `tests/fixtures/golden-*`.
- [~] Verify: headless render reproducible - `--seed` byte-identical CONFIRMED (2026-07-14,
      md5 match; see Status snapshot). Still TODO: `make test` green (empty/placeholder ok);
      goldens committed.

## M1 - Introduce `ISidEngine` wrapper (NOT gated; must be bit-identical)
- [ ] Create `src/engines/isid_engine.h` (neutral enums; NO engine headers included).
- [ ] Create `src/engines/engine_factory.{h,cpp}` (backend enum -> `ISidEngine*`; single-backend enforce).
- [ ] Wrap old integer engine: `engine_resid_old.cpp` (`debug()` returns `debugCount`).
- [ ] Wrap old float engine: `engine_residfp_old.cpp` (applies global `filterparams` in setup).
- [ ] Refactor `gsid.cpp`: drive `ISidEngine* chip[4]`; collapse the `if(sid)/if(sidfp)` ladder to a loop.
- [ ] Repoint `sid_debug()` -> `chip[0]->debug()`.
- [ ] Add `engines/*.o` + `engines/*.d` to `makefile.common` (objects, `-include`, `clean`).
- [ ] Unit test #1 (pan mapping) + #6 (M1 bit-identical hash) pass.
- [ ] Verify: golden WAVs BIT-IDENTICAL to M0 capture on both legacy backends.

## M2 - reSID-vice, mono, compile-gated
- [ ] Copy `_upstream/resid/*` -> `src/resid-vice/` (keep `.cc`; leave `_upstream` pristine).
- [ ] Run `samp2src.pl` once; commit 8 `wave*.h` fragments. (only `wave.o` built; no per-table .o)
- [ ] Commit configured `src/resid-vice/siddefs.h` (NEW_8580_FILTER=1, inlining/branch-hints on).
- [ ] Commit `src/resid-vice/resid-config.h`.
- [ ] Add compile gate `WITH_RESID_VICE` in `makefile.common` (mirror SANITIZE); gated object list.
- [ ] Confirm: NO `-I` flag added; DROP `version.o`; classic `filter.cc` excluded.
- [ ] Wrapper `engine_resid_vice.cpp` (`#ifdef WITH_RESID_VICE`): mono clock -> duplicate L into R.
- [ ] Factory: reSID-vice branch under `#ifdef`; fallback+warn if requested but not compiled.
- [ ] Verify: `make linux-build WITH_RESID_VICE=1` links & plays; default build unchanged (byte-identical).
- [ ] Verify: `make ... WITH_RESID_VICE=1 SANITIZE=1` clean; 8580 tunes improve vs golden (tolerance).

## M3 - reSID-vice, TrueStereo (two-filter patch)
- [ ] Patch `wave.h/.cc`: add `pan` + `getPan()`; `writePW_HI` sets pan; `reset()` zeros it.
- [ ] Patch `sid.h`: `Filter filter[2]`, `ExternalFilter extfilt[2]`; fan model/enable/mask/reset/input to both.
- [ ] Patch `SID::clock()` (both overloads): per-voice pan split with EXACT legacy formula into filter[0]/[1].
- [ ] Add stereo `output(short*L,short*R)` + split-buffer `clock(...,bufferHalfSize,...)`; 2nd resample ring.
- [ ] Point `engine_resid_vice::clockStereo` at the new split clock.
- [ ] Save `patches/resid-vice-truestereo.patch` (diff vs `_upstream/resid`).
- [ ] Unit tests #2 (split-buffer), #3 (center==mono tol), #4 (sync/ring-mod), #5 (factory fallback) pass.
- [ ] Verify: pan preserved; hard-sync + ring-mod tune correct; feature-preservation vs golden.

## M4 - libresidfp, mono, compile-gated
- [ ] Copy `_upstream/libresidfp` -> `src/residfp-lib/`; commit configured `siddefs-fp.h`.
- [ ] Add gate `WITH_RESIDFP_LIB` + object list (`src/*.cpp` + `resample/*.cpp`); no generator/Perl.
- [ ] Wrapper `engine_residfp_lib.cpp`; factory branch under `#ifdef`.
- [ ] Verify: gated build links & plays; 6581 tunes vs golden (tolerance).

## M5 - libresidfp, TrueStereo (float rewrite)
- [ ] Pan storage; dual `Filter`/`ExternalFilter`/`Resampler`; pan-aware `Filter::clock(...,gL,gR)`.
- [ ] Stereo clock loop -> split buffer; `patches/libresidfp-truestereo.patch`.
- [ ] Verify: pan + sync/ring-mod preserved vs golden.

## M6 - Config / UI
- [ ] Append `.cfg` key `sidbackend` (append-only; loader tolerates absence; `interpolate>>1` default).
- [ ] Append libresidfp tuning keys (curve/range/combinedwaveforms/old6581caps) w/ stock defaults.
- [ ] Status-area "experimental/unstable" indicator for new backends.
- [ ] Verify: old `.cfg` files load unchanged; unknown/uncompiled backend falls back gracefully.

## M7 - Deprecate old engines (only after M3 [+M5] pass full corpus)
- [ ] Remove `src/resid/`, `src/resid-fp/`, their objects, legacy `filterparams` application, old wrappers.
- [ ] Flip gate defaults / maturity ladder Phase C->D; update CHANGELOG `[Unreleased]` + docs.
- [ ] Verify: full corpus green; `gsid.h` C API unchanged (nothing above `gsid.cpp` touched).

---

## Cross-cutting (keep ticking throughout)
- [ ] CI: one dedicated gated Linux job (`WITH_RESID_VICE=1 SANITIZE=1` + `make test` + golden compare).
- [ ] CI: default Linux/macOS/Windows jobs remain unchanged (build + smoke + goatdata determinism).
- [ ] Determinism: new-engine goldens are tolerance/statistical (filter8580new injects `rand()` noise), NOT hashes.
- [ ] Header firewall held: only `engines/*.cpp` include `resid*/…` headers.
- [ ] Never edit `_upstream/*`; patches are the record of engine edits.

## Maturity ladder (rollout state)
- [ ] Phase A: `WITH_RESID_VICE` OFF by default; opt-in build + gated CI only.
- [ ] Phase B: compiled in by default, runtime OFF (`sidbackend` legacy default); labeled experimental.
- [ ] Phase C: reSID-vice default for 8580; old engines still selectable.
- [ ] Phase D: old engines removed (= M7).
</content>
