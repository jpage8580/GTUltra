# GoatTracker v2.77 → GTUltra Port Analysis

Status: draft hand-off for next agent  
Scope: identify what should be back-ported from GoatTracker v2.77 (mono + stereo) into the GTUltra `new-emulators` branch.  

## 1. Context

- `GTUltra 1.50` (`readme.txt`) is based on **GoatTracker v2.76 Stereo**.
- `_upstream/GoatTracker_2.77_Stereo` is the v2.77 stereo release (2021-05-13).
- `_upstream/GoatTracker_2.77` is the v2.77 mono release (2025-01-20 re-release on SourceForge).
- The `new-emulators` branch was just rebased over `main` (now at `v1.5.8`), so the `main`-line `greloc`/`gt2reloc` fixes from `af0a4ec` are already present.
- This analysis is about **editor/packer/CLI bugfixes and small features** in GoatTracker 2.77, not the SID emulator migration (that is covered in `notes/new-emulator-versions.md` and `notes/emulator-migration-proposal.md`).

## 2. Methodology

1. Read both `readme.txt` changelogs for v2.77 release notes.
2. `diff -rq --ignore-all-space src/ _upstream/GoatTracker_2.77_Stereo/src/` to get the high-level file map.
3. `git show af0a4ec -- src/greloc.c src/gt2reloc.c` to confirm the already-rebased `main` fixes.
4. Manual inspection of the key files identified by the v2.77 changelogs:
   - `src/gt2stereo.c` — config loader / startup path
   - `src/greloc.c` / `src/gt2reloc.c` — packer/relocator
   - `src/gsid.cpp` / `src/gplay.c` / `src/gsound.c` — SID register write order
   - `src/player.s` / `src/altplayer.s` — player assembly
5. Cross-checked `sidorder`/`altsidorder` arrays and `SIDWAVEDELAY` handling.

## 3. Executive Summary

Only three production changes are listed in the v2.77 changelogs:

| # | Source | Changelog text | Status in GTUltra | Action |
|---|---|---|---|---|
| 1 | `GoatTracker_2.77_Stereo` | *Fixed bug in packed songs with no instrument vibrato and only calculated speeds.* | **Already present** in `greloc.c` and `player.s` | Verify with a test case; no code port expected |
| 2 | `GoatTracker_2.77_Stereo` | *Fixed failure to start up with missing config file parameters.* | **Partially missing** — explicit string initialisation before `fopen()` is not done | **Port** the two-liner init |
| 3 | `GoatTracker_2.77` (mono) | *Fixed editor write order to match stereo version.* | **Not applicable** — GTUltra is a stereo fork and already uses the stereo write order and `SIDWAVEDELAY` | None |

In addition, there is one GTUltra-specific cleanup item: a commented-out `debug_0.s` dump in `src/greloc.c:1842` that should be removed.

## 4. Detailed Findings

### 4.1 Config-loader startup fix (v2.77 Stereo, MISSING)

**Changelog:** *Fixed failure to start up with missing config file parameters.*

**What changed in v2.77 Stereo:**

```c
// src/gt2stereo.c (2.77 Stereo, around line 119-121)
  specialnotenames[0] = 0;
  scalatuningfilepath[0] = 0;
  configfile = fopen(filename, "rt");
```

The two string buffers are explicitly NUL-terminated before the config file is opened, so if the file is missing or truncated, `scalatuningfilepath` and `specialnotenames` are guaranteed empty instead of left as whatever happened to be in the buffer.

**GTUltra state:**

- `specialnotenames` is declared `src/gt2stereo.c:156`.
- `scalatuningfilepath` is declared `src/gt2stereo.c:157`.
- The config path is built at `src/gt2stereo.c:241-242`:
  ```c
  strcpy(appFileName, getenv("HOME"));
  strcat(appFileName, "/.goattrk/gtultra.cfg");
  ```
  *(Note: `getenv("HOME")` is not checked for `NULL`; that is a separate, pre-existing crash hardening issue and not part of the v2.77 changelog, but it is worth doing at the same time.)*
- `configfile = fopen(appFileName, "rt");` is at `src/gt2stereo.c:292`.
- The first `getstringparam(configfile, specialnotenames);` is at `src/gt2stereo.c:340`.
- The second `getstringparam(configfile, scalatuningfilepath);` is at `src/gt2stereo.c:341`.

