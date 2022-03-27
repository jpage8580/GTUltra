/*
 * GoatTracker V2.xx pattern splitter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bme_end.h"
#include "gcommon.h"

#define MAX_SPLITS 16

INSTR instr[MAX_INSTR];
char ident[4];
unsigned char ltable[4][MAX_TABLELEN*2];
unsigned char rtable[4][MAX_TABLELEN*2];
unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
unsigned char destsongorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
unsigned char ordermaptbl[MAX_SONGLEN+2];
unsigned char pattern[MAX_PATT][MAX_PATTROWS*4+4];
unsigned char destpattern[MAX_PATT+1][MAX_PATTROWS*4+4];
unsigned char songname[MAX_STR];
unsigned char authorname[MAX_STR];
unsigned char copyrightname[MAX_STR];
int pattlen[MAX_PATT];
int songlen[MAX_SONGS][MAX_CHN];
int destpattlen[MAX_PATT+1];
int destsonglen[MAX_SONGS][MAX_CHN];
int destpattsplits[MAX_PATT+1];
int patternmaptable[MAX_PATT][MAX_SPLITS];
int tables = 0;

int dp;
int highestusedpattern = 0;
int highestusedinstr = 0;
int targetlen = 16;

int wavelen = 0, pulselen = 0, filtlen = 0;

int main(int argc, char **argv);
int loadsong(char *name);
int savesong(char *name);
int processsong(void);
void countpatternlengths(void);
void countdestpatternlengths(void);
void clearsong(void);
int gettablelen(int num);

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Usage: SS2STEREO <source> <destination> [length]\n\n"
           "Splits patterns of the song into smaller patterns with [length] rows,\n"
           "searching for possible duplicates and probably making the song take less\n"
           "memory. For safety reasons source & destination cannot be same, because\n"
           "a splitted song is harder to edit. Always keep the original!\n\n"
           "Default length is 16.\n");
    return 1;
  }
  if (!strcmp(argv[1], argv[2]))
  {
    printf("ERROR: Source and destination are not allowed to be the same.");
    return 1;
  }

  if (argc >= 4)
  {
    sscanf(argv[3], "%u", &targetlen);
    if (targetlen < 1) targetlen = 1;
    if (targetlen > MAX_PATTROWS) targetlen = MAX_PATTROWS;
  }

  if (!loadsong(argv[1]))
  {
    printf("ERROR: Couldn't load source song.");
    return 1;
  }
  if (!processsong())
  {
    return 1;
  }
  if (!savesong(argv[2]))
  {
    printf("ERROR: Couldn't save destination song.");
    return 1;
  }
  return 0;
}

int processsong(void)
{
  int c,d,e;
  int splitsize;
  int songs;
  int dsl;

  dp = 0; // Destination patterns

  for (c = 0; c <= highestusedpattern; c++)
  {
    destpattsplits[c] = 0;
    splitsize = targetlen;
    while (pattlen[c] / splitsize > MAX_SPLITS) splitsize *= 2;
    if (pattlen[c] <= splitsize) splitsize = pattlen[c];

    d = 0;
    while (d < pattlen[c])
    {
      int remain = pattlen[c] - d;
      int splitfound = 0;

      // Check existing patterns for matches
      for (e = 0; e < dp; e++)
      {
        if ((destpattlen[e] <= remain) && (destpattlen[e] >= splitsize) && (!memcmp(&pattern[c][d*4], destpattern[e], destpattlen[e]*4)))
        {
          patternmaptable[c][destpattsplits[c]] = e;
          destpattsplits[c]++;
          d += destpattlen[e];
          splitfound = 1;
          break;
        }
      }
      if (!splitfound)
      {
        // If less than 2 splits left, do in one part
        if (remain < splitsize * 2)
        {
          memcpy(destpattern[dp], &pattern[c][d*4], remain*4);
          destpattern[dp][remain*4] = ENDPATT;
          destpattern[dp][remain*4+1] = 0;
          destpattern[dp][remain*4+2] = 0;
          destpattern[dp][remain*4+3] = 0;
          destpattlen[dp] = remain;
          patternmaptable[c][destpattsplits[c]] = dp;
          destpattsplits[c]++;
          d += remain;
          dp++;
        }
        else
        {
          memcpy(destpattern[dp], &pattern[c][d*4], splitsize*4);
          destpattern[dp][splitsize*4] = ENDPATT;
          destpattern[dp][splitsize*4+1] = 0;
          destpattern[dp][splitsize*4+2] = 0;
          destpattern[dp][splitsize*4+3] = 0;
          destpattlen[dp] = splitsize;
          patternmaptable[c][destpattsplits[c]] = dp;
          destpattsplits[c]++;
          d += splitsize;
          dp++;
        }
      }
      // This should never happen 
      if (destpattsplits[c] >= MAX_SPLITS)
      {
        printf("ERROR: Internal error, too many splits!");
        return 0;
      }
      // This might happen :-) 
      if (dp > MAX_PATT)
      {
        printf("ERROR: 255 patterns exceeded!");
        return 0;
      }
    }
  }

  // Now convert all songs 
  // Determine amount of songs to be processed
  c = 0;
  for (;;)
  {
    if (c == MAX_SONGS) break;
    if ((!songlen[c][0])||
       (!songlen[c][1])||
       (!songlen[c][2])) break;
    c++;
  }
  songs = c;

  for (c = 0; c < songs; c++)
  {
    for (d = 0; d < MAX_CHN; d++)
    {
      dsl = 0;
      for (e = 0; e <= songlen[c][d]+1; e++)
      {
        int pattnum = songorder[c][d][e];

        ordermaptbl[e] = dsl;
        if (e < songlen[c][d])
        {
          if (pattnum < MAX_PATT)
          {
            int f;
            for (f = 0; f < destpattsplits[pattnum]; f++)
            {
              destsongorder[c][d][dsl] = patternmaptable[pattnum][f];
              dsl++;
              if (dsl > MAX_SONGLEN)
              {
                printf("ERROR: Orderlist-length of 254 exceeded!");
                return 0;
              }
            }
          }
          else
          {
            destsongorder[c][d][dsl] = pattnum;
            dsl++;
            if (dsl > MAX_SONGLEN)
            {
              printf("ERROR: Orderlist-length of 254 exceeded!");
              return 0;
            }
          }
        }
        else
        {
          if (pattnum == LOOPSONG)
          {
            destsongorder[c][d][dsl] = pattnum;
          }
          else
          {
            // Map old orderlist position to new 
            destsongorder[c][d][dsl] = ordermaptbl[pattnum];
          }
          dsl++;
        }
      }
    }
  }
  // Everything ok! 
  countdestpatternlengths();
  {
    int destpatttbl = 0, destpatt = 0, destsong = 0;
    int srcpatttbl = 0, srcpatt = 0, srcsong = 0;

    for (c = 0; c < MAX_SONGS; c++)
    {
      if ((songlen[c][0]) && (songlen[c][1]) && (songlen[c][2]))
      {
        for (d = 0; d < MAX_CHN; d++)
        {
          srcsong += songlen[c][d]+1;
          destsong += destsonglen[c][d]+1;
        }
      }
    }
    for (c = 0; c < highestusedpattern; c++)
    {
      srcpatt += pattlen[c]*4 + 4;
      srcpatttbl += 2;
    }
    for (c = 0; c < dp; c++)
    {
      destpatt += destpattlen[c]*4 + 4;
      destpatttbl += 2;
    }
    printf("Processing complete. Results:\n\n"
           "       Songdata Patterns Patt.Tbl Total\n"
           "Before %-8d %-8d %-8d %-8d\n"
           "After  %-8d %-8d %-8d %-8d\n",
           srcsong,srcpatt,srcpatttbl,srcsong+srcpatt+srcpatttbl,
           destsong,destpatt,destpatttbl,destsong+destpatt+destpatttbl);
  }
  return 1;
}

int loadsong(char *name)
{
  int c;

  FILE *srchandle = fopen(name, "rb");
  if (srchandle)
  {
    fread(ident, 4, 1, srchandle);
    if (!memcmp(ident, "GTS2", 4)) tables = 3;
    if (!memcmp(ident, "GTS3", 4)) tables = 4;
    if (!memcmp(ident, "GTS4", 4)) tables = 4;
    if (!memcmp(ident, "GTS5", 4)) tables = 4;

    if (tables)
    {
      int d;
      unsigned char length;
      unsigned char amount;
      int loadbytes;
      clearsong();

      // Read infotexts
      fread(songname, sizeof songname, 1, srchandle);
      fread(authorname, sizeof authorname, 1, srchandle);
      fread(copyrightname, sizeof copyrightname, 1, srchandle);

      // Read songorderlists
      amount = fread8(srchandle);
      for (d = 0; d < amount; d++)
      {
        for (c = 0; c < MAX_CHN; c++)
        {
          length = fread8(srchandle);
          loadbytes = length;
          loadbytes++;
          fread(songorder[d][c], loadbytes, 1, srchandle);
        }
      }
      // Read instruments
      highestusedinstr = fread8(srchandle);
      for (c = 1; c <= highestusedinstr; c++)
      {
        instr[c].ad = fread8(srchandle);
        instr[c].sr = fread8(srchandle);
        instr[c].ptr[WTBL] = fread8(srchandle);
        instr[c].ptr[PTBL] = fread8(srchandle);
        instr[c].ptr[FTBL] = fread8(srchandle);
        instr[c].ptr[STBL] = fread8(srchandle);
        instr[c].vibdelay = fread8(srchandle);
        instr[c].gatetimer = fread8(srchandle);
        instr[c].firstwave = fread8(srchandle);
        fread(&instr[c].name, MAX_INSTRNAMELEN, 1, srchandle);
      }
      // Read tables
      for (c = 0; c < tables; c++)
      {
        loadbytes = fread8(srchandle);
        fread(ltable[c], loadbytes, 1, srchandle);
        fread(rtable[c], loadbytes, 1, srchandle);
      }
      // Read patterns
      amount = fread8(srchandle);
      for (c = 0; c < amount; c++)
      {
        length = fread8(srchandle);
        fread(pattern[c], length*4, 1, srchandle);
      }
      countpatternlengths();
      fclose(srchandle);
      return 1;
    }
    fclose(srchandle);
  }
  return 0;
}

int savesong(char *name)
{
  FILE *handle;
  int c;

  handle = fopen(name, "wb");
  if (handle)
  {
    int d;
    unsigned char length;
    unsigned char amount;
    int writebytes;
    fwrite(ident, 4, 1, handle);

    countdestpatternlengths();

    // Write infotexts
    fwrite(songname, sizeof songname, 1, handle);
    fwrite(authorname, sizeof authorname, 1, handle);
    fwrite(copyrightname, sizeof copyrightname, 1, handle);

    // Determine amount of songs to be saved
    c = 0;
    for (;;)
    {
      if (c == MAX_SONGS) break;
      if ((!destsonglen[c][0])||
         (!destsonglen[c][1])||
         (!destsonglen[c][2])) break;
      c++;
    }
    amount = c;

    fwrite8(handle, amount);
    // Write songorderlists
    for (d = 0; d < amount; d++)
    {
      for (c = 0; c < MAX_CHN; c++)
      {
        length = destsonglen[d][c]+1;
        fwrite8(handle, length);
        writebytes = length;
        writebytes++;
        fwrite(destsongorder[d][c], writebytes, 1, handle);
      }
    }
    // Write instruments
    fwrite8(handle, highestusedinstr);
    for (c = 1; c <= highestusedinstr; c++)
    {
      fwrite8(handle, instr[c].ad);
      fwrite8(handle, instr[c].sr);
      fwrite8(handle, instr[c].ptr[WTBL]);
      fwrite8(handle, instr[c].ptr[PTBL]);
      fwrite8(handle, instr[c].ptr[FTBL]);
      fwrite8(handle, instr[c].ptr[STBL]);
      fwrite8(handle, instr[c].vibdelay);
      fwrite8(handle, instr[c].gatetimer);
      fwrite8(handle, instr[c].firstwave);
      fwrite(&instr[c].name, MAX_INSTRNAMELEN, 1, handle);
    }
    // Write tables
    for (c = 0; c < tables; c++)
    {
      writebytes = gettablelen(c);
      fwrite8(handle, writebytes);
      fwrite(ltable[c], writebytes, 1, handle);
      fwrite(rtable[c], writebytes, 1, handle);
    }
    // Write patterns
    amount = dp;
    fwrite8(handle, amount);
    for (c = 0; c < amount; c++)
    {
      length = destpattlen[c]+1;
      fwrite8(handle, length);
      fwrite(destpattern[c], length*4, 1, handle);
    }

    fclose(handle);
    return 1;
  }
  return 0;
}

void countpatternlengths(void)
{
  int c, d, e;

  highestusedpattern = 0;
  for (c = 0; c < MAX_PATT; c++)
  {
    for (d = 0; d <= MAX_PATTROWS; d++)
    {
      if (pattern[c][d*4] == ENDPATT) break;
    }
    pattlen[c] = d;
  }
  for (e = 0; e < MAX_SONGS; e++)
  {
    for (c = 0; c < MAX_CHN; c++)
    {
      for (d = 0; d < MAX_SONGLEN; d++)
      {
        if (songorder[e][c][d] >= LOOPSONG) break;
        if (songorder[e][c][d] < MAX_PATT)
        {
          if (songorder[e][c][d] > highestusedpattern)
            highestusedpattern = songorder[e][c][d];
        }
      }
      songlen[e][c] = d;
    }
  }
}

void countdestpatternlengths(void)
{
  int c, d, e;

  for (c = 0; c < MAX_PATT; c++)
  {
    for (d = 0; d <= MAX_PATTROWS; d++)
    {
      if (destpattern[c][d*4] == ENDPATT) break;
    }
    destpattlen[c] = d;
  }
  for (e = 0; e < MAX_SONGS; e++)
  {
    for (c = 0; c < MAX_CHN; c++)
    {
      for (d = 0; d < MAX_SONGLEN; d++)
      {
        if (destsongorder[e][c][d] >= LOOPSONG) break;
      }
      destsonglen[e][c] = d;
    }
  }
}

void clearsong(void)
{
  int c;

  for (c = 0; c < MAX_CHN; c++)
  {
    int d;
    for (d = 0; d < MAX_SONGS; d++)
    {
      destsonglen[d][c] = 0;
      destsongorder[d][c][0] = LOOPSONG;

      memset(&songorder[d][c][0], 0, MAX_SONGLEN);
      if (!d)
      {
        songorder[d][c][0] = c;
        songorder[d][c][1] = LOOPSONG;
        songorder[d][c][2] = 0;
      }
      else
      {
        songorder[d][c][0] = LOOPSONG;
        songorder[d][c][1] = 0;
      }
    }
  }
  memset(songname, 0, sizeof songname);
  memset(authorname, 0, sizeof authorname);
  memset(copyrightname, 0, sizeof copyrightname);

  for (c = 0; c < MAX_PATT; c++)
  {
    int d;
    memset(&pattern[c][0], 0, MAX_PATTROWS*4);
    for (d = 0; d < MAX_PATTROWS; d++) pattern[c][d*4] = REST;
    for (d = MAX_PATTROWS; d <= MAX_PATTROWS; d++) pattern[c][d*4] = ENDPATT;
  }
  for (c = 0; c < MAX_INSTR; c++)
  {
    memset(&instr[c], 0, sizeof(INSTR));
  }
  memset(ltable, 0, sizeof ltable);
  memset(rtable, 0, sizeof rtable);
  countpatternlengths();
}


int gettablelen(int num)
{
  int c;

  for (c = MAX_TABLELEN-1; c >= 0; c--)
  {
    if (ltable[num][c] | rtable[num][c]) break;
  }
  return c+1;
}

