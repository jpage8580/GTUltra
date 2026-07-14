# Golden-reference corpus candidates

Candidate `.sng` tunes for the SID-migration golden/A-B test corpus (M0 in
`sid-engine-migration-tracking.md`; open decision #4 in `sid-engine-migration-plan.md`).
Goal: cover **6581 and 8580 x hard-sync / ring-mod / filter sweeps x mono / SID-stereo /
TrueStereo**, so an engine swap that breaks any of those shows up.

Data below is from `sng2wav --scan examples/` (see `docs/sng2wav.md` for column meaning).
`wave/sync/ring/filt` = counts of instrument-table steps programming that feature (a usage
hint, not playback time). "settings" = whether the `.sng` carries a 0x1f settings block; when
absent, model/HR/speed/stereo are NOT stored and must be passed as flags to render faithfully.

## Notable tunes (from the scan)

| Tune | Author | model | wave | sync | ring | filt | settings | note |
|---|---|---|---|---|---|---|---|---|
| `tests/fixtures/Stereo_Pendejo.sng` | (fixture) | 8580 | 71 | 2 | 0 | 66 | yes (**mono/st=0**, x2, HR FF00) | current fixture; filter-heavy, no ring-mod. Was mis-flagged `st=2`; author-confirmed mono, corrected 2026-07-15 |
| `examples/JasonPage/RType_Amiga2SID.sng` | Jason Page | 8580 | 40 | 1 | 5 | 41 | yes (TrueStereo, x2, HR FF00) | 8580 + ring-mod, settings stored |
| `examples/Mibri/$3Mibri - Psychic Carpet Bombing [3x speed 8580].sng` | Mibri | 8580 | 41 | 3 | 2 | 101 | yes (1 SID / 3ch, x3) | only single-SID candidate; heavy filter |
| `examples/Shogoon/$3In-clouds.sng` | Shogoon | - | 33 | 24 | 24 | 15 | no | **the hard-sync + ring-mod stress test** |
| `examples/Shogoon/$3jammar-akhbar.sng` | Shogoon | - | 96 | 9 | 8 | 149 | no | heaviest overall (most filter); strong all-round |
| `examples/Linus/$3sys64738-days.sng` | Linus | - | 110 | 1 | 10 | 100 | no | most waveform steps; heavy filter/ring, little sync |

## Recommended minimal corpus

Covers the matrix with the fewest tunes:

1. **Stereo_Pendejo** (fixture) - 8580, **mono (st=0)**, filter-heavy. Has settings. *Keep* -
   now fills the mono slot (was mis-flagged TrueStereo; author-confirmed mono, fixed 2026-07-15).
2. **RType_Amiga2SID** (Jason Page) - 8580, `st=2`, ring-mod. Has settings. **TrueStereo flag
   UNVERIFIED** - after Pendejo's bogus flag, do not assume it actually pans; check L!=R (or make
   a purpose-built panned tune) before relying on it as the TrueStereo case. See caveat below.
3. **$3In-clouds** (Shogoon) - 6581, the hard-sync + ring-mod stress test. *No settings -> render with explicit flags.*
4. **$3jammar-akhbar** (Shogoon) *or* **$3sys64738-days** (Linus) - 6581, filter-sweep + all-round complexity. *No settings -> flags.*
5. **$3Mibri - Psychic Carpet Bombing** (Mibri) - 8580, single-SID (3ch/1 SID). Has settings. Adds the 1-SID case (all others are 2-SID/6ch).

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
- **No verified TrueStereo tune in the corpus.** Pendejo's `st=2` turned out bogus (now mono);
  `RType`'s `st=2` is unverified. Before M3 (the pan-split feature test) either confirm a tune
  actually pans (L!=R across frames) or author a small purpose-built TrueStereo tune with a
  couple of hard-panned voices.
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
