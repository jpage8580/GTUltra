### 55. SIDTracker64 Mode

![image](../images/manual/sidtracker64-mode-img-1.png)

a. NOTE: This mode is not compatible with standard GT editor mode. KeyOff does not react in the same way. Songs created in this mode will
    sound different in standard GT mode
b. Enable / Disable using Shift (or CTRL) F12
c. This makes GT work a little more like SIDTracker64.

    i. This is the editor I feel most at home with. All changes here are for my own personal benefit!
    ii. Instruments automatically key OFF when a rest is reached (a blank space)
    iii. Pressing ENTER to enter a key ON command will fill blank notes from the previous note to the cursor position with keyON
    iv. Display is changed to show Key ON as vertical lines instead of +++
d. F2-F3 keys are different:

    i. F2 = Play current pattern from the cursor position
    ii. F3 = Play current pattern from the beginning
e. Multiple KeyOn commands are now also compressed when exporting to .SID file in the same manner as when there is a gap between notes (where the
    note and note length is stored. Now, the keyon and keyon length is stored if
    necessary)

[<<<](mouse-wheel.md) | [index](README.md) | [>>>](drag-and-drop.md)
