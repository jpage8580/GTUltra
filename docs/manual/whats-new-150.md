## What’s new for 1.5.0?

- Drag / Drop to load files
- No need to use the F10 menu option
- Bug fixes and stability improvements
- Improved jamming mode
- KeyOff performed automatically on key release
- SIDTracker64 mode
- KeyOff automatically applied on an empty pattern row
- Enter Key - create Key On entries from previous note to cursor position to extend note length prior to Key Off
- F2-F3 keys act different:
  - F2 = Play current pattern from the cursor position
  - F3 = Play current pattern from the beginning
  - Shift (or CTRL) F12 to toggle this mode on or off
- Export to WAV
  - Shift-F11 to display this panel
  - Normalization on/off option
  - Will export to 16 bit, stereo, 44.1KHz .WAV file
  - Uses current mute settings (allows you to export individual channels)
  - Change: CTRL+A to select whole pattern row
- SHIFT+A to decrease note values (used to also work with CTRL+A)
- Change: 3 different auto-advance modes (SHIFT-Z) when entering data, depending on where cursor is within pattern area
  - Notes only (advance if you enter a note, rest, key on or key off. NOT if you are entering hex data)
  - All (Advance when entering notes or values)
  - OFF (no auto advance)
- Mouse Wheel
  - Use the mouse wheel to scroll through panels, depending on where your flashing editor cursor is currently located
- New: Improved failing merge of 2 .sng files (if running out of instruments or patterns)
  - Original GT gave no warning of a failed merge.
- Optional Debug mode in .cfg file
  - When enabled, memory checks are performed to check for memory leaks
- Optional “Use repeats when compressing from expanded order list view”
  - By default, the expanded orderlist will insert REPEAT commands when converting back to the original orderlist. This keeps the data small, however it makes it difficult to read. Use this option to disable the use of the REPEAT command
- Optional “Automatically move to next or previous pattern in order list when moving cursor”
  - Previously this was always enabled. Now it is possible to disable
- SHIFT+Click on Follow icon to toggle this mode
- Added new .sng file (JasonPage/RType: Amiga2SID.sng)
  - This uses the SIDTracker64 mode

[Back to index](index.md)
