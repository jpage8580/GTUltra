GTUltra v1.1.6
------------------
Release: 29th March 2022
Last Update: 30th April 2022
Editor and C64 GT player enhancements - Jason Page / MultiStyle Labs

Charset: LMan & Skins

Initial beta testing:
Thanks to Shogoon, Russell Hoy, Jani Väisänen and Egon Sander

Example songs:
Thanks to Jammer, Linus, LMan, Mibri and Shogoon



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
  Also allows for listening playing notes whilst editing instruments / tables, etc.
Note number / ARP chord info display
  Show the number number if a single note is held down
  Shows the arp chord values (eg. 00,04,07) if multiple notes are held down
Automatically move to previous / next song position when user scrolls to beginning / end of current pattern
Shows the number of times an instrument has been used within a song (makes it easy to find free instruments)
Shows the overall length of the song (automatically calculated).
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
Unique table views for Wave,Pulse and Filter

