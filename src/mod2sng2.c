/*
 * MOD (4chn or 6chn) -> GoatTracker stereo SNG converter.
 * Dedicated to T.M.R!
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bme_end.h"

typedef struct
{
  unsigned char note;
  unsigned char instr;
  unsigned char command;
  unsigned char data;
} NOTE;

typedef struct
{
  unsigned char note;
  unsigned char command;
  unsigned char data;
} GOATNOTE;


unsigned short periodtable[16][12] =
{
        {6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624},
        {6800, 6416, 6056, 5720, 5392, 5096, 4808, 4536, 4280, 4040, 3816, 3600},
        {6752, 6368, 6016, 5672, 5360, 5056, 4776, 4504, 4256, 4016, 3792, 3576},
        {6704, 6328, 5968, 5632, 5320, 5024, 4736, 4472, 4224, 3984, 3760, 3552},
        {6656, 6280, 5928, 5592, 5280, 4984, 4704, 4440, 4192, 3960, 3736, 3528},
        {6608, 6232, 5888, 5552, 5240, 4952, 4672, 4408, 4160, 3928, 3704, 3496},
        {6560, 6192, 5840, 5512, 5208, 4912, 4640, 4376, 4128, 3896, 3680, 3472},
        {6512, 6144, 5800, 5472, 5168, 4880, 4600, 4344, 4104, 3872, 3656, 3448},
        {7256, 6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4032, 3840},
        {7200, 6800, 6416, 6056, 5720, 5400, 5088, 4808, 4536, 4280, 4040, 3816},
        {7152, 6752, 6368, 6016, 5672, 5360, 5056, 4776, 4504, 4256, 4016, 3792},
        {7096, 6704, 6328, 5968, 5632, 5320, 5024, 4736, 4472, 4224, 3984, 3760},
        {7048, 6656, 6280, 5928, 5592, 5280, 4984, 4704, 4440, 4192, 3952, 3736},
        {7000, 6608, 6232, 5888, 5552, 5240, 4952, 4672, 4408, 4160, 3928, 3704},
        {6944, 6560, 6192, 5840, 5512, 5208, 4912, 4640, 4376, 4128, 3896, 3680},
        {6896, 6512, 6144, 5800, 5472, 5168, 4880, 4600, 4344, 4104, 3872, 3656}
};

unsigned char modheader[1084];
unsigned char modpatterns[64*64*6*4];
NOTE modnotes[64*64*6];
GOATNOTE goatnotes[208][65];
unsigned char orderlist[6][256];
GOATNOTE tempnotes[65];

char ident[] = {'G', 'T', 'S', '!'};

char zeroarray[32] = {0};

int maxpatt = 0;
int orderlen;
int transpose = 0;
int goatchan = 0;
int goatpatt = 1;
int channels = 4;

int main(int argc, char **argv)
{
  FILE *in, *out;
  int c, d;
  unsigned char *srcptr;
  NOTE *destptr;

  if (argc < 3)
  {
    printf("Usage: mod2sng2 <mod> <sng> [halfstep transpose, default 0]\n");
    return 1;
  }

  in = fopen(argv[1], "rb");
  if (!in)
  {
    printf("Source open error.\n");
    return 1;
  }
  out = fopen(argv[2], "wb");
  if (!out)
  {
    printf("Destination open error.\n");
    return 1;
  }

  if (argc > 3)
  {
    sscanf(argv[3], "%d", &transpose);
  }

  printf("Load mod header\n");
  for (c = 0; c < 1084; c++)
  {
    modheader[c] = fread8(in);
  }
  if (modheader[1080] == '6') channels = 6;

  orderlen = modheader[950];
  for (c = 0; c < 128; c++)
  {
    if (modheader[952+c] > maxpatt) maxpatt = modheader[952+c];
  }
  maxpatt++;
  printf("Read in patterns\n");
  for (c = 0; c < maxpatt * channels * 4 * 64; c++)
  {
    modpatterns[c] = fread8(in);
  }
  fclose(in);

  // Convert patterns into easier-to-read format

  printf("Convert patterns into easy to read format\n");
  destptr = modnotes;
  srcptr = modpatterns;
  for (c = 0; c < maxpatt * channels * 64; c++)
  {
    // Note: FT2 saves the 13th bit of period into 5th bit of
    // samplenumber, and when loading it ofcourse cannot read
    // the period back correctly! We don't use the 13th bit!

    unsigned short period = ((srcptr[0] & 0x0f) << 8) | srcptr[1];
    unsigned char note = 0, instrument, command;
    if (period)
    {
            int findnote;
            int offset = 0x7fffffff;

            for (findnote = 0; findnote < 96; findnote++)
            {
                    if (abs(period - (periodtable[0][findnote % 12] >> (findnote / 12))) < offset)
                    {
                            note = findnote + 1;
                            offset = abs(period - (periodtable[0][findnote % 12] >> (findnote / 12)));
                    }
            }
    }
    instrument = (srcptr[0] & 0xf0) | ((srcptr[2] & 0xf0) >> 4);
    command = srcptr[2] & 0x0f;
    destptr->note = note;
    destptr->instr = instrument;
    destptr->command = command;
    destptr->data = srcptr[3];
    srcptr += 4;
    destptr++;
  }

  // Make empty pattern
  for (c = 0; c < 65; c++)
  {
    goatnotes[0][c].note = 0x5f;
    goatnotes[0][c].command = 0;
    goatnotes[0][c].data = 0;
  }
  goatnotes[0][64].note = 0xff;

  printf("Convert patterns into goatpatterns\n");
  // Convert patterns into goatpatterns, and create orderlists
  for (c = 0; c < channels; c++)
  {
    for (d = 0; d < orderlen; d++)
    {
      int patt = modheader[952+d];
      int e = 0;
      int o = patt*(64*channels)+c;
      int breakflag = 0;
      unsigned char prevgoatdata = 0;

      memset(tempnotes, 0, sizeof tempnotes);
      for (;;)
      {
        unsigned char goatcommand = 0;
        unsigned char goatdata = 0;

        if (modnotes[o+e*4].note)
        {
          tempnotes[e].note = modnotes[o+e*4].note - 1 + transpose;
          if (tempnotes[e].note > 0x5d) tempnotes[e].note = 0x5d;
        }
        else tempnotes[e].note = 0x5f;
        switch(modnotes[o+e*4].command)
        {
          case 0:
          goatcommand = modnotes[o+e*4].command;
          goatdata = modnotes[o+e*4].data;
          break;

          case 1:
          case 2:
          case 3:
          case 4:
          goatcommand = modnotes[o+e*4].command;
          if (modnotes[o+e*4].data) goatdata = modnotes[o+e*4].data;
          else goatdata = prevgoatdata;
          prevgoatdata = goatdata;
          break;

          case 0xf:
          if (modnotes[o+e*4].data < 0x20) // Ticktempo
          {
            goatcommand = 7;
            goatdata = modnotes[o+e*4].data;
          }
          break;

          case 0xc:
          // Convert zero volume to keyoff
          if ((!modnotes[o+e*4].data) && (tempnotes[e].note == 0x5f))
            tempnotes[e].note = 0x5e;
          break;

          case 0xb:
          case 0xd:
          breakflag = 1;
          break;

          default:
          break;
        }
        tempnotes[e].command = (modnotes[o+e*4].instr << 3) | goatcommand;
        tempnotes[e].data = goatdata;

        e++;
        if (e == 64) break;
        if (breakflag) break;
      }
      tempnotes[e].note = 0xff;
      for (e = 0; e < goatpatt; e++)
      {
        int f;
        for (f = 0; f < 65; f++)
        {
          if ((goatnotes[e][f].note != tempnotes[f].note) ||
              (goatnotes[e][f].command != tempnotes[f].command) ||
              (goatnotes[e][f].data != tempnotes[f].data)) break;
        }
        // Same pattern
        if (f == 65)
        {
          orderlist[goatchan][d] = e;
          break;
        }
      }
      // No same pattern
      if (e == goatpatt)
      {
        int f;
        if (goatpatt >= 208)
        {
          printf("208 patterns exceeded!\n");
          return 1;
        }
        for (f = 0; f < 65; f++)
        {
          goatnotes[goatpatt][f].note = tempnotes[f].note;
          goatnotes[goatpatt][f].command = tempnotes[f].command;
          goatnotes[goatpatt][f].data = tempnotes[f].data;
        }
        orderlist[goatchan][d] = goatpatt;
        goatpatt++;
      }
    }
    goatchan++;
  }
  printf("Write song\n");
  fwrite(ident, 4, 1, out); // Ident
  fwrite(&modheader[0], 20, 1, out); // Infotexts
  fwrite(zeroarray, 12, 1, out);
  fwrite(zeroarray, 32, 1, out);
  fwrite(zeroarray, 32, 1, out);
  fwrite8(out, 1); // Number of songs
  // Orderlist
  for (c = 0; c < channels; c++)
  {
    fwrite8(out, orderlen+1);
    for (d = 0; d < orderlen; d++)
    {
      fwrite8(out, orderlist[c][d]);
    }
    fwrite8(out, 0xff); // Loop song
    fwrite8(out, 0x0); // From beginning
  }
  for (c = channels; c < 6; c++)
  {
    fwrite8(out, 2);
    fwrite8(out, 0);
    fwrite8(out, 0xff); // Loop song
    fwrite8(out, 0x0); // From beginning
  }
  // Instruments
  for (c = 1; c < 32; c++)
  {
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 2);
    fwrite(&modheader[20+30*(c-1)], 15, 1, out);
    fwrite8(out, 0);
    fwrite8(out, 0);
    fwrite8(out, 0);
  }
  // Patterns
  fwrite8(out, goatpatt);
  for (c = 0; c < goatpatt; c++)
  {
    fwrite8(out, 65*3);
    for (d = 0; d < 65; d++)
    {
      fwrite8(out, goatnotes[c][d].note);
      fwrite8(out, goatnotes[c][d].command);
      fwrite8(out, goatnotes[c][d].data);
    }
  }
  fclose(out);
  printf("Converted successfully.\n");
  return 0;
}