There is **no explicit pre-`fopen` initialisation** of these strings in GTUltra. They are global `char[]` arrays, so they are zero-initialised by the linker in the normal case, but the v2.77 fix makes this explicit and also protects against the unlikely case that the compiler/linker zero-init assumption is violated or the data is in a section that is not zeroed. The `getstringparam` implementation in GTUltra (`src/gt2stereo.c:2841`) is actually *more* robust than the v2.77 one because it handles leading comment lines and seeks back when a value is missing, so the v2.77 fix is mostly the explicit two-liner.

**Recommended port:**

Insert before `configfile = fopen(appFileName, "rt");` at `src/gt2stereo.c:292`:

```c
specialnotenames[0] = 0;
scalatuningfilepath[0] = 0;
```

Optionally, also guard `getenv("HOME")`:

```c
const char* home = getenv("HOME");
if (home)
{
    strcpy(appFileName, home);
    strcat(appFileName, "/.goattrk/gtultra.cfg");
}
else
{
    appFileName[0] = 0;
}
```

This is a separate hardening issue; the v2.77 changelog does not mention it, but it is the same startup path.

### 4.2 Packed-song vibrato / speed fix (v2.77 Stereo, LIKELY ALREADY PRESENT)

**Changelog:** *Fixed bug in packed songs with no instrument vibrato and only calculated speeds.*

**Where the code lives:**

- `src/greloc.c` — `calcspeedtest()` and the `nocalculatedspeed` / `nonormalspeed` / `nozerospeed` / `noinsvib` flag logic.
- `src/player.s` / `src/altplayer.s` — the `NOCALCULATEDSPEED`, `NONORMALSPEED`, `NOZEROSPEED`, `NOINSTRVIB` conditionals.

**Evidence:**

- `calcspeedtest()` in GTUltra (`src/greloc.c:2849`) is byte-identical to the v2.77 Stereo version (`_upstream/GoatTracker_2.77_Stereo/src/greloc.c:2109`):

  ```c
  void calcspeedtest(unsigned char pos)
  {
    if (!pos)
    {
      nozerospeed = 0;
      return;
    }
    if (ltable[STBL][pos - 1] >= 0x80) nocalculatedspeed = 0;
    else nonormalspeed = 0;
  }
  ```

- The instrument scanning block in `greloc.c` that calls `calcspeedtest(instr[c].ptr[d])` only when `d == STBL` is present in both GTUltra (`src/greloc.c:866`) and v2.77 Stereo (`_upstream/GoatTracker_2.77_Stereo/src/greloc.c:350`).

- The `if (instr[c].ptr[STBL]) { novib = 0; noinsvib = 0; }` logic is present in both GTUltra (`src/greloc.c:1277`) and v2.77 Stereo (`_upstream/GoatTracker_2.77_Stereo/src/greloc.c:766`).

- `src/player.s` is byte-identical between GTUltra and `_upstream/GoatTracker_2.77_Stereo/src/player.s`.

- `src/altplayer.s` is byte-identical as well.

**Conclusion:** GTUltra already contains the v2.77 packed-song logic. No code port is needed.

**One subtle point for the next agent to verify:**

The `noinsvib` flag is set to 0 whenever `instr[c].ptr[STBL]` is non-zero, regardless of whether `instr[c].vibdelay` is zero. If an instrument has a speed-table pointer but `vibdelay == 0` (i.e. it has no actual instrument vibrato), `noinsvib` still becomes 0. This is the same logic in v2.77 Stereo, so either both are correct or both share the same subtle bug. A good regression test would be a `.sng` that uses pattern vibrato/calculated speed commands but has no instrument vibrato, and pack it with `gt2reloc`.

### 4.3 Editor write order fix (v2.77 Mono, NOT APPLICABLE)

**Changelog:** *Fixed editor write order to match stereo version.*

**Why no action is needed:**

- GTUltra is a stereo fork. The `sidorder` and `altsidorder` arrays in `src/gsid.cpp:22-32` are identical to those in `_upstream/GoatTracker_2.77_Stereo/src/gsid.cpp` and `_upstream/GoatTracker_2.77/src/gsid.cpp`.

  ```c
  unsigned char sidorder[] =
  { 0x15,0x16,0x18,0x17,
    0x05,0x06,0x02,0x03,0x00,0x01,0x04,
    0x0c,0x0d,0x09,0x0a,0x07,0x08,0x0b,
    0x13,0x14,0x10,0x11,0x0e,0x0f,0x12 };
  ```

