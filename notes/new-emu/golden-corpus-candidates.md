# Golden-reference corpus candidates

Candidate `.sng` tunes for the SID-migration golden/A-B test corpus (M0 in
`sid-engine-migration-tracking.md`; open decision #4 in `sid-engine-migration-plan.md`).
Goal: cover **6581 and 8580 x hard-sync / ring-mod / filter sweeps**, so an engine swap that
breaks any of those shows up.

**Stereo is a render MODE, not a tune property.** The `examples/` tunes are mono compositions
(some use 2 SIDs / 6 channels, but that is voice count, not authored stereo). Mono /
SID-stereo / TrueStereo are playback transforms selected at render time (`--stereo 0/1/2`), so
we cover the three stereo *mixing paths* by rendering the **same** tunes under each `--stereo`
flag, NOT by choosing "stereo tunes." The `st` value stored in a `.sng` is just a saved
default and has been wrong in practice (see caveats). Genuine TrueStereo *panning* (per-voice
pan split, M3) needs real pan data, which none of these tunes have (all instruments `0x77`
center), so that one case needs a purpose-built tune.

Data below is from `sng2wav --scan examples/` (see `docs/sng2wav.md` for column meaning).
`wave/sync/ring/filt` = counts of instrument-table steps programming that feature (a usage
hint, not playback time). "settings" = whether the `.sng` carries a 0x1f settings block; when
absent, model/HR/speed/stereo are NOT stored and must be passed as flags to render faithfully.

## Notable tunes (from the scan)

| Tune | Author | model | wave | sync | ring | filt | settings | note |
|---|---|---|---|---|---|---|---|---|
| `tests/fixtures/Stereo_Pendejo.sng` | (fixture) | 8580 | 71 | 2 | 0 | 66 | yes (x2, HR FF00) | current fixture; **mono composition**; filter-heavy, no ring-mod. Was mis-flagged `st=2`; corrected to `st=0` 2026-07-15 |
| `examples/JasonPage/RType_Amiga2SID.sng` | Jason Page | 8580 | 40 | 1 | 5 | 41 | yes (x2, HR FF00) | 8580 + ring-mod; **mono composition** (2 SID/6ch = voice count, not stereo; all pans `0x77`) |
| `examples/Mibri/$3Mibri - Psychic Carpet Bombing [3x speed 8580].sng` | Mibri | 8580 | 41 | 3 | 2 | 101 | yes (1 SID / 3ch, x3) | only single-SID candidate; heavy filter |
| `examples/Shogoon/$3In-clouds.sng` | Shogoon | - | 33 | 24 | 24 | 15 | no | **the hard-sync + ring-mod stress test** |
| `examples/Shogoon/$3jammar-akhbar.sng` | Shogoon | - | 96 | 9 | 8 | 149 | no | heaviest overall (most filter); strong all-round |
| `examples/Linus/$3sys64738-days.sng` | Linus | - | 110 | 1 | 10 | 100 | no | most waveform steps; heavy filter/ring, little sync |

## Recommended minimal corpus

Pick tunes to cover the **model x DSP-feature x SID-count** axes; cover the **stereo axis
separately** by rendering (some of) these under each `--stereo` mode (see below).

Tunes:
1. **Stereo_Pendejo** (fixture) - 8580, filter-heavy, no ring-mod. Has settings. *Keep.*
2. **RType_Amiga2SID** (Jason Page) - 8580 + ring-mod. Has settings. (Mono composition; 2 SID/6ch.)
3. **$3In-clouds** (Shogoon) - 6581, the hard-sync + ring-mod stress test.
4. **$3jammar-akhbar** (Shogoon) *or* **$3sys64738-days** (Linus) - 6581, filter-sweep + all-round complexity.
5. **$3Mibri - Psychic Carpet Bombing** (Mibri) - 8580, single-SID (3ch/1 SID). Adds the 1-SID case (others are 2-SID/6ch).

(Post-RE, most tunes now carry a settings block; re-scan to confirm before pinning render flags - see caveats.)

Stereo-mode coverage (the mixing paths, on any of the above):
- `--stereo 0` (mono), `--stereo 1` (SID-stereo, pans the 2 chips L/R), `--stereo 2` (TrueStereo
  per-voice mix) - render the same tune under each to exercise all three mix paths.
- **TrueStereo *panning* (per-voice separation) is NOT covered** by these tunes (all pans `0x77`
  center -> `--stereo 2` collapses to mono). Add a **purpose-built tune** with a few off-center /
  hard-panned instrument pans to validate the M3 pan-split. Until then, `--stereo 2` here only
  tests the code path, not the panning math.

Superlatives (for quick reference):
- Most complex overall: **$3jammar-akhbar** (Shogoon).
- Most waveform steps: **$3sys64738-days** (Linus).
- Most hard-sync + ring-mod: **$3In-clouds** (Shogoon).

## Caveats

- **STALE (2026-07-15):** the `settings` / model columns in the tables above predate a pass that
  reverse-engineered the `examples/*.sng` from their `.sid` originals to carry Ultra `.sng`
  settings blocks. Many tunes previously marked "no settings" now have them. **Re-run
  `sng2wav --scan examples/` and refresh these tables before finalizing the corpus.** (Left as-is
  here on purpose - the example `.sng` edits are owned/handled separately.)
- **All these tunes are mono compositions; stereo is only a render mode.** Both former "stereo"
  candidates were bogus: Pendejo `st=2` -> corrected to mono; RType is a mono tune too (its 2
  SIDs are voice count, not authored stereo; all pans `0x77`, and its L!=R only appears when you
  force `--stereo 1`, which is the SID-stereo *mode* panning the chips). Verified by dumping
  instrument pans (RType + Pendejo both 63x `0x77`) and by L/R measurement.
  => For M3 (per-voice pan-split feature test) author a small **purpose-built TrueStereo tune**
  with a few off-center / hard-panned instrument pans. Stereo *mode* paths are covered by
  re-rendering any tune under `--stereo 0/1/2`.
- Most (pre-RE) `examples/` tunes had **no settings block**, so they render at defaults (6581 /
  stereo1 / half-speed / HR fallback) unless you pass `--model`, `--stereo`, `--pal/--ntsc`, and a
  speed as flags. Re-saving a tune in the GTUltra editor writes the block.
- These are third-party compositions shipped under `examples/`. Confirm licensing/attribution
  before committing derived golden WAVs to the repo (or store hashes + metrics instead).
- Regenerate this survey any time with: `sng2wav --scan examples/`.
- **`--seed` is inert unless a tune is TrueStereo AND has instruments with a pan range**
  (hi nibble ≠ lo nibble of the instrument pan byte). `Stereo_Pendejo` uses fixed pans
  (`0x77`), so its goldens are seed-independent. If the corpus should exercise the seeded
  pan-spread path at all, include one pan-range TrueStereo tune; otherwise capture goldens
  without relying on `--seed`. Full trace: `sid-engine-migration-tracking.md`
  ("What `--seed` actually controls").
