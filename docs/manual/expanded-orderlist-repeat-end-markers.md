### 47. Expanded OrderList - Repeat / End Markers

![image](../images/manual/expanded-orderlist-repeat-end-markers-img-1.png)

a. In  Classic  view, the RPT marker  at the end of an  order list dictates the position to loop to.
b. In  Expanded  view, this is the same. However, the marker  is shown as the value FF
c. In the Expanded  view, it is possible to have up to  2048 (0x800) entries (based on a compressed order list that uses R0 (repeat 16) multiple times). As such, the loop values are converted to this larger range when a song is expanded.
d. Other than this, the same rules apply:

    i. A value that is lower than the FF position will cause the channel to loop to the position specified
    ii. A value that is higher or equal to the FF position will cause the song to stop.

[<<<](expanded-orderlist-compressed-size.md) | [index](README.md) | [>>>](disable-all-midi.md)
