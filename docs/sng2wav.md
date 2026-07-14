# sng2wav

`sng2wav` is a small command-line tool that plays a GoatTracker/GTUltra song
(`.sng`) **without opening the editor** and either:

- **renders** it to a stereo WAV file, or
- **scans** it (and whole folders of songs) to show what each tune uses.

It needs no window, no audio device, and no GUI session, so it is handy for
batch jobs, automated tests, and comparing how songs sound on different SID
emulation engines.

The binary is built alongside the other tools (it lands next to `gtultra`, e.g.
`mac/sng2wav`, `linux/sng2wav`, `win32/sng2wav.exe`).

---

## Quick start

```sh
# Render a song to a 30-second WAV
sng2wav mysong.sng out.wav --seconds 30

# Survey one song
sng2wav --scan mysong.sng

# Survey every .sng under a folder (recurses subfolders)
sng2wav --scan examples/
```

Options may appear in any order. The first plain argument is the input
(`.sng` file, or a folder when using `--scan`); the second is the output `.wav`.

---

## Render mode

```
sng2wav <song.sng> <out.wav> [options]
```

Loads the song, plays it, and writes a 16-bit stereo WAV. By default it plays
for 10 seconds. Most C64 tunes loop forever, so the render always stops at the
length you ask for (it never hangs).

### Options

| Option | Meaning | Default |
|---|---|---|
| `--seconds S` | Render length in seconds | `10` |
| `--frames F` | Render an exact number of player frames instead of seconds (for reproducible length) | — |
| `--subsong N` | Which subsong to play | `0` |
| `--model M` | Force the SID chip: `6581` or `8580` | from the song |
| `--stereo K` | `0` = mono, `1` = SID-stereo, `2` = TrueStereo (per-voice panning) | from the song |
| `--ntsc` / `--pal` | Force timing | from the song |
| `--channels N` | Force the SID channel count | auto from the song |
| `--rate HZ` | Output sample rate | `44100` |
| `--backend B` | Emulation engine: `legacy-resid` (integer) or `legacy-residfp` (float) | `legacy-residfp` |
| `--interp I` | `0` = fast, `1` = interpolate/resample | `1` |
| `--interpolate V` | Set engine **and** interpolation at once with the packed value from the app config (see below) | — |
| `--normalize` | Scale the output up so the loudest peak is near maximum | off |
| `--seed K` | Random seed, so repeated renders are byte-for-byte identical | `0` |

### The two engines

- **`legacy-resid`** (integer reSID) — cleaner, lower CPU; strong on the **8580** chip.
- **`legacy-residfp`** (floating-point reSID-FP) — models analog distortion and
  non-linearity; strong on the **6581** chip's warm/dirty character.

To hear the same tune both ways:

```sh
sng2wav song.sng a.wav --backend legacy-resid   --seconds 20 --seed 1
sng2wav song.sng b.wav --backend legacy-residfp --seconds 20 --seed 1
```

### `--interpolate` (the packed config value)

The GTUltra editor stores engine + interpolation as a single number in its
`.cfg` (the `interpolate` key). `--interpolate` mirrors it exactly:

| Value | Meaning | Same as |
|---|---|---|
| `0` | reSID, interpolation off | `--backend legacy-resid --interp 0` |
| `1` | reSID, interpolation on | `--backend legacy-resid --interp 1` |
| `2` | reSID-FP ("distortion") | `--backend legacy-residfp --interp 0` |
| `3` | reSID-FP + interpolation (the app default) | `--backend legacy-residfp --interp 1` |

"distortion" is just the config's name for the floating-point engine.

### Settings come from the song

