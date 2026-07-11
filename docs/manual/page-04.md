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
- Optional “Use repeats when compressing from expanded order list view

- By default, the expanded orderlist will insert REPEAT commands when converting back to the original orderlist. This keeps the data small, however it makes it difficult to read. Use this option to disable the use of the REPEAT command
- Optional “Automatically move to next or previous pattern in order list when moving cursor”

- Previously this was always enabled. Now it is possible to disable
- SHIFT+Click on Follow icon to toggle this mode
- Added new .sng file (JasonPage/RType: Amiga2SID.sng)

- This uses the SIDTracker64 mode

## What’s new for 1.4.1?

- Multiple .sng file support (load multiple .sng files and copy/paste between them)
- Mac executable added to the release
- Mac - fixed MIDI support
- Linux executable added to the release
- Linux - fixed automatic backup (crashed GTUltra)
- Various fixes for Expanded Order List
- MIDI (general) fixed (hasn’t worked since 1.3.0. Sorry)
- 4SID export fix (GTUltra quits when exporting, due to 6510 BCS being out of range)

## What’s new for 1.3.0?

- Expanded Order List View
- True Stereo Panning per instrument (emulation only)
- Definable SID Chip Pan positions (eg. place the 3rd SID in the center!)
- Automatic regular .sng backups (user definable duration between saves)
- Inter-pattern looping now optional
- Optional “Classic” F1-F3 key use (shift-click on record button to enable)
- Improved 6502 code for 9 and 12 channel SIDs for simpler playback
- SID emulation now disabled when seeking to start position (the delay was noticeable and annoying)
- Option to fully disable MIDI within now within .cfg file
- Editor settings are now saved in .sng files:

- FV, PO, RO, NTSC/PAL, SID Type, HR Value, Speed,  # SIDs, Stereo mode