- The `gplay.c` per-channel register write order in GTUltra (`src/gplay.c:1236-1241`) is the same stereo order:

  ```c
  gt->sidreg[sidIndex][0x0 + 7 * (c % 3)] = cptr->freq & 0xff;
  gt->sidreg[sidIndex][0x1 + 7 * (c % 3)] = cptr->freq >> 8;
  gt->sidreg[sidIndex][0x2 + 7 * (c % 3)] = cptr->pulse & 0xfe;
  gt->sidreg[sidIndex][0x3 + 7 * (c % 3)] = (cptr->pulse >> 8) | (cptr->pan << 4);
  gt->sidreg[sidIndex][0x4 + 7 * (c % 3)] = cptr->wave & cptr->gate & cptr->requestKeyOff;
  ```

  This matches the v2.77 Stereo order (`src/gplay.c` around line 1115): frequency, pulse, waveform.

- The `SIDWAVEDELAY` extra delay for waveform register writes (`o == 4 || o == 11 || o == 18`) is present in GTUltra's `sid_fillbuffer` (`src/gsid.cpp:303-328`) and in `src/gsound.c:434-457`, with the same 4-SID expansion. The v2.77 Mono `gsid.cpp` does not have this waveform delay, but the v2.77 Stereo source does; GTUltra follows the stereo source.

**Conclusion:** GTUltra already uses the stereo write order and the waveform delay. No port from the Mono v2.77 changelog is needed.

### 4.4 `greloc.c` debug dump leftover (CLEANUP, not a 2.77 feature)

`src/greloc.c:1842-1853` contains a commented-out block:

```c
/*
sprintf(textbuffer, "debug_0.s");

FILE *handle = fopen(textbuffer, "wt");
fwrite(membuf_get(&src), membuf_memlen(&src), 1, handle);
fclose(handle);

if (assemble(&src, &dest))
{
    exit(1);
}
*/
```

The active version of this dump was removed in `main` `v1.5.8` (`git show af0a4ec`). The commented skeleton is dead code and should be deleted. The `new-emulators` branch has the same `greloc.c` content as `main` after the rebase except for this leftover, so this is a small, safe cleanup.

**Recommended action:** remove the commented `debug_0.s` block from `src/greloc.c:1842-1853`.

## 5. Other v2.76 features already present in GTUltra

The `GoatTracker_2.77_Stereo/readme.txt` changelog goes back to v2.59. GTUltra is based on v2.76 Stereo, so the v2.76 features are already in GTUltra. Quick verification:

| v2.76 Stereo feature | Where in GTUltra | Status |
|---|---|---|
| `/Q` equal divisions per octave | `src/gt2stereo.c:118`, `src/gt2stereo.c:527` | Present |
| `/J` different note names | `src/gt2stereo.c:531`, `src/gt2stereo.c:663` | Present |
| `/Y` Scala tuning file path | `src/gt2stereo.c:535`, `src/gt2stereo.c:652` | Present |
| `bigwindow` / `/w` window sizes | `src/gt2stereo.c:121`, `src/gt2stereo.c:539`, `src/gconsole.c` | Present |
| Isomorphic key layout / `KEY_JANKO` | `src/gpattern.c:106`, `src/gpattern.c:2146` | Present |
| `SHIFT+F9` mono/stereo switch | `src/gt2stereo.c` / `stereoMode` | Present |

No extra v2.76 feature port is needed.

## 6. File-level diff summary (high-level)

`diff -rq --ignore-all-space src/ _upstream/GoatTracker_2.77_Stereo/src/` produces 121 lines. The differences fall into three buckets:

### 6.1 Files only in GTUltra (GTUltra additions, do NOT port)

- `src/RtMidi.cpp`, `src/RtMidi.h` — MIDI I/O
- `src/gmidi.cpp`, `src/gmidi.h`, `src/gmidiselect.c`, `src/gmidiselect.h` — MIDI UI
- `src/gchareditor.c`, `src/gchareditor.h` — character editor
- `src/ginfo.c`, `src/ginfo.h` — info display
- `src/gpaletteeditor.c`, `src/gpaletteeditor.h` — palette editor
- `src/gundo.c`, `src/gundo.h` — undo system
- `src/gtabledisplay.c`, `src/gtabledisplay.h` — table display
- `src/gfkeys.c`, `src/gfkeys.h` — function key config
- `src/player3.s`, `src/player9.s`, `src/player12.s`, `src/altplayer3.s`, `src/altplayer9.s`, `src/altplayer12.s` — multi-SID player variants
- `src/version.h` — GTUltra version
- `src/makefile.mac` — macOS build
- `src/gtpalettes/`, `src/gtskins.bin`, `src/default.gtp`, `src/*default.gtp` — palette/skin assets

### 6.2 Files only in v2.77 Stereo upstream (not needed in GTUltra)

