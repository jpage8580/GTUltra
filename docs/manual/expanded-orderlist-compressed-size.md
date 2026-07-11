### 46. Expanded OrderList - Compressed Size

![image](../images/manual/expanded-orderlist-compressed-size-img-1.png)

a. When a .sng is saved or is exported, it is still done so in classic mode. This ensures that the data size is still as small as possible.
b. In Expanded mode, is it possible to create order list data that is too large - the number of entries does not compress to under 256 bytes of data.
c. The current compressed data size is shown at the top of the expanded orderlist view. In the case above, you can see:

    i. Channel 00:  **
    ii. Channel 01: 33
    iii. Channel 02: 38
d. The  **  shows that the size of the channel is invalid  (over 256 bytes). As such,

    it is not possible to swap back to the classic view  -  or the .sng to be
    saved or exported  -  until the data in this channel  has been reduced to a valid
    size.

[Back to index](index.md)
