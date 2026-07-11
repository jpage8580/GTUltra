### 40. Automatic .sng backup

a. There’s always a chance that GTUltra may crash (or a power cut, or you accidentally save over your song..etc.). GTUltra automatically saves a copy of
    the current song at regular intervals in a folder named “gtbackup”, which is
    located within the same folder as the GTUltra.cfg file.
b. If GTUltra crashes, you can use this .sng file to hopefully recover your work with minimal loss of time and effort.
c. The gtbackup folder is automatically created when GTUltra is run.
d. You can specify how often a song is backed up either by changing the value in the GTUltra.cfg file or via the -b command line option
e. Setting the backup time to 0 will disable auto backups
f. Default is to save the backup every 30 seconds.
g. A copy of the sng is automatically saved every n seconds to gtbackup folder.
h. Saving only happens when a song is not being played to ensure that nothing interrupts playback.
i. Saving only happens if there has been a change to the song since the last save.
j. Each backup filename contains the time and date.

[Back to index](index.md)