- `_upstream/GoatTracker_2.77_Stereo/src/goatdata.c` — generated packed player data; GTUltra has its own generated `goatdata.c` (per `AGENTS.md`, never edit directly)
- `_upstream/GoatTracker_2.77_Stereo/src/gt2stereo.dat` — compiled datafile
- `_upstream/GoatTracker_2.77_Stereo/src/bme/dat2inc.exe`, `datafile.exe` — Windows build tools

### 6.3 Files with content differences

These are the files that differ but, on inspection, the differences are GTUltra-specific extensions rather than missing v2.77 fixes:

- `src/gt2stereo.c` — main UI; has GTUltra additions (MIDI, palettes, undo, etc.) plus the v2.77 config init gap identified above.
- `src/greloc.c` — packer/relocator; has GTUltra multi-SID support plus the dead `debug_0.s` block.
- `src/gt2reloc.c` — does not exist in v2.77; it is GTUltra's CLI relocator.
- `src/gsid.cpp` / `src/gsid.h` — GTUltra's 4-SID stereo reSID interface.
- `src/gplay.c` / `src/gplay.h` — GTUltra 4-SID playroutine.
- `src/gsound.c` / `src/gsound.h` — GTUltra mixer/sound path.
- `src/gsong.c` / `src/gsong.h` — GTUltra song handling.
- `src/gpattern.c`, `src/gorder.c`, `src/gtable.c`, `src/ginstr.c` — GTUltra UI extensions.
- `src/gconsole.c`, `src/gdisplay.c`, `src/gfile.c`, `src/ghelp.c` — GTUltra UI/file changes.
- `src/resid/*` and `src/resid-fp/*` — GTUltra TrueStereo patches to the old reSID/reSID-FP engines (not part of v2.77).
- `src/asm/*` — vendored Exomizer assembler; GTUltra `main` has backported `%p` fixes (`v1.5.8`); do not touch `src/asm/` per `AGENTS.md`.
- `src/bme/*` — BME library; differences are GTUltra-specific (SDL2, macOS, etc.)

## 7. Recommendations

### 7.1 Must port (v2.77 Stereo)

1. **`src/gt2stereo.c` — pre-open string init**
   - Add `specialnotenames[0] = 0;` and `scalatuningfilepath[0] = 0;` before `configfile = fopen(appFileName, "rt");` at line 292.
   - Optional but strongly encouraged: guard `getenv("HOME")` for `NULL` when building the config path at lines 241-242.

### 7.2 Verify but no code expected

2. **Packer `noinsvib` / `nocalculatedspeed` interaction**
   - Create a regression `.sng` with no instrument vibrato and only pattern-level calculated speed commands.
   - Pack it with `gt2reloc` and confirm the output is identical between the current `new-emulators` build and a v2.77 Stereo build.
   - If the test passes, no port is needed.

### 7.3 Cleanup

3. **Remove dead code in `src/greloc.c`**
   - Delete the commented `debug_0.s` block at lines 1842-1853.

### 7.4 Do not port

4. **Mono v2.77 editor write order fix** — GTUltra already uses the stereo order.
5. **Any v2.76 Stereo features** — already present in GTUltra.
6. **Any new engine or player code** — this is out of scope; use the emulator migration notes.

## 8. Appendices

### 8.1 v2.77 Stereo changelog

```text
v2.77     - Fixed bug in packed songs with no instrument vibrato and only
            calculated speeds.
          - Fixed failure to start up with missing config file parameters.
```

### 8.2 v2.77 Mono changelog

```text
v2.77     - Fixed editor write order to match stereo version.
```

### 8.3 Key line references

- `src/gt2stereo.c:156` — `specialnotenames[186]` declaration
- `src/gt2stereo.c:157` — `scalatuningfilepath[MAX_PATHNAME]` declaration
- `src/gt2stereo.c:241-242` — `getenv("HOME")` + `/.goattrk/gtultra.cfg` path
- `src/gt2stereo.c:292` — `configfile = fopen(appFileName, "rt");`
- `src/gt2stereo.c:340` — `getstringparam(configfile, specialnotenames);`
- `src/gt2stereo.c:341` — `getstringparam(configfile, scalatuningfilepath);`
- `src/gt2stereo.c:2841` — `getstringparam()` implementation
- `src/greloc.c:2849` — `calcspeedtest()`
- `src/greloc.c:1277` — `if (instr[c].ptr[STBL]) { novib = 0; noinsvib = 0; }`
- `src/greloc.c:1842` — commented `debug_0.s` block
- `src/gsid.cpp:22-32` — `sidorder` / `altsidorder`
- `src/gsid.cpp:303-328` — `SIDWAVEDELAY` waveform write delay
- `src/gplay.c:1236-1241` — per-channel register write order

