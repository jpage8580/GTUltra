GTUltra v1.2.0
------------------
Release: 29th March 2022
Last Update: 25th September 2022
Editor and C64 GT player enhancements - Jason Page / MultiStyle Labs
Charset & additional palettes: LMan

Initial beta testing:
Thanks to Shogoon, Russell Hoy, Jani Väisänen and Egon Sander

Example songs:
Thanks to MultiStyle Labs (Jammer, Linus, LMan, Mibri and Shogoon)

Special thanks to Kalle Selin, Jani Vaisanen, tlr and theK and everyone who's helped with bug fixes!

V 1.4.0
- Multiple .sng file support (load multiple .sng files and copy/paste between them)
- Linux executable added to the release
- Linux - fixed automatic backup (crashed GTUltra)
- Various fixes for Expanded Order List
- MIDI fixed (hasn’t worked since 1.3.0. Sorry)
- 4SID export fix (GTUltra quits when exporting, due to 6510 BCS being out of range)


V 1.3.0:
- Expanded Order List editing
- True Stereo Panning (15 positions) per instrument (emulation only)
- Definable SID Chip Pan positions (eg. place the 3rd SID in the center!)
- Automatic regular .sng backups (user definable duration between saves)
- Inter-pattern (marked area) looping is now optional
- Optional “Classic” F1-F3 key use (shift-click on record button to enable)
- Improved 6502 code for 9 and 12 channel SIDs for simpler playback
- SID emulation now disabled when seeking to start position (the delay was noticeable and annoying)
- Option to fully disable MIDI within now within .cfg file (set port to 9999)
- Editor settings are now saved in .sng files:
  - FV, PO, RO, NTSC/PAL, SID Type, HR Value, Speed,  # SIDs, Stereo mode



V 1.2.0

FIXES:
Fixed playback / editor lockups when looping sections 
Can now step backwards in orderlist if reaching a transpose or repeat marker
Linux support - many fixes (thank you tlr and theK)

CHANGES:
Window can be resized (dragging bottom corner) - Window width/height ratio is retained (scaled by window height)
Antialias display option added (can be enabled/disabled from .cfg file entry)
F3 in order list now selects correct patterns for patttern view
New palette system, replacing the single gtskins.bin file
(Up to 16 Individual, human readable palette files are loaded at startup from gtpalette folder)
Palette changed to full 8 bit resolution
Palette editor saves individual palette files
Increased palette skinning options to 16
Increased charset size to 2 banks of 256 chars
New charset (thank you to LMan)
New skins (thank you to LMan and Jammer)
SDL2 support
Added MIDI port selection window (Shift-Click on the keyboard icon)
ctrl-Left / ctrl-Right to select previous / next position (emulating clicking the buttons on the transport bar)





GTUltra features:

4 SID Chip (12 channel) support
Transport Bar:
  Change Skin
  Select Octave
  Follow On/Off
  Loop On/Off
  Record (aka Jam Mode) On/Off 
  Play / Pause (play resumes from stopped position)
  Rewind (to start of pattern, to previous pattern, to start of song)
  Fast Forward to next pattern
  JAM Mode SID chip On/Off
  Piano keyboard view
    - Shows playing notes
UI Skinning
Play from any song position or pattern position
  All playing / fast forward / rewind selection syncs all channels to their correct positions, taking into consideration channel-specific tempo channels, pattern lengths, repeat info, etc.
  Playback also emulates audio playing  instruments prior to the selected position.
MIDI Input
  For both jamming and note input.
  Also allows for playing notes whilst editing instruments / tables, etc.
Note number / ARP chord info display
  Show the number number if a single note is held down
  Shows the arp chord values (eg. 00,04,07) if multiple notes are held down
Automatically move to previous / next song position when user scrolls to beginning / end of current pattern
Shows the number of times an instrument has been used within a song (makes it easy to find free instruments)
Shows the total length of the song (automatically calculated). Based on the either the first channel to stop the song, or the last channel to loop.
Table views show each section (making it easier to see where a table section starts and ends)
Full undo (Ctrl-Z)
Save song from anywhere (Ctrl-S)
Info line (shows details on what cursor is currently over. For example, explains the pattern instruction value (1 = portamento up), as well as the corresponding value from the speed table.
Displays SID chip filter information:
  Which channels have filter enabled
  Filter type enabled (low. Band, high)
  Cutoff and Resonance
Save Song / Load Song UI design change to make it less likely to accidentally save over a song instead of loading!
F8 now moves cursor to Table area, instead of Song Name
JAM Mode polyphony (up to 12 SID channels)
Auto-note portamento calculation
  Press a single key to add portamento from cursor position to the next note in the pattern. Automatically calculates and inserts the correct portamento speed table value.
  Palette and charset editor
Unique table views for Wave,Pulse and Filter - No need to remember what hex values do what.
Waveform editor view. Simply click on waveforms/sync/ringmod, etc. to enable or disable.
Window can be resized (dragging bottom corner) - Window width/height ratio is retained (scaled by window height)
Antialias display option added (can be enabled/disabled from .cfg file entry)
Play song from anywhere (even from within a pattern). All channels are synced correctly.
Loop whole patterns or steps within a pattern (looping will sync all channels correctly, taking into consideration pattern lengths and channel-specific tempo changes)
Up to 16 Individual, human readable palette files are loaded at startup from gtpalette folder
Palette editor for setting 8 bit RGB for display components.