By default `sng2wav` uses the chip model, timing, speed, stereo mode, and
hard-restart value **stored in the `.sng`**. A command-line flag overrides only
that one setting. (Older songs imported from GoatTracker may not store these —
see [Blank settings](#blank-settings) below — in which case defaults are used
and you may want to pass `--model` / `--stereo` etc. yourself.)

---

## Scan mode

```
sng2wav --scan <song.sng | folder>
```

Prints a table describing each song. Give it a single file, or a folder to walk
the whole tree. Example:

```
┌──────┬────┬──────┬──────┬──────┬──────┬───────┬──────┬─────┬────┬─────┬──────────┬─────────────────────────────────┐
│ SIDs │ ch │ wave │ sync │ ring │ filt │ model │ HR   │ spd │ st │ tim │ FV/PO/RO │ file                            │
├──────┼────┼──────┼──────┼──────┼──────┼───────┼──────┼─────┼────┼─────┼──────────┼─────────────────────────────────┤
│ 2    │ 6  │ 40   │ 1    │ 5    │ 41   │ 8580  │ FF00 │ x2  │ 2  │ PAL │ 1/0/0    │ examples/RType_Amiga2SID.sng    │
├──────┼────┼──────┼──────┼──────┼──────┼───────┼──────┼─────┼────┼─────┼──────────┼─────────────────────────────────┤
│ 2    │ 6  │ 33   │ 24   │ 24   │ 15   │       │      │     │    │     │          │ examples/In-clouds.sng          │
└──────┴────┴──────┴──────┴──────┴──────┴───────┴──────┴─────┴────┴─────┴──────────┴─────────────────────────────────┘
```

### Column meanings

**Identity**

- **SIDs** — how many SID chips the tune uses (1 = 3 channels, 2 = 6 channels).
- **ch** — channel count.

**Effect usage** (`wave`, `sync`, `ring`, `filt`)

These tell you **how much a tune's instruments are set up to use each feature.**
They are *not* seconds, note counts, or how often something is heard — they are
counts of entries in the song's instrument tables (think of it as "how many
places in the instrument definitions do this").

- **wave** — how many instrument steps set an oscillator waveform
  (triangle / sawtooth / pulse / noise).
- **sync** — of those, how many turn on **hard sync** (a harsh, tearing timbre
  made by resetting one oscillator with another).
- **ring** — of those, how many turn on **ring modulation** (a metallic,
  bell-like timbre made by multiplying two oscillators).
- **filt** — how many filter-table steps are used (filter sweeps / movement).

Higher numbers mean the tune leans on that feature more. For example, a tune
showing `sync 24 / ring 24` is a great choice for testing hard-sync and
ring-modulation; one showing `sync 0 / ring 0` uses neither. A tune could
program an effect once but use it constantly (or vice-versa), so treat these as
a *strong hint*, not an exact measurement.

**Stored settings** (`model`, `HR`, `spd`, `st`, `tim`, `FV/PO/RO`)

These come from the song's settings block:

- **model** — SID chip the tune was made for (`6581` or `8580`).
- **HR** — hard-restart value (ADSR handling), in hex.
- **spd** — speed multiplier (`x1`, `x2`, ...; `x0` means half speed).
- **st** — stereo mode: `0` mono, `1` SID-stereo, `2` TrueStereo.
- **tim** — `PAL` or `NTSC`.
- **FV/PO/RO** — three editor flags: fine-vibrato, pulse-optimize, realtime-optimize.

<a name="blank-settings"></a>
### Blank settings cells

If the stored-settings columns are **blank**, the `.sng` does not contain a
settings block (this is common for songs imported from older GoatTracker
versions). In that case the tune's real model / speed / stereo / hard-restart
are unknown, and `sng2wav` falls back to defaults when rendering. Opening the
song in the GTUltra editor and saving it again writes a settings block, after
which the values appear.

A row marked `[LOAD FAILED]` means the file could not be read as a song.

---

## Reproducibility

The SID emulation uses a little randomness (filter noise, etc.). Passing the
same `--seed` makes repeated renders **byte-for-byte identical**, which is what
you want for automated comparisons and reference files. Different seeds produce
tiny, inaudible differences.

`sng2wav` runs the *same* synthesis and mixing code as the GTUltra editor, so it
reproduces the editor's WAV **export** sample-for-sample under matched conditions
(engine/model/settings/rate, master volume = 1.0, and matching RNG/`residdelay`).
It is not a bit-perfect capture of an arbitrary live editor session. The exact
shared code and every possible divergence are documented in
`notes/new-emu/gtultra-vs-sng2wav.md`.

---

## Running without a screen (CI / servers)

`sng2wav` never opens a window or audio device, so it runs fine in headless
environments. On **Linux** it works from any shell (use dummy SDL drivers if
needed):

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy sng2wav --scan examples/
```

On **macOS**, run it from a normal Terminal while logged into the desktop.
(GoatTracker's command-line tools that link SDL can stall on a startup dialog
when launched from a session with no desktop, e.g. some CI runners; for
automated jobs prefer Linux.)

---

## Examples

```sh
# Reference render at the song's own settings
sng2wav tune.sng ref.wav --seconds 30 --seed 1

# Force 6581 vs 8580 to compare chip character
sng2wav tune.sng c6581.wav --model 6581 --seconds 20 --seed 1
sng2wav tune.sng c8580.wav --model 8580 --seconds 20 --seed 1

# Mono vs TrueStereo
sng2wav tune.sng mono.wav   --stereo 0 --seconds 20 --seed 1
sng2wav tune.sng stereo.wav --stereo 2 --seconds 20 --seed 1

# Find the sync / ring-mod heavy tunes in a library
sng2wav --scan songs/

# Exact-length, deterministic render (good for test fixtures)
sng2wav tune.sng fixture.wav --frames 500 --seed 1
```

---

## Exit codes

- `0` — success.
- `1` — bad arguments, or the song could not be loaded (render mode).

Usage/error text is printed to standard error; the scan table and render
progress are printed to standard output / progress log.
