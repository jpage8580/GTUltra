### 35. Detailed Table Editing: Filter Table

![image](../images/manual/filter-table-editing-img-1.png)

a. Clicking on the FILT.TBL title decodes the filter table data and displays it in a more user-friendly way
    i. Click on the title again to show the original table view
b. For each row, you can select functionality by clicking either

    ![image](../images/manual/filter-table-editing-img-2.png)

    i. C: “CUTOFF ” Set Filter Cutoff (0-$FF)
    ii. M: “FLT MOD”  Modify Filter Cutoff (left column =  time, right column=speed)
    iii. F: “ FLT SET”  Filter Info (Resonance, Channel On/Off,  Filter Type)
    iv. J: Jump (1-$FF or 0 to Stop)

        ![image](../images/manual/filter-table-editing-img-3.png)

c. Filter Info
    i. The 3 circles represent the channels which will be affected by the filter
        1. Red = filter is active on the channel
        2. Grey = filter is not active on the channel
        3. Click on the circles to enable / disable filters
    ii. The filter type (lowpass, bandpass, high pass) is shown as 3 icons
        1. Green = filter type is enabled
        2. Grey = filter type is disabled
        3. Click on the icons to enable / disable filter types
d. When modifying filter cutoff, the right column (speed can be changed from + to - by clicking on the + or - symbol)

    ![image](../images/manual/filter-table-editing-img-4.png)
e. Remember that the combination of CTRL-C / CTRL-V can be used to quickly copy & paste single entries

[<<<](pulse-table-editing.md) | [index](README.md) | [>>>](waveform-editor.md)
