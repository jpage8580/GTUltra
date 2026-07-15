# Side-quests

Parking lot for feature/idea spikes that are adjacent to the main SID-engine migration but
not on its critical path. Capture the idea + enough technical context that it is actionable
later. Move to a real plan/tracker when picked up.

---

## Per-channel panning track (new mode)

**Idea (marcin, 2026-07-15):** add a new mode that gives each channel an extra **panning
track** - a dedicated per-row pan lane alongside the note/instrument data - so pan can be
automated over time per channel, instead of being a fixed per-instrument property.

### Why (what's missing today)
Panning is currently **instrument-level only** and **static**:
- Each instrument carries one pan byte (`instr[].pan`), read as two nibbles
  `panMin = pan>>4`, `panMax = pan&0xf`. Range `0` = hard left, `7` = center, `E` = hard
  right, `F` unused. reSID gain: `fpan = pan/14; L = s*(1-fpan); R = s*fpan`.
- If `panMin != panMax`, a **random** position in that range is chosen per note (the "pan
  spread"); if equal, it is fixed. Default (from `clearinstr()`) is `0x77` = locked center.
- It is applied **once, at note-on** (`gplay.c`, inside `if (cptr->newnote)`), then written to
  the unused hi-nibble of SID reg `0x3` and picked up by the engine.
- It is consumed **only in TrueStereo mode** (`stereoMode==2`). Mono forces pan=7; SID-stereo
  uses per-SID-chip placement (`SID_StereoPanPositions`), not the instrument byte.
- **There is no pan pattern command.** The effect set is a fixed 16 (`0`..`F`, `gcommon.h`
  `CMD_DONOTHING..CMD_SETTEMPO`); none is pan. So today a composer cannot move a voice across
  the stereo field over time.

### Sketch / open questions
- **Shape:** a per-channel pan lane (Renoise-style "pan column") is the natural fit, and dovetails
  with the planned Renoise-style GUI revamp. Alternative/cheaper: a new `CMD_SETPAN` pattern
  command (uses a spare effect slot) - less expressive but far smaller change.
- **Data model:** a per-row pan lane means extending the `.sng` pattern format (new per-row
  field) -> a format-version bump + back-compat load path. A command approach needs no new
  lane, just a new command id + player support.
- **Playback:** pan would need to update **per row/tick**, not just at note-on -> move the pan
  write out of the `newnote` block into the per-frame path; still lands in reg `0x3` hi-nibble.
- **Engine:** the reSID per-voice pan split already exists (`sid.cpp` `voiceLR[]`), so the
  audio side largely works once pan is updated continuously. Check the `ISidEngine` wrapper
  (M1+) exposes/forwards pan cleanly.
- **Modes:** decide how this interacts with the existing `stereoMode` (mono / SID-stereo /
  TrueStereo). Likely a new mode, or an extension of TrueStereo where per-row pan overrides the
  instrument pan.

### Relation to other work
- Depends conceptually on the SID-engine migration settling first ([[sid-emulator-migration]]);
  the pan split lives in the engine being refactored.
- Strong candidate to design **with** the Renoise-style GUI revamp (pan column is a Renoise
  staple).
