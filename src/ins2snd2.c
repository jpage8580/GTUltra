/*
 * GoatTracker V2.0 Instrument -> Sound effect convertor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bme_end.h"
#include "gcommon.h"

int main(int argc, char **argv);
unsigned char swapnybbles(unsigned char n);
void outputbyte(unsigned char c);

int covert = 0;
int binary = 0;
int bytecount = 0;
FILE *handle;
FILE *out;

int main(int argc, char **argv)
{
  int c,d;
  int prevwave = 0xff;
  int currwave = 0;
  int fileok = 0;
  INSTR instr;
  int wavelen,pulselen,filtlen;  
  unsigned char ident[4] = {0};
  unsigned char wavetable[MAX_TABLELEN*2];
  unsigned char pulsetable[MAX_TABLELEN*2];
  unsigned char filttable[MAX_TABLELEN*2];
  unsigned char pulse = 0;


  if (argc < 3)
  {
    printf("Usage: INS2SND2 <instrumentfile> <sourcecodefile> <options>\n"
           "-b Produce binary output\n"
           "-c Produce output in CovertScript format (deprecated)\n\n"
           "Takes a GoatTracker V1.xx or V2 instrument and outputs the corresponding sound\n"
           "effect data as source code (default) or binary. Things that can't be converted\n"
           "and will result in an error:\n"
           "- Waveforms greater than $81\n"
           "- Relative notes\n"
           "- Wavetable longer than 128 bytes\n"
           "- Absolute notes C-0 & C#0\n"
           "Things that will be lost in conversion:\n"
           "- Wavetable loops\n"
           "- Pulsewidth modulation\n"
           "- Filter settings\n");
    return 1;
  }

  if (argc > 3)
  {
    for (c = 3; c < argc; c++)
    {
      if (((argv[c][0] == '-') || (argv[c][0] == '/')) && (strlen(argv[c]) > 1))
      {
        int ch = tolower(argv[c][1]);
        switch(ch)
        {
          case 'b':
          binary = 1;
          break;

          case 'c':
          covert = 1;
          break;
        }
      }
    }
  }


  handle = fopen(argv[1], "rb");
  if (!handle)
  {
    printf("ERROR: Can't open instrumentfile\n");
    return 1;
  }

  memset(wavetable, 0, MAX_TABLELEN*2);
  memset(pulsetable, 0, MAX_TABLELEN*2);
  memset(filttable, 0, MAX_TABLELEN*2);

  fread(ident, 4, 1, handle);
  if (!memcmp(ident, "GTI!", 4))
  {
    fileok = 1;
    instr.ad = fread8(handle);
    instr.sr = fread8(handle);
    pulse = fread8(handle) & 0xfe;
    fread8(handle); // Throw away pulse speed
    fread8(handle); // Throw away pulse limit low
    fread8(handle); // Throw away pulse limit high
    fread8(handle); // Throw away filtersetting
    wavelen = fread8(handle);
    fread(&instr.name, MAX_INSTRNAMELEN, 1, handle);
    fread(wavetable, wavelen, 1, handle);
  }
  if (!memcmp(ident, "GTI2", 4))
  {
    fileok = 1;
    instr.ad = fread8(handle);
    instr.sr = fread8(handle);
    instr.ptr[0] = fread8(handle);
    instr.ptr[1] = fread8(handle);
    instr.ptr[2] = fread8(handle);
    instr.vibdelay = fread8(handle);
    instr.ptr[3] = fread8(handle);
    instr.gatetimer = fread8(handle);
    instr.firstwave = fread8(handle);
    fread(&instr.name, MAX_INSTRNAMELEN, 1, handle);
    wavelen = fread8(handle);
    for (c = 0; c < wavelen; c++)
      wavetable[c*2] = fread8(handle);
    for (c = 0; c < wavelen; c++)
      wavetable[c*2+1] = fread8(handle);
    pulselen = fread8(handle);
    for (c = 0; c < pulselen; c++)
      pulsetable[c*2] = fread8(handle);
    for (c = 0; c < pulselen; c++)
      pulsetable[c*2+1] = fread8(handle);
    filtlen = fread8(handle);
    for (c = 0; c < filtlen; c++)
      filttable[c*2] = fread8(handle);
    for (c = 0; c < filtlen; c++)
      filttable[c*2+1] = fread8(handle);
    pulse = (pulsetable[0] << 4) | (pulsetable[1] >> 4);
  }
  if ((!memcmp(ident, "GTI3", 4)) || (!memcmp(ident, "GTI4", 4) || (!memcmp(ident, "GTI5", 4))))
  {
    fileok = 1;
    instr.ad = fread8(handle);
    instr.sr = fread8(handle);
    instr.ptr[0] = fread8(handle);
    instr.ptr[1] = fread8(handle);
    instr.ptr[2] = fread8(handle);
    instr.ptr[3] = fread8(handle);
    instr.vibdelay = fread8(handle);
    instr.gatetimer = fread8(handle);
    instr.firstwave = fread8(handle);
    fread(&instr.name, MAX_INSTRNAMELEN, 1, handle);
    wavelen = fread8(handle);
    for (c = 0; c < wavelen; c++)
      wavetable[c*2] = fread8(handle);
    for (c = 0; c < wavelen; c++)
      wavetable[c*2+1] = fread8(handle);
    pulselen = fread8(handle);
    for (c = 0; c < pulselen; c++)
      pulsetable[c*2] = fread8(handle);
    for (c = 0; c < pulselen; c++)
      pulsetable[c*2+1] = fread8(handle);
    filtlen = fread8(handle);
    for (c = 0; c < filtlen; c++)
      filttable[c*2] = fread8(handle);
    for (c = 0; c < filtlen; c++)
      filttable[c*2+1] = fread8(handle);
    pulse = (pulsetable[0] << 4) | (pulsetable[1] >> 4);
  }
  fclose(handle);
  if (!fileok)
  {
    printf("ERROR: File is not a GoatTracker instrument!\n");
    return 1;
  }

  if (!binary)
    out = fopen(argv[2], "wt");
  else
    out = fopen(argv[2], "wb");

  if (!out)
  {
    printf("ERROR: Can't write to output file.\n");
    return 1;
  }

  d = 0;
  outputbyte(instr.ad);
  d++;
  outputbyte(instr.sr);
  d++;
  outputbyte(swapnybbles(pulse));
  d++;

  for (c = 0; c < MAX_TABLELEN; c++)
  {
    if (wavetable[c*2] == 0xff)
    {
      outputbyte(0);
      d++;
      break;
    }
    if (wavetable[c*2+1] < 0x82)
    {
      printf("ERROR: Relative note or absolute C-0, C#0 found\n");
      fclose(out);
      return 1;
    }
    if (wavetable[c*2] > 0x81)
    {
      printf("ERROR: Waveform greater than $81 found\n");
      fclose(out);
      return 1;
    }
    if (wavetable[c*2])
    {
      currwave = wavetable[c*2];
    }
    outputbyte(wavetable[c*2+1]);
    d++;
    if (currwave != prevwave)
    {
      outputbyte(currwave);
      prevwave = currwave;
      d++;
    }
  }
  if (d > 255)
  {
    fclose(out);
    printf("ERROR: Sound effect exceeds 255 bytes\n");
    return 1;
  }
  fclose(out);
  return 0;
}

void outputbyte(unsigned char c)
{
  if (binary)
    fwrite8(out, c);
  else
  {
    if (!covert)
    {
      if (!bytecount)
      {
        fprintf(out, "        dc.b ");
      }
      else fprintf(out, ",");
      fprintf(out, "$%02X", c);
      bytecount++;
      if (bytecount == 16)
      {
        bytecount = 0;
        fprintf(out, "\n");
      }
    }
    else
    {
      if (bytecount)
      {
        fprintf(out,",");
      }
      if (bytecount == 16)
      {
        fprintf(out,"\n");
        bytecount = 0;
      }
      bytecount++;
      fprintf(out, "0x%02x", c);
    }
  }
}

unsigned char swapnybbles(unsigned char n)
{
  unsigned char highnybble = n >> 4;
  unsigned char lownybble = n & 0xf;

  return (lownybble << 4) | highnybble;
}

