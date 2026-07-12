### 52. Pattern order change when exporting to .SID

a. The original GoatTracker will export patterns in order, based on if they are used within a song.
b. For example, if pattern 0 is used, that will be stored in the .SID file first.
c. In 1.5.0 update, the order is based on when the pattern is played within the .sng. For example, if pattern 6 is played first in the song, then it will be stored
    first in the .SID file.
d. Why? This makes it easier to track and reuse memory if a pattern is no longer going to play again, if used in a game or demo with tight memory restrictions

[Back to index](README.md)
