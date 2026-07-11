Mono / Stereo / True Stereo

i. In mono mode (1 SID is selected), this will toggle between mono (Mon) and True Stereo (Pan).
ii. If more than one SID is selected, this will toggle between mono (Mon), stereo (Ste) and True Stereo (Pan).
iii. See  Stereo Panning  for more information on stereo  panning options
iv. See  True Stereo Panning  for more information on True  Stereo panning options

### 6. True Stereo (Editor emulation only)

a. GT Ultra SID emulation has been modified to allow per-channel stereo panning. This can be used to bring new life to SID music, as well as being useful for SID remixes and such like.
b. As an example of what stereo panning can offer, here’s a set of classic Rob Hubbard music, processed in TrueStereo
    https://soundcloud.com/noothermedicine/reconstructed-rob-hubbard-in-truestereo

c. To enable TrueStereo, set the panning type to “Pan”

    ![image](../images/manual/true-stereo-img-1.png)

d. Within each instrument, there is a “Pan” setting, which is comprised of two pan values

    ![image](../images/manual/true-stereo-img-2.png)

    i. 0-7 = Left > Center
    ii. 7-E = Center > Right
    iii. (F does nothing. If 16 (or any even number) of values were available, it would not be possible to have a value that is in the center)
e. Every time an instrument is played (a key ON is performed), a random pan value is set, somewhere between (and including) the two pan values.

    i. Example 1: 07 would pan anywhere from hard-left to center
    ii. Example 2: 0E would pan anywhere from hard-left to hard-right
    iii. Example 3: E0 does the same as 0E (you can put values in any order)
    iv. Example 4: 77 would pan to center only..
f. Stereo SID emulation trivia:

    i. Stereo SID emulation uses the unused 4 bits of the Pulse Width High byte ($D403, $D40A, $D411). Therefore keeping somewhat to the possibility of reality in what a stereo SID could have achieved.
g. Panning information is ignored if True Stereo is not enabled


[Back to index](index.md)
