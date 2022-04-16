//
// GTUltra song data model, loading/saving/conversion (currently GOATTRACKER STEREO V2)
//

#define GSONG_C

#include "goattrk2.h"

INSTR instr[MAX_INSTR];
unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN + 2];

unsigned int detailedTableLValue[MAX_TABLELEN];
unsigned char detailedTableMaxLValue[MAX_TABLELEN];
unsigned char detailedTableMinLValue[MAX_TABLELEN];
int detailedTableBaseLValue[MAX_TABLELEN];

unsigned char detailedTableRValue[MAX_TABLELEN];
unsigned char detailedTableMaxRValue[MAX_TABLELEN];
unsigned char detailedTableMinRValue[MAX_TABLELEN];
int detailedTableBaseRValue[MAX_TABLELEN];





CHN_QUICKPLAY songQuickPlay[MAX_SONGS][MAX_CHN][MAX_SONGLEN + 2];


unsigned char pattern[MAX_PATT][MAX_PATTROWS * 4 + 4];
char songname[MAX_STR];
char authorname[MAX_STR];
char copyrightname[MAX_STR];
int pattlen[MAX_PATT];
int songlen[MAX_SONGS][MAX_CHN];
int highestusedpattern;
int highestusedinstr;
int determinechannels(FILE* handle);


int checkFor3ChannelSong()
{
	int amount;

	// Determine amount of songs to be saved
	int c = MAX_SONGS - 1;
	for (;;)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2])) break;
		if (c == 0) break;
		c--;
	}
	amount = c + 1;

	for (int d = 0; d < amount; d++)	// song
	{
		for (int c = 3; c < MAX_CHN; c++)	// channels
		{
			int length = songlen[d][c];
			if (length)
			{
				for (int i = 0;i < length;i++)
				{
					if (songorder[d][c][i] == LOOPSONG)
						break;

					// Transpose
					if ((songorder[d][c][i] >= TRANSDOWN) && (songorder[d][c][i] < LOOPSONG))
					{
						i++;
					}
					// Repeat
					if ((songorder[d][c][i] >= REPEAT) && (songorder[d][c][i] < TRANSDOWN))
					{
						i++;
					}
					int p = songorder[d][c][i];

			//		sprintf(textbuffer, "echeck. p:%d", p);
			//		printtext(70, 36, 0xe, textbuffer);
					if (patternContainsData(p))
					{
			//			sprintf(textbuffer, "not empty");
			//			printtext(86, 36, 0xe, textbuffer);
						return 6;				// found a pattern that contains data. Save as 6 channel SID
					}
				}
			}
		}
	}

//	sprintf(textbuffer, "empty");
//	printtext(86, 36, 0xe, textbuffer);
	return 3;	// no pattern data found 
}

int patternContainsData(int p)
{
	int patternLen = pattlen[p];
	for (int i = 0;i < patternLen;i++)
	{
		if (pattern[p][(i * 4)] != REST)
			return 1;	// pattern contains data

		for (int j = 1;j < 4;j++)
		{
			if (pattern[p][(i * 4)+j] != 0)
				return 1;	// pattern contains data
		}
	}
	return 0;	// empty pattern

}


int savesong(void)
{
	int c;
	char ident[] = { 'G', 'T', 'S', '5' };
	FILE *handle;
	int amount;

	// Determine amount of songs to be saved
	c = MAX_SONGS - 1;
	for (;;)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2])) break;
		if (c == 0) break;
		c--;
	}
	amount = c + 1;

	int songChannelCount = checkFor3ChannelSong();

	if (strlen(songfilename) < MAX_FILENAME - 4)
	{
		int extfound = 0;
		for (c = strlen(songfilename) - 1; c >= 0; c--)
		{
			if (songfilename[c] == '.') extfound = 1;
		}
		if (!extfound) strcat(songfilename, ".sng");
	}
	handle = fopen(songfilename, "wb");
	if (handle)
	{
		int d;
		int length;

		int writebytes;
		fwrite(ident, 4, 1, handle);

		// Determine amount of patterns & instruments
		countpatternlengths();
		for (c = 1; c < MAX_INSTR; c++)
		{
			if ((instr[c].ad) || (instr[c].sr) || (instr[c].ptr[0]) || (instr[c].ptr[1]) ||
				(instr[c].ptr[2]) || (instr[c].vibdelay) || (instr[c].ptr[3]))
			{
				if (c > highestusedinstr) highestusedinstr = c;
			}
		}

		// Write infotexts
		fwrite(songname, sizeof songname, 1, handle);
		fwrite(authorname, sizeof authorname, 1, handle);
		fwrite(copyrightname, sizeof copyrightname, 1, handle);

		fwrite8(handle, amount);
		// Write songorderlists
		for (d = 0; d < amount; d++)
		{
			for (c = 0; c < songChannelCount; c++)
			{
				length = songlen[d][c] + 1;
				fwrite8(handle, length);
				writebytes = length;
				writebytes++;
				fwrite(songorder[d][c], writebytes, 1, handle);
			}
		}
		// Write amount of instruments
		fwrite8(handle, highestusedinstr);
		// Write instruments
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
		for (c = 0; c < MAX_TABLES; c++)
		{
			writebytes = gettablelen(c);
			fwrite8(handle, writebytes);
			fwrite(ltable[c], writebytes, 1, handle);
			fwrite(rtable[c], writebytes, 1, handle);
		}
		// Write patterns
		amount = highestusedpattern + 1;
		fwrite8(handle, amount);
		for (c = 0; c < amount; c++)
		{
			length = pattlen[c] + 1;
			fwrite8(handle, length);
			fwrite(pattern[c], length * 4, 1, handle);
		}
		fclose(handle);
		strcpy(loadedsongfilename, songfilename);
		return 1;
	}
	return 0;
}

int saveinstrument(void)
{
	int c;
	char ident[] = { 'G', 'T', 'I', '5' };
	FILE *handle;

	if (strlen(instrfilename) < MAX_FILENAME - 4)
	{
		int extfound = 0;
		for (c = strlen(instrfilename) - 1; c >= 0; c--)
		{
			if (instrfilename[c] == '.') extfound = 1;
		}
		if (!extfound) strcat(instrfilename, ".ins");
	}

	handle = fopen(instrfilename, "wb");
	if (handle)
	{
		fwrite(ident, 4, 1, handle);

		// Write instrument
		fwrite8(handle, instr[editorInfo.einum].ad);
		fwrite8(handle, instr[editorInfo.einum].sr);
		fwrite8(handle, instr[editorInfo.einum].ptr[WTBL]);
		fwrite8(handle, instr[editorInfo.einum].ptr[PTBL]);
		fwrite8(handle, instr[editorInfo.einum].ptr[FTBL]);
		fwrite8(handle, instr[editorInfo.einum].ptr[STBL]);
		fwrite8(handle, instr[editorInfo.einum].vibdelay);
		fwrite8(handle, instr[editorInfo.einum].gatetimer);
		fwrite8(handle, instr[editorInfo.einum].firstwave);
		fwrite(&instr[editorInfo.einum].name, MAX_INSTRNAMELEN, 1, handle);
		for (c = 0; c < MAX_TABLES; c++)
		{
			if (instr[editorInfo.einum].ptr[c])
			{
				int pos = instr[editorInfo.einum].ptr[c] - 1;
				int len = gettablepartlen(c, pos);
				fwrite8(handle, len);
				fwrite(&ltable[c][pos], len, 1, handle);
				fwrite(&rtable[c][pos], len, 1, handle);
			}
			else fwrite8(handle, 0);
		}
		fclose(handle);
		return 1;
	}
	return 0;
}

int loadedSongCount = 0;

int loadsong(GTOBJECT *gt)
{
	int c;
	int ok = 0;
	char ident[4];
	FILE *handle;
	int channelstoload = MAX_CHN;


	if (strlen(songfilename) > 2)
	{
		int chCount = 0;
		if (songfilename[0] == '$')
		{
			//			sprintf(textbuffer, "found $");
			//			printtext(70, 36, 0xe, textbuffer);

			for (int i = 0;i < 2;i++)
			{
				char c = songfilename[1 + i];
				if (c >= '0' && c <= '9')
				{
					chCount *= 10;
					c -= '0';
					chCount += c;
				}
			}
		}
		//		sprintf(textbuffer, "found $ %d", chCount);
		//		printtext(70, 36, 0xe, textbuffer);

		if (chCount == 3 || chCount == 6 || chCount == 9 || chCount == 12)
		{
			handleSIDChannelCountChange(&gtObject);
			maxSIDChannels = chCount;
		}

	}
	handle = fopen(songfilename, "rb");

	if (handle)
	{
		fread(ident, 4, 1, handle);
		if ((!memcmp(ident, "GTS3", 4)) || (!memcmp(ident, "GTS4", 4)) || (!memcmp(ident, "GTS5", 4)))
		{
			int d;
			int length;
			int amount;
			int loadsize;
			clearsong(1, 1, 1, 1, 1, gt);
			ok = 1;

			// Read infotexts
			fread(songname, sizeof songname, 1, handle);
			fread(authorname, sizeof authorname, 1, handle);
			fread(copyrightname, sizeof copyrightname, 1, handle);

			// Read songorderlists
			channelstoload = determinechannels(handle);

			amount = fread8(handle);
			for (d = 0; d < amount; d++)
			{
				for (c = 0; c < channelstoload; c++)
				{
					length = fread8(handle);
					loadsize = length;
					loadsize++;
					fread(songorder[d][c], loadsize, 1, handle);
				}
			}
			// Read instruments
			amount = fread8(handle);
			for (c = 1; c <= amount; c++)
			{
				instr[c].ad = fread8(handle);
				instr[c].sr = fread8(handle);
				instr[c].ptr[WTBL] = fread8(handle);
				instr[c].ptr[PTBL] = fread8(handle);
				instr[c].ptr[FTBL] = fread8(handle);
				instr[c].ptr[STBL] = fread8(handle);
				instr[c].vibdelay = fread8(handle);
				instr[c].gatetimer = fread8(handle);
				instr[c].firstwave = fread8(handle);
				fread(&instr[c].name, MAX_INSTRNAMELEN, 1, handle);
			}
			// Read tables
			for (c = 0; c < MAX_TABLES; c++)
			{
				loadsize = fread8(handle);
				fread(ltable[c], loadsize, 1, handle);
				fread(rtable[c], loadsize, 1, handle);
			}
			// Read patterns
			amount = fread8(handle);
			for (c = 0; c < amount; c++)
			{
				length = fread8(handle) * 4;
				fread(pattern[c], length, 1, handle);
			}
			countpatternlengths();
			songchange(gt, 1);
		}

		// Goattracker v2.xx (3-table) import
		if (!memcmp(ident, "GTS2", 4))
		{
			int d;
			int length;
			int amount;
			int loadsize;
			clearsong(1, 1, 1, 1, 1, gt);
			ok = 1;

			// Read infotexts
			fread(songname, sizeof songname, 1, handle);
			fread(authorname, sizeof authorname, 1, handle);
			fread(copyrightname, sizeof copyrightname, 1, handle);

			// Read songorderlists
			channelstoload = determinechannels(handle);
			amount = fread8(handle);
			for (d = 0; d < amount; d++)
			{
				for (c = 0; c < channelstoload; c++)
				{
					length = fread8(handle);
					loadsize = length;
					loadsize++;
					fread(songorder[d][c], loadsize, 1, handle);
				}
			}
			// Read instruments
			amount = fread8(handle);
			for (c = 1; c <= amount; c++)
			{
				instr[c].ad = fread8(handle);
				instr[c].sr = fread8(handle);
				instr[c].ptr[WTBL] = fread8(handle);
				instr[c].ptr[PTBL] = fread8(handle);
				instr[c].ptr[FTBL] = fread8(handle);
				instr[c].vibdelay = fread8(handle);
				instr[c].ptr[STBL] = makespeedtable(fread8(handle), finevibrato, 0) + 1;
				instr[c].gatetimer = fread8(handle);
				instr[c].firstwave = fread8(handle);
				fread(&instr[c].name, MAX_INSTRNAMELEN, 1, handle);
			}
			// Read tables
			for (c = 0; c < MAX_TABLES - 1; c++)
			{
				loadsize = fread8(handle);
				fread(ltable[c], loadsize, 1, handle);
				fread(rtable[c], loadsize, 1, handle);
			}
			// Read patterns
			amount = fread8(handle);
			for (c = 0; c < amount; c++)
			{
				int d;
				length = fread8(handle) * 4;
				fread(pattern[c], length, 1, handle);

				// Convert speedtable-requiring commands
				for (d = 0; d < length; d++)
				{
					switch (pattern[c][d * 4 + 2])
					{
					case CMD_FUNKTEMPO:
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_FUNKTEMPO, 0) + 1;
						break;

					case CMD_PORTAUP:
					case CMD_PORTADOWN:
					case CMD_TONEPORTA:
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_PORTAMENTO, 0) + 1;
						break;

					case CMD_VIBRATO:
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], finevibrato, 0) + 1;
						break;
					}
				}
			}
			countpatternlengths();
			songchange(gt, 1);
		}
		// Goattracker 1.xx import
		if (!memcmp(ident, "GTS!", 4))
		{
			int d;
			int length;
			int amount;
			int loadsize;
			int fw = 0;
			int fp = 0;
			int ff = 0;
			int fi = 0;
			int numfilter = 0;
			unsigned char filtertable[256];
			unsigned char filtermap[64];
			int arpmap[32][256];
			unsigned char pulse[32], pulseadd[32], pulselimitlow[32], pulselimithigh[32];
			int filterjumppos[64];

			clearsong(1, 1, 1, 1, 1, gt);
			ok = 1;

			// Read infotexts
			fread(songname, sizeof songname, 1, handle);
			fread(authorname, sizeof authorname, 1, handle);
			fread(copyrightname, sizeof copyrightname, 1, handle);

			// Read songorderlists
			channelstoload = determinechannels(handle);
			amount = fread8(handle);
			for (d = 0; d < amount; d++)
			{
				for (c = 0; c < channelstoload; c++)
				{
					length = fread8(handle);
					loadsize = length;
					loadsize++;
					fread(songorder[d][c], loadsize, 1, handle);
				}
			}

			// Convert instruments
			for (c = 1; c < 32; c++)
			{
				unsigned char wavelen;

				instr[c].ad = fread8(handle);
				instr[c].sr = fread8(handle);
				pulse[c] = fread8(handle);
				pulseadd[c] = fread8(handle);
				pulselimitlow[c] = fread8(handle);
				pulselimithigh[c] = fread8(handle);
				instr[c].ptr[FTBL] = fread8(handle); // Will be converted later
				if (instr[c].ptr[FTBL] > numfilter) numfilter = instr[c].ptr[FTBL];
				if (pulse[c] & 1) instr[c].gatetimer |= 0x80; // "No hardrestart" flag
				pulse[c] &= 0xfe;
				wavelen = fread8(handle) / 2;
				fread(&instr[c].name, MAX_INSTRNAMELEN, 1, handle);
				instr[c].ptr[WTBL] = fw + 1;

				// Convert wavetable
				for (d = 0; d < wavelen; d++)
				{
					if (fw < MAX_TABLELEN)
					{
						ltable[WTBL][fw] = fread8(handle);
						rtable[WTBL][fw] = fread8(handle);
						if (ltable[WTBL][fw] == 0xff)
							if (rtable[WTBL][fw]) rtable[WTBL][fw] += instr[c].ptr[WTBL] - 1;
						if ((ltable[WTBL][fw] >= 0x8) && (ltable[WTBL][fw] <= 0xf))
							ltable[WTBL][fw] |= 0xe0;
						fw++;
					}
					else
					{
						fread8(handle);
						fread8(handle);
					}
				}

				// Remove empty wavetable afterwards
				if ((wavelen == 2) && (!ltable[WTBL][fw - 2]) && (!rtable[WTBL][fw - 2]))
				{
					instr[c].ptr[WTBL] = 0;
					fw -= 2;
					ltable[WTBL][fw] = 0;
					rtable[WTBL][fw] = 0;
					ltable[WTBL][fw + 1] = 0;
					rtable[WTBL][fw + 1] = 0;
				}

				// Convert pulsetable
				if (pulse[c])
				{
					int pulsetime, pulsedist, hlpos;

					// Check for duplicate pulse settings
					for (d = 1; d < c; d++)
					{
						if ((pulse[d] == pulse[c]) && (pulseadd[d] == pulseadd[c]) && (pulselimitlow[d] == pulselimitlow[c]) &&
							(pulselimithigh[d] == pulselimithigh[c]))
						{
							instr[c].ptr[PTBL] = instr[d].ptr[PTBL];
							goto PULSEDONE;
						}
					}

					// Initial pulse setting
					if (fp >= MAX_TABLELEN) goto PULSEDONE;
					instr[c].ptr[PTBL] = fp + 1;
					ltable[PTBL][fp] = 0x80 | (pulse[c] >> 4);
					rtable[PTBL][fp] = pulse[c] << 4;
					fp++;

					// Pulse modulation
					if (pulseadd[c])
					{
						int startpulse = pulse[c] * 16;
						int currentpulse = pulse[c] * 16;
						// Phase 1: From startpos to high limit
						pulsedist = pulselimithigh[c] * 16 - currentpulse;
						if (pulsedist > 0)
						{
							pulsetime = pulsedist / pulseadd[c];
							currentpulse += pulsetime * pulseadd[c];
							while (pulsetime)
							{
								int acttime = pulsetime;
								if (acttime > 127) acttime = 127;
								if (fp >= MAX_TABLELEN) goto PULSEDONE;
								ltable[PTBL][fp] = acttime;
								rtable[PTBL][fp] = pulseadd[c] / 2;
								fp++;
								pulsetime -= acttime;
							}
						}

						hlpos = fp;
						// Phase 2: from high limit to low limit
						pulsedist = currentpulse - pulselimitlow[c] * 16;
						if (pulsedist > 0)
						{
							pulsetime = pulsedist / pulseadd[c];
							currentpulse -= pulsetime * pulseadd[c];
							while (pulsetime)
							{
								int acttime = pulsetime;
								if (acttime > 127) acttime = 127;
								if (fp >= MAX_TABLELEN) goto PULSEDONE;
								ltable[PTBL][fp] = acttime;
								rtable[PTBL][fp] = -(pulseadd[c] / 2);
								fp++;
								pulsetime -= acttime;
							}
						}

						// Phase 3: from low limit back to startpos/high limit
						if ((startpulse < pulselimithigh[c] * 16) && (startpulse > currentpulse))
						{
							pulsedist = startpulse - currentpulse;
							if (pulsedist > 0)
							{
								pulsetime = pulsedist / pulseadd[c];
								while (pulsetime)
								{
									int acttime = pulsetime;
									if (acttime > 127) acttime = 127;
									if (fp >= MAX_TABLELEN) goto PULSEDONE;
									ltable[PTBL][fp] = acttime;
									rtable[PTBL][fp] = pulseadd[c] / 2;
									fp++;
									pulsetime -= acttime;
								}
							}
							// Pulse jump back to beginning
							if (fp >= MAX_TABLELEN) goto PULSEDONE;
							ltable[PTBL][fp] = 0xff;
							rtable[PTBL][fp] = instr[c].ptr[PTBL] + 1;
							fp++;
						}
						else
						{
							pulsedist = pulselimithigh[c] * 16 - currentpulse;
							if (pulsedist > 0)
							{
								pulsetime = pulsedist / pulseadd[c];
								while (pulsetime)
								{
									int acttime = pulsetime;
									if (acttime > 127) acttime = 127;
									if (fp >= MAX_TABLELEN) goto PULSEDONE;
									ltable[PTBL][fp] = acttime;
									rtable[PTBL][fp] = pulseadd[c] / 2;
									fp++;
									pulsetime -= acttime;
								}
							}
							// Pulse jump back to beginning
							if (fp >= MAX_TABLELEN) goto PULSEDONE;
							ltable[PTBL][fp] = 0xff;
							rtable[PTBL][fp] = hlpos + 1;
							fp++;
						}
					}
					else
					{
						// Pulse stopped
						if (fp >= MAX_TABLELEN) goto PULSEDONE;
						ltable[PTBL][fp] = 0xff;
						rtable[PTBL][fp] = 0;
						fp++;
					}
				PULSEDONE: {}
				}
			}
			// Convert patterns
			amount = fread8(handle);
			for (c = 0; c < amount; c++)
			{
				length = fread8(handle);
				for (d = 0; d < length / 3; d++)
				{
					unsigned char note, cmd, data, instr;
					note = fread8(handle);
					cmd = fread8(handle);
					data = fread8(handle);
					instr = cmd >> 3;
					cmd &= 7;

					switch (note)
					{
					default:
						note += FIRSTNOTE;
						if (note > LASTNOTE) note = REST;
						break;

					case OLDKEYOFF:
						note = KEYOFF;
						break;

					case OLDREST:
						note = REST;
						break;

					case ENDPATT:
						break;
					}
					switch (cmd)
					{
					case 5:
						cmd = CMD_SETFILTERPTR;
						if (data > numfilter) numfilter = data;
						break;

					case 7:
						if (data < 0xf0)
							cmd = CMD_SETTEMPO;
						else
						{
							cmd = CMD_SETMASTERVOL;
							data &= 0x0f;
						}
						break;
					}
					pattern[c][d * 4] = note;
					pattern[c][d * 4 + 1] = instr;
					pattern[c][d * 4 + 2] = cmd;
					pattern[c][d * 4 + 3] = data;
				}
			}
			countpatternlengths();
			fi = highestusedinstr + 1;
			songchange(gt, 1);

			// Read filtertable
			fread(filtertable, 256, 1, handle);

			// Convert filtertable
			for (c = 0; c < 64; c++)
			{
				filterjumppos[c] = -1;
				filtermap[c] = 0;
				if (filtertable[c * 4 + 3] > numfilter) numfilter = filtertable[c * 4 + 3];
			}

			if (numfilter > 63) numfilter = 63;

			for (c = 1; c <= numfilter; c++)
			{
				filtermap[c] = ff + 1;

				if (filtertable[c * 4] | filtertable[c * 4 + 1] | filtertable[c * 4 + 2] | filtertable[c * 4 + 3])
				{
					// Filter set
					if (filtertable[c * 4])
					{
						ltable[FTBL][ff] = 0x80 + (filtertable[c * 4 + 1] & 0x70);
						rtable[FTBL][ff] = filtertable[c * 4];
						ff++;
						if (filtertable[c * 4 + 2])
						{
							ltable[FTBL][ff] = 0x00;
							rtable[FTBL][ff] = filtertable[c * 4 + 2];
							ff++;
						}
					}
					else
					{
						// Filter modulation
						int time = filtertable[c * 4 + 1];

						while (time)
						{
							int acttime = time;
							if (acttime > 127) acttime = 127;
							ltable[FTBL][ff] = acttime;
							rtable[FTBL][ff] = filtertable[c * 4 + 2];
							ff++;
							time -= acttime;
						}
					}

					// Jump to next step: unnecessary if follows directly
					if (filtertable[c * 4 + 3] != c + 1)
					{
						filterjumppos[c] = ff;
						ltable[FTBL][ff] = 0xff;
						rtable[FTBL][ff] = filtertable[c * 4 + 3]; // Fix the jump later
						ff++;
					}
				}
			}

			// Now fix jumps as the filterstep mapping is known
			for (c = 1; c <= numfilter; c++)
			{
				if (filterjumppos[c] != -1)
					rtable[FTBL][filterjumppos[c]] = filtermap[rtable[FTBL][filterjumppos[c]]];
			}

			// Fix filterpointers in instruments
			for (c = 1; c < 32; c++)
				instr[c].ptr[FTBL] = filtermap[instr[c].ptr[FTBL]];

			// Now fix pattern commands
			memset(arpmap, 0, sizeof arpmap);
			for (c = 0; c < MAX_PATT; c++)
			{
				unsigned char i = 0;
				for (d = 0; d <= MAX_PATTROWS; d++)
				{
					if (pattern[c][d * 4 + 1]) i = pattern[c][d * 4 + 1];

					// Convert portamento & vibrato
					if (pattern[c][d * 4 + 2] == CMD_PORTAUP)
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_PORTAMENTO, 0) + 1;
					if (pattern[c][d * 4 + 2] == CMD_PORTADOWN)
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_PORTAMENTO, 0) + 1;
					if (pattern[c][d * 4 + 2] == CMD_TONEPORTA)
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_PORTAMENTO, 0) + 1;
					if (pattern[c][d * 4 + 2] == CMD_VIBRATO)
						pattern[c][d * 4 + 3] = makespeedtable(pattern[c][d * 4 + 3], MST_NOFINEVIB, 0) + 1;

					// Convert filterjump
					if (pattern[c][d * 4 + 2] == CMD_SETFILTERPTR)
						pattern[c][d * 4 + 3] = filtermap[pattern[c][d * 4 + 3]];

					// Convert funktempo
					if ((pattern[c][d * 4 + 2] == CMD_SETTEMPO) && (!pattern[c][d * 4 + 3]))
					{
						pattern[c][d * 4 + 2] = CMD_FUNKTEMPO;
						pattern[c][d * 4 + 3] = makespeedtable((filtertable[2] << 4) | (filtertable[3] & 0x0f), MST_FUNKTEMPO, 0) + 1;
					}
					// Convert arpeggio
					if ((pattern[c][d * 4 + 2] == CMD_DONOTHING) && (pattern[c][d * 4 + 3]))
					{
						// Must be in conjunction with a note
						if ((pattern[c][d * 4] >= FIRSTNOTE) && (pattern[c][d * 4] <= LASTNOTE))
						{
							unsigned char param = pattern[c][d * 4 + 3];
							if (i)
							{
								// Old arpeggio
								if (arpmap[i][param])
								{
									// As command, or as instrument?
									if (arpmap[i][param] < 256)
									{
										pattern[c][d * 4 + 2] = CMD_SETWAVEPTR;
										pattern[c][d * 4 + 3] = arpmap[i][param];
									}
									else
									{
										pattern[c][d * 4 + 1] = arpmap[i][param] - 256;
										pattern[c][d * 4 + 3] = 0;
									}
								}
								else
								{
									int e;
									unsigned char arpstart;
									unsigned char arploop;

									// New arpeggio
									// Copy first the instrument's wavetable up to loop/end point
									arpstart = fw + 1;
									if (instr[i].ptr[WTBL])
									{
										for (e = instr[i].ptr[WTBL] - 1;; e++)
										{
											if (ltable[WTBL][e] == 0xff) break;
											if (fw < MAX_TABLELEN)
											{
												ltable[WTBL][fw] = ltable[WTBL][e];
												fw++;
											}
										}
									}
									// Then make the arpeggio
									arploop = fw + 1;
									if (fw < MAX_TABLELEN - 3)
									{
										ltable[WTBL][fw] = (param & 0x80) >> 7;
										rtable[WTBL][fw] = (param & 0x70) >> 4;
										fw++;
										ltable[WTBL][fw] = (param & 0x80) >> 7;
										rtable[WTBL][fw] = (param & 0xf);
										fw++;
										ltable[WTBL][fw] = (param & 0x80) >> 7;
										rtable[WTBL][fw] = 0;
										fw++;
										ltable[WTBL][fw] = 0xff;
										rtable[WTBL][fw] = arploop;
										fw++;

										// Create new instrument if possible
										if (fi < MAX_INSTR)
										{
											arpmap[i][param] = fi + 256;
											instr[fi] = instr[i];
											instr[fi].ptr[WTBL] = arpstart;
											// Add arpeggio parameter to new instrument name
											if (strlen(instr[fi].name) < MAX_INSTRNAMELEN - 3)
											{
												char arpname[8];
												sprintf(arpname, "0%02X", param & 0x7f);
												strcat(instr[fi].name, arpname);
											}
											fi++;
										}
										else
										{
											arpmap[i][param] = arpstart;
										}
									}

									if (arpmap[i][param])
									{
										// As command, or as instrument?
										if (arpmap[i][param] < 256)
										{
											pattern[c][d * 4 + 2] = CMD_SETWAVEPTR;
											pattern[c][d * 4 + 3] = arpmap[i][param];
										}
										else
										{
											pattern[c][d * 4 + 1] = arpmap[i][param] - 256;
											pattern[c][d * 4 + 3] = 0;
										}
									}
								}
							}
						}
						// If arpeggio could not be converted, databyte zero
						if (!pattern[c][d * 4 + 2])
							pattern[c][d * 4 + 3] = 0;
					}
				}
			}
		}
		fclose(handle);
	}
	if (ok)
	{
		loadedSongCount++;

		strcpy(loadedsongfilename, songfilename);

		// Reset table views
		for (c = 0; c < MAX_TABLES; c++) settableview(c, 0);

		// Convert pulsemodulation speed of < v2.4 songs
		if (ident[3] < '4')
		{
			for (c = 0; c < MAX_TABLELEN; c++)
			{
				if ((ltable[PTBL][c] < 0x80) && (rtable[PTBL][c]))
				{
					int speed = ((signed char)rtable[PTBL][c]);
					speed <<= 1;
					if (speed > 127) speed = 127;
					if (speed < -128) speed = -128;
					rtable[PTBL][c] = speed;
				}
			}
		}

		// Convert old legato/nohr parameters
		if (ident[3] < '5')
		{
			for (c = 1; c < MAX_INSTR; c++)
			{
				if (instr[c].firstwave >= 0x80)
				{
					instr[c].gatetimer |= 0x80;
					instr[c].firstwave &= 0x7f;
				}
				if (!instr[c].firstwave) instr[c].gatetimer |= 0x40;
			}
		}


		// If was a mono song, create empty orderlists for channels 4-6
		if (channelstoload <= MAX_CHN)
		{
			int emptypatt = MAX_PATT - 1;

			findusedpatterns();
			for (c = 0; c < MAX_PATT; c++)
			{
				if (!pattused[c])
				{
					int d;
					int ok = 1;
					for (d = 0; d < pattlen[c]; d++)
					{
						if ((pattern[c][d * 4] != REST) || (pattern[c][d * 4 + 1] != 0x00) ||
							(pattern[c][d * 4 + 2] != 0x00) || (pattern[c][d * 4 + 3] != 0x00))
							ok = 0;
					}

					if (ok)
					{
						emptypatt = c;
						break;
					}
				}
			}

			int evenSongLen = 0;
			for (c = 0; c < MAX_SONGS; c++)
			{
				int evenSong = c & 1;
				if (!evenSong)
					evenSongLen = songlen[c][0];

				if (songlen[c][0])
				{
					int d;
					for (d = channelstoload; d < MAX_CHN; d++)
					{
						songorder[c][d][0] = emptypatt;
						songorder[c][d][1] = 0xff;
						songorder[c][d][2] = 0x00;
						songlen[c][d] = 1;
					}
				}
				else if ((evenSong) && evenSongLen > 0 && maxSIDChannels > 6)
				{
					for (int d = 0; d < MAX_CHN; d++)
					{
						songorder[c][d][0] = emptypatt;
						songorder[c][d][1] = 0xff;
						songorder[c][d][2] = 0x00;
						songlen[c][d] = 1;
					}
				}
			}
			songchange(gt, 1);
		}
	}


	return ok;
}

void loadinstrument(GTOBJECT *gt)
{
	char ident[4];
	FILE *handle;
	int c, d;
	int pulsestart = -1;
	int pulseend = -1;

	handle = fopen(instrfilename, "rb");
	if (handle)
	{
		stopsong(gt);
		fread(ident, 4, 1, handle);

		if ((!memcmp(ident, "GTI3", 4)) || (!memcmp(ident, "GTI4", 4)) || (!memcmp(ident, "GTI5", 4)))
		{
			unsigned char optr[4];

			instr[editorInfo.einum].ad = fread8(handle);
			instr[editorInfo.einum].sr = fread8(handle);
			optr[0] = fread8(handle);
			optr[1] = fread8(handle);
			optr[2] = fread8(handle);
			optr[3] = fread8(handle);
			instr[editorInfo.einum].vibdelay = fread8(handle);
			instr[editorInfo.einum].gatetimer = fread8(handle);
			instr[editorInfo.einum].firstwave = fread8(handle);
			fread(&instr[editorInfo.einum].name, MAX_INSTRNAMELEN, 1, handle);

			// Erase old tabledata
			deleteinstrtable(editorInfo.einum);

			// Load new tabledata
			for (c = 0; c < MAX_TABLES; c++)
			{
				int start = gettablelen(c);
				int len = fread8(handle);

				if (len)
				{
					for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
						ltable[c][d] = fread8(handle);
					while (d < start + len)
					{
						fread8(handle);
						d++;
					}
					for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
						rtable[c][d] = fread8(handle);
					while (d < start + len)
					{
						fread8(handle);
						d++;
					}
					if (c != STBL)
					{
						for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
						{
							if (ltable[c][d] == 0xff)
							{
								if (rtable[c][d])
									rtable[c][d] = rtable[c][d] - optr[c] + start + 1;
							}
						}
					}
					if (c == PTBL)
					{
						pulsestart = start;
						pulseend = start + len;
					}
					instr[editorInfo.einum].ptr[c] = start + 1;
				}
				else instr[editorInfo.einum].ptr[c] = 0;
			}
		}

		// Goattracker v2.xx (3-table) import
		if (!memcmp(ident, "GTI2", 4))
		{
			unsigned char optr[3];

			instr[editorInfo.einum].ad = fread8(handle);
			instr[editorInfo.einum].sr = fread8(handle);
			optr[0] = fread8(handle);
			optr[1] = fread8(handle);
			optr[2] = fread8(handle);
			instr[editorInfo.einum].vibdelay = fread8(handle);
			instr[editorInfo.einum].ptr[STBL] = makespeedtable(fread8(handle), finevibrato, 0) + 1;
			instr[editorInfo.einum].gatetimer = fread8(handle);
			instr[editorInfo.einum].firstwave = fread8(handle);
			fread(&instr[editorInfo.einum].name, MAX_INSTRNAMELEN, 1, handle);

			// Erase old tabledata
			deleteinstrtable(editorInfo.einum);

			// Load new tabledata
			for (c = 0; c < MAX_TABLES - 1; c++)
			{
				int start = gettablelen(c);
				int len = fread8(handle);

				if (len)
				{
					for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
						ltable[c][d] = fread8(handle);
					while (d < start + len)
					{
						fread8(handle);
						d++;
					}
					for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
						rtable[c][d] = fread8(handle);
					while (d < start + len)
					{
						fread8(handle);
						d++;
					}
					for (d = start; (d < start + len) && (d < MAX_TABLELEN); d++)
					{
						if (ltable[c][d] == 0xff)
						{
							if (rtable[c][d])
								rtable[c][d] = rtable[c][d] - optr[c] + start + 1;
						}
					}
					if (c == PTBL)
					{
						pulsestart = start;
						pulseend = start + len;
					}
					instr[editorInfo.einum].ptr[c] = start + 1;
				}
				else instr[editorInfo.einum].ptr[c] = 0;
			}
		}
		// Goattracker 1.xx import
		if (!memcmp(ident, "GTI!", 4))
		{

			unsigned char pulse, pulseadd, pulselimitlow, pulselimithigh, wavelen;
			unsigned char filtertemp[4];
			int fw, fp, ff;

			// Erase old tabledata
			deleteinstrtable(editorInfo.einum);

			fw = gettablelen(WTBL);
			fp = gettablelen(PTBL);
			ff = gettablelen(FTBL);

			instr[editorInfo.einum].ad = fread8(handle);
			instr[editorInfo.einum].sr = fread8(handle);
			if (multiplier)
				instr[editorInfo.einum].gatetimer = 2 * multiplier;
			else
				instr[editorInfo.einum].gatetimer = 1;
			instr[editorInfo.einum].firstwave = 0x9;
			pulse = fread8(handle);
			pulseadd = fread8(handle);
			pulselimitlow = fread8(handle);
			pulselimithigh = fread8(handle);
			instr[editorInfo.einum].ptr[FTBL] = fread8(handle) ? ff + 1 : 0;
			if (pulse & 1) instr[editorInfo.einum].gatetimer |= 0x80; // "No hardrestart" flag
			wavelen = fread8(handle) / 2;
			fread(&instr[editorInfo.einum].name, MAX_INSTRNAMELEN, 1, handle);
			instr[editorInfo.einum].ptr[WTBL] = fw + 1;

			// Convert wavetable
			for (d = 0; d < wavelen; d++)
			{
				if (fw < MAX_TABLELEN)
				{
					ltable[WTBL][fw] = fread8(handle);
					rtable[WTBL][fw] = fread8(handle);
					if (ltable[WTBL][fw] == 0xff)
						if (rtable[WTBL][fw]) rtable[WTBL][fw] += instr[editorInfo.einum].ptr[WTBL] - 1;
					fw++;
				}
				else
				{
					fread8(handle);
					fread8(handle);
				}
			}

			// Remove empty wavetable afterwards
			if ((wavelen == 2) && (!ltable[WTBL][fw - 2]) && (!rtable[WTBL][fw - 2]))
			{
				instr[editorInfo.einum].ptr[WTBL] = 0;
				fw -= 2;
				ltable[WTBL][fw] = 0;
				rtable[WTBL][fw] = 0;
				ltable[WTBL][fw + 1] = 0;
				rtable[WTBL][fw + 1] = 0;
			}

			// Convert pulsetable
			pulse &= 0xfe;
			if (pulse)
			{
				int pulsetime, pulsedist, hlpos;

				// Initial pulse setting
				if (fp >= MAX_TABLELEN) goto PULSEDONE;
				pulsestart = fp;
				instr[editorInfo.einum].ptr[PTBL] = fp + 1;
				ltable[PTBL][fp] = 0x80 | (pulse >> 4);
				rtable[PTBL][fp] = pulse << 4;
				fp++;

				// Pulse modulation
				if (pulseadd)
				{
					int startpulse = pulse * 16;
					int currentpulse = pulse * 16;
					// Phase 1: From startpos to high limit
					pulsedist = pulselimithigh * 16 - currentpulse;
					if (pulsedist > 0)
					{
						pulsetime = pulsedist / pulseadd;
						currentpulse += pulsetime * pulseadd;
						while (pulsetime)
						{
							int acttime = pulsetime;
							if (acttime > 127) acttime = 127;
							if (fp >= MAX_TABLELEN) goto PULSEDONE;
							ltable[PTBL][fp] = acttime;
							rtable[PTBL][fp] = pulseadd / 2;
							fp++;
							pulsetime -= acttime;
						}
					}

					hlpos = fp;
					// Phase 2: from high limit to low limit
					pulsedist = currentpulse - pulselimitlow * 16;
					if (pulsedist > 0)
					{
						pulsetime = pulsedist / pulseadd;
						currentpulse -= pulsetime * pulseadd;
						while (pulsetime)
						{
							int acttime = pulsetime;
							if (acttime > 127) acttime = 127;
							if (fp >= MAX_TABLELEN) goto PULSEDONE;
							ltable[PTBL][fp] = acttime;
							rtable[PTBL][fp] = -(pulseadd / 2);
							fp++;
							pulsetime -= acttime;
						}
					}

					// Phase 3: from low limit back to startpos/high limit
					if ((startpulse < pulselimithigh * 16) && (startpulse > currentpulse))
					{
						pulsedist = startpulse - currentpulse;
						if (pulsedist > 0)
						{
							pulsetime = pulsedist / pulseadd;
							while (pulsetime)
							{
								int acttime = pulsetime;
								if (acttime > 127) acttime = 127;
								if (fp >= MAX_TABLELEN) goto PULSEDONE;
								ltable[PTBL][fp] = acttime;
								rtable[PTBL][fp] = pulseadd / 2;
								fp++;
								pulsetime -= acttime;
							}
						}
						// Pulse jump back to beginning
						if (fp >= MAX_TABLELEN) goto PULSEDONE;
						ltable[PTBL][fp] = 0xff;
						rtable[PTBL][fp] = instr[editorInfo.einum].ptr[PTBL] + 1;
						fp++;
					}
					else
					{
						pulsedist = pulselimithigh * 16 - currentpulse;
						if (pulsedist > 0)
						{
							pulsetime = pulsedist / pulseadd;
							while (pulsetime)
							{
								int acttime = pulsetime;
								if (acttime > 127) acttime = 127;
								if (fp >= MAX_TABLELEN) goto PULSEDONE;
								ltable[PTBL][fp] = acttime;
								rtable[PTBL][fp] = pulseadd / 2;
								fp++;
								pulsetime -= acttime;
							}
						}
						// Pulse jump back to beginning
						if (fp >= MAX_TABLELEN) goto PULSEDONE;
						ltable[PTBL][fp] = 0xff;
						rtable[PTBL][fp] = hlpos + 1;
						fp++;
					}
				}
				else
				{
					// Pulse stopped
					if (fp >= MAX_TABLELEN) goto PULSEDONE;
					ltable[PTBL][fp] = 0xff;
					rtable[PTBL][fp] = 0;
					fp++;
				}
			PULSEDONE:
				pulseend = fp;
			}

			// Convert filter (if any)
			if ((instr[editorInfo.einum].ptr[FTBL]) && (ff < MAX_TABLELEN - 2))
			{
				fread(filtertemp, sizeof filtertemp, 1, handle);
				// Filter set
				if (filtertemp[0])
				{
					ltable[FTBL][ff] = 0x80 + (filtertemp[1] & 0x70);
					rtable[FTBL][ff] = filtertemp[0];
					ff++;
					if (filtertemp[2])
					{
						ltable[FTBL][ff] = 0x00;
						rtable[FTBL][ff] = filtertemp[2];
						ff++;
					}
				}
				else
				{
					// Filter modulation
					int time = filtertemp[1];

					while (time)
					{
						int acttime = time;
						if (acttime > 127) acttime = 127;
						ltable[FTBL][ff] = acttime;
						rtable[FTBL][ff] = filtertemp[2];
						ff++;
						time -= acttime;
					}
				}

				// Jump to next step: always end the filter
				ltable[FTBL][ff] = 0xff;
				rtable[FTBL][ff] = 0;
				ff++;
			}
		}

		fclose(handle);

		// Convert pulsemodulation speed of < v2.4 instruments
		if ((ident[3] < '4') && (pulsestart != -1))
		{
			for (c = pulsestart; (c < pulseend) && (c < MAX_TABLELEN); c++)
			{
				if ((ltable[PTBL][c] < 0x80) && (rtable[PTBL][c]))
				{
					int speed = ((signed char)rtable[PTBL][c]);
					speed <<= 1;
					if (speed > 127) speed = 127;
					if (speed < -128) speed = -128;
					rtable[PTBL][c] = speed;
				}
			}
		}
		// Convert old legato/nohr parameters
		if (ident[3] < '5')
		{
			if (instr[editorInfo.einum].firstwave >= 0x80)
			{
				instr[editorInfo.einum].firstwave &= 0x7f;
				instr[editorInfo.einum].gatetimer |= 0x80;
			}
			if (!instr[editorInfo.einum].firstwave) instr[editorInfo.einum].gatetimer |= 0x40;
		}
	}
}

void clearsong(int cs, int cp, int ci, int ct, int cn, GTOBJECT *gt)
{
	int c;

	if (!(cs | cp | ci | ct | cn)) return;

	stopsong(gt);

	gt->masterfader = 0x0f;
	editorInfo.epmarkchn = -1;
	editorInfo.etmarknum = -1;
	editorInfo.esmarkchn = -1;
	followplay = 0;

	for (c = 0; c < maxSIDChannels; c++)
	{

		gt->chn[c].mute = 0;
		if (multiplier)
			gt->chn[c].tempo = multiplier * 6 - 1;
		else
			gt->chn[c].tempo = 6 - 1;
		gt->chn[c].pattptr = 0;
		gt->chn[c].lastpattptr = 0;
	}


	initQuickPlay();



	for (c = 0; c < MAX_CHN; c++)
	{
		if (cs)
		{
			memset(loadedsongfilename, 0, sizeof loadedsongfilename);
			for (int d = 0; d < MAX_SONGS; d++)
			{
				memset(&songorder[d][c][0], 0, MAX_SONGLEN + 2);
				if (!d)
				{
					songorder[d][c][0] = c;
					songorder[d][c][1] = LOOPSONG;
				}
				else
				{
					songorder[d][c][0] = LOOPSONG;
				}
			}

			int a = 1;
			if (maxSIDChannels > 6)
				a = 2;

			for (int i = 0;i < a;i++)
			{
				gt->editorInfo[c + (i * 6)].epnum = songorder[i][c][0];
				gt->editorInfo[c + (i * 6)].espos = 0;
				gt->editorInfo[c + (i * 6)].esend = 0;
			}
		}
	}
	if (cs)
	{
		editorInfo.esview = 0;
		editorInfo.eseditpos = 0;
		editorInfo.escolumn = 0;
		editorInfo.eschn = 0;
		editorInfo.esnum = 0;
		editorInfo.eppos = 0;
		editorInfo.epview = -VISIBLEPATTROWS / 2;
		editorInfo.epcolumn = 0;
		editorInfo.epchn = 0;
	}
	if (cn)
	{
		memset(songname, 0, sizeof songname);
		memset(authorname, 0, sizeof authorname);
		memset(copyrightname, 0, sizeof copyrightname);
		editorInfo.enpos = 0;
	}
	if (cp)
	{
		memset(loadedsongfilename, 0, sizeof loadedsongfilename);
		for (c = 0; c < MAX_PATT; c++)
			clearpattern(c);
	}
	if (ci)
	{
		for (c = 0; c < MAX_INSTR; c++)
			clearinstr(c);
		memset(&instrcopybuffer, 0, sizeof(INSTR));
		editorInfo.eipos = 0;
		editorInfo.eicolumn = 0;
		editorInfo.einum = 1;
	}
	if (ct == 1)
	{
		for (c = MAX_TABLES - 1; c >= 0; c--)
		{
			memset(ltable[c], 0, MAX_TABLELEN);
			memset(rtable[c], 0, MAX_TABLELEN);
			settableview(c, 0);
		}
	}
	countpatternlengths();

	// JP: is this correct? shouldn't I just do undo? back up all patterns, orderlist, tables, instruments?
	if (cs)
		undoInitAllAreas(gt);	// may just reinit no matter what part is getting cleared. ?
}

void countpatternlengths(void)
{
	int c, d, e;

	highestusedpattern = 0;
	highestusedinstr = 0;
	for (c = 0; c < MAX_PATT; c++)
	{
		for (d = 0; d <= MAX_PATTROWS; d++)
		{
			if (pattern[c][d * 4] == ENDPATT) break;
			if ((pattern[c][d * 4] != REST) || (pattern[c][d * 4 + 1]) || (pattern[c][d * 4 + 2]) || (pattern[c][d * 4 + 3]))
				highestusedpattern = c;
			if (pattern[c][d * 4 + 1] > highestusedinstr) highestusedinstr = pattern[c][d * 4 + 1];
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
				if ((songorder[e][c][d] < REPEAT) && (songorder[e][c][d] > highestusedpattern))
					highestusedpattern = songorder[e][c][d];
			}
			songlen[e][c] = d;
		}
	}
}

void countthispattern(GTOBJECT *gt)
{
	int c, d, e;

	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12

	c = gt->editorInfo[c2].epnum;
	for (d = 0; d <= MAX_PATTROWS; d++)
	{
		if (pattern[c][d * 4] == ENDPATT) break;
	}
	pattlen[c] = d;

	e = editorInfo.esnum;
	c = editorInfo.eschn;
	for (d = 0; d < MAX_SONGLEN; d++)
	{
		if (songorder[e][c][d] >= LOOPSONG) break;
		if (songorder[e][c][d] > highestusedpattern)
			highestusedpattern = songorder[e][c][d];
	}
	songlen[e][c] = d;
}

int insertpattern(int p, GTOBJECT *gt)
{
	int c, d, e;

	findusedpatterns();
	if (p >= MAX_PATT - 2) return 0;
	if (pattused[MAX_PATT - 1]) return 0;

	// Mark all patterns from this point onwards for undo.
	for (int i=p+1;i<MAX_PATT-p-2;i++)
	{
		undoAreaSetCheckForChange(UNDO_AREA_PATTERN, i, UNDO_AREA_DIRTY_CHECK);
	}

	memmove(pattern[p + 2], pattern[p + 1], (MAX_PATT - p - 2)*(MAX_PATTROWS * 4 + 4));	// Move ALL pattern data past the insert point up one pattern
	countpatternlengths();

	for (c = 0; c < MAX_SONGS; c++)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2]))
		{
			for (d = 0; d < MAX_CHN; d++)
			{
				for (e = 0; e < songlen[c][d]; e++)
				{
					if ((songorder[c][d][e] < REPEAT) && (songorder[c][d][e] > p) && (songorder[c][d][e] != MAX_PATT - 1))
						songorder[c][d][e]++;
				}
			}
		}
	}

	for (c = 0; c < MAX_CHN; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		if ((gt->editorInfo[c2].epnum > p) && (gt->editorInfo[c2].epnum != MAX_PATT - 1))
			gt->editorInfo[c2].epnum++;
	}

	return 1;
}

void deletepattern(int p, GTOBJECT *gt)
{
	int c, d, e;

	if (p == MAX_PATT - 1) return;

	memmove(pattern[p], pattern[p + 1], (MAX_PATT - p - 1)*(MAX_PATTROWS * 4 + 4));
	clearpattern(MAX_PATT - 1);
	countpatternlengths();

	for (c = 0; c < MAX_SONGS; c++)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2]))
		{
			for (d = 0; d < MAX_CHN; d++)
			{
				for (e = 0; e < songlen[c][d]; e++)
				{
					if ((songorder[c][d][e] < REPEAT) && (songorder[c][d][e] > p))
						songorder[c][d][e]--;
				}
			}
		}
	}

	for (c = 0; c < MAX_CHN; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		if (gt->editorInfo[c2].epnum > p)
			gt->editorInfo[c2].epnum--;
	}
}

void clearpattern(int p)
{
	int c;

	memset(pattern[p], 0, MAX_PATTROWS * 4);
	for (c = 0; c < defaultpatternlength; c++) pattern[p][c * 4] = REST;
	for (c = defaultpatternlength; c <= MAX_PATTROWS; c++) pattern[p][c * 4] = ENDPATT;
}

void findusedpatterns(void)
{
	int c, d, e;

	countpatternlengths();
	memset(pattused, 0, sizeof pattused);
	for (c = 0; c < MAX_SONGS; c++)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2]))
		{
			for (d = 0; d < MAX_CHN; d++)
			{
				for (e = 0; e < songlen[c][d]; e++)
				{
					if (songorder[c][d][e] < REPEAT)
					{
						if (songorder[c][d][e] == 0xcf)
						{

						}
						pattused[songorder[c][d][e]] = 1;
					}
				}
			}
		}
	}
}

void findduplicatepatterns(GTOBJECT *gt)
{
	int c, d;

	findusedpatterns();

	for (c = 0; c < MAX_PATT; c++)
	{
		if (pattused[c])
		{
			for (d = c + 1; d < MAX_PATT; d++)
			{
				if (pattlen[d] == pattlen[c])
				{
					if (!memcmp(pattern[c], pattern[d], pattlen[c] * 4))
					{
						int f, g, h;

						for (f = 0; f < MAX_SONGS; f++)
						{
							if ((songlen[f][0]) &&
								(songlen[f][1]) &&
								(songlen[f][2]))
							{
								for (g = 0; g < MAX_CHN; g++)
								{
									for (h = 0; h < songlen[f][g]; h++)
									{
										if (songorder[f][g][h] == d)
											songorder[f][g][h] = c;
									}
								}
							}
						}
						for (f = 0; f < maxSIDChannels; f++)
						{
							//				int c2 = getActualChannel(esnum, f);	// 0-12
							if (gt->editorInfo[f].epnum == d)
								gt->editorInfo[f].epnum = c;
						}
					}
				}
			}
		}
	}

	findusedpatterns();
}

void optimizeeverything(int oi, int ot, GTOBJECT *gt)
{
	int c, d, e;

	stopsong(gt);

	findduplicatepatterns(gt);

	memset(instrused, 0, sizeof instrused);

	for (c = MAX_PATT - 1; c >= 0; c--)
	{
		if (pattused[c])
		{
			for (d = 0; d < MAX_PATTROWS; d++)
			{
				if (pattern[c][d * 4] == ENDPATT) break;
				if (pattern[c][d * 4 + 1])
					instrused[pattern[c][d * 4 + 1]] = 1;
			}
		}
		else deletepattern(c, gt);
	}

	countpatternlengths();

	if (oi)
	{
		for (c = MAX_INSTR - 2; c >= 1; c--)
		{
			if (!instrused[c])
			{
				clearinstr(c);

				if (c < MAX_INSTR - 2)
				{
					memmove(&instr[c], &instr[c + 1], (MAX_INSTR - 2 - c) * sizeof(INSTR));
					clearinstr(MAX_INSTR - 2);
					for (d = 0; d < MAX_PATT; d++)
					{
						for (e = 0; e < pattlen[d]; e++)
						{
							if ((pattern[d][e * 4 + 1] > c) && (pattern[d][e * 4 + 1] != MAX_INSTR - 1))
								pattern[d][e * 4 + 1]--;
						}
					}
				}
			}
		}
	}

	if (ot)
	{
		for (c = 0; c < MAX_TABLES; c++) optimizetable(c);
	}
}

int determinechannels(FILE* handle)
{
	int returnpos = ftell(handle);
	int c, d;
	int songs = fread8(handle);
	unsigned char songbuffer[257];

	for (d = 0; d < songs; d++)
	{
		for (c = 0; c < MAX_CHN; c++)
		{
			int loadsize = fread8(handle);
			loadsize++;
			memset(songbuffer, 0, 257);
			fread(songbuffer, loadsize, 1, handle);

			// Check that each track of each song has a valid endmark.
			// Should fail if it's a mono song (not certain)
			if ((songbuffer[loadsize - 2] != 0xff) || (songbuffer[loadsize - 1] >= loadsize))
			{
				fseek(handle, returnpos, SEEK_SET);
				return 3;
			}
		}
	}

	fseek(handle, returnpos, SEEK_SET);
	return MAX_CHN;
}

int mergesong(GTOBJECT *gt)
{
	int c;
	char ident[4];
	FILE *handle;
	int songbase;
	int pattbase;
	int instrbase;
	int tablebase[MAX_TABLES];
	int ok = 0;

	// Determine amount of patterns & instruments
	countpatternlengths();
	highestusedinstr = 0;
	for (c = 1; c < MAX_INSTR; c++)
	{
		if ((instr[c].ad) || (instr[c].sr) || (instr[c].ptr[0]) || (instr[c].ptr[1]) ||
			(instr[c].ptr[2]) || (instr[c].vibdelay) || (instr[c].ptr[3]))
		{
			if (c > highestusedinstr) highestusedinstr = c;
		}
	}

	// Determine amount of songs
	c = MAX_SONGS - 1;
	for (;;)
	{
		if ((songlen[c][0]) &&
			(songlen[c][1]) &&
			(songlen[c][2]) &&
			(songlen[c][3]) &&
			(songlen[c][4]) &&
			(songlen[c][5])) break;
		if (c == 0) break;
		c--;
	}

	pattbase = highestusedpattern + 1;
	instrbase = highestusedinstr;
	songbase = c + 1;

	for (c = 0; c < MAX_TABLES; c++)
	{
		tablebase[c] = gettablelen(c);
	}

	handle = fopen(songfilename, "rb");

	if (handle)
	{
		fread(ident, 4, 1, handle);
		if ((!memcmp(ident, "GTS3", 4)) || (!memcmp(ident, "GTS4", 4)) || (!memcmp(ident, "GTS5", 4)))
		{
			int d;
			int e;
			int length;
			int amount;
			int loadsize;
			int channelstoload;

			// Skip infotexts
			fseek(handle, sizeof songname + sizeof authorname + sizeof copyrightname, SEEK_CUR);

			// Read songorderlists
			channelstoload = determinechannels(handle);
			amount = fread8(handle);
			if (amount + songbase > MAX_SONGS)
				goto ABORT;

			for (d = 0; d < amount; d++)
			{
				for (c = 0; c < channelstoload; c++)
				{
					length = fread8(handle);
					loadsize = length;
					loadsize++;
					fread(songorder[songbase + d][c], loadsize, 1, handle);
					// Remap patterns
					for (e = 0; e < loadsize - 1; e++)
					{
						if (songorder[songbase + d][c][e] < REPEAT)
							songorder[songbase + d][c][e] += pattbase;
					}
				}
			}
			// Read instruments
			amount = fread8(handle);
			if (amount + instrbase > MAX_INSTR)
				goto ABORT;
			for (c = 1; c <= amount; c++)
			{
				instr[c + instrbase].ad = fread8(handle);
				instr[c + instrbase].sr = fread8(handle);
				instr[c + instrbase].ptr[WTBL] = fread8(handle);
				instr[c + instrbase].ptr[PTBL] = fread8(handle);
				instr[c + instrbase].ptr[FTBL] = fread8(handle);
				instr[c + instrbase].ptr[STBL] = fread8(handle);
				if (instr[c + instrbase].ptr[WTBL] > 0)
					instr[c + instrbase].ptr[WTBL] += tablebase[WTBL];
				if (instr[c + instrbase].ptr[PTBL] > 0)
					instr[c + instrbase].ptr[PTBL] += tablebase[PTBL];
				if (instr[c + instrbase].ptr[FTBL] > 0)
					instr[c + instrbase].ptr[FTBL] += tablebase[FTBL];
				if (instr[c + instrbase].ptr[STBL] > 0)
					instr[c + instrbase].ptr[STBL] += tablebase[STBL];
				instr[c + instrbase].vibdelay = fread8(handle);
				instr[c + instrbase].gatetimer = fread8(handle);
				instr[c + instrbase].firstwave = fread8(handle);
				fread(&instr[c + instrbase].name, MAX_INSTRNAMELEN, 1, handle);
			}
			// Read tables
			for (c = 0; c < MAX_TABLES; c++)
			{
				loadsize = fread8(handle);
				if (loadsize + tablebase[c] > MAX_TABLELEN)
					goto ABORT;
				fread(&ltable[c][tablebase[c]], loadsize, 1, handle);
				fread(&rtable[c][tablebase[c]], loadsize, 1, handle);
				// Remap jumps and tablecommands
				for (d = tablebase[c]; d < tablebase[c] + loadsize; d++)
				{
					if (ltable[c][d] == 0xff && rtable[c][d] > 0)
						rtable[c][d] += tablebase[c];
					if (c == 0 && (ltable[c][d] >= WAVECMD && ltable[c][d] <= WAVELASTCMD))
					{
						int cmd = ltable[c][d] & 0xf;
						if (cmd == CMD_SETWAVEPTR && rtable[c][d] > 0)
							rtable[c][d] += tablebase[WTBL];
						if (cmd == CMD_SETPULSEPTR && rtable[c][d] > 0)
							rtable[c][d] += tablebase[PTBL];
						if (cmd == CMD_SETFILTERPTR && rtable[c][d] > 0)
							rtable[c][d] += tablebase[FTBL];
						if (((cmd >= CMD_PORTAUP && cmd <= CMD_VIBRATO) || cmd == CMD_FUNKTEMPO) && rtable[c][d] > 0)
							rtable[c][d] += tablebase[STBL];
					}
				}
			}
			// Read patterns
			amount = fread8(handle);
			if (amount + pattbase > MAX_PATT)
				goto ABORT;

			for (c = 0; c < amount; c++)
			{
				length = fread8(handle) * 4;
				fread(pattern[c + pattbase], length, 1, handle);
				// Remap pattern instruments and commands
				for (d = 0; d < length; d += 4)
				{
					if (pattern[c + pattbase][d + 1] > 0)
						pattern[c + pattbase][d + 1] += instrbase;
					if (pattern[c + pattbase][d + 2] == CMD_SETWAVEPTR && pattern[c + pattbase][d + 3] > 0)
						pattern[c + pattbase][d + 3] += tablebase[WTBL];
					if (pattern[c + pattbase][d + 2] == CMD_SETPULSEPTR && pattern[c + pattbase][d + 3] > 0)
						pattern[c + pattbase][d + 3] += tablebase[PTBL];
					if (pattern[c + pattbase][d + 2] == CMD_SETFILTERPTR && pattern[c + pattbase][d + 3] > 0)
						pattern[c + pattbase][d + 3] += tablebase[FTBL];
					if (((pattern[c + pattbase][d + 2] >= CMD_PORTAUP && pattern[c + pattbase][d + 2] <= CMD_VIBRATO) ||
						pattern[c + pattbase][d + 2] == CMD_FUNKTEMPO) && pattern[c + pattbase][d + 3] > 0)
						pattern[c + pattbase][d + 3] += tablebase[STBL];
				}
			}
		}
	}

	ok = 1;
ABORT:
	fclose(handle);
	countpatternlengths();
	songchange(gt, 1);
	return ok;
}


/*
QUICKPLAY
- Clicking on pattern to set play position / play from position has a delay as the song has to emulate from the beginning
Argghh. It wont work as it won't then have the right SID info.
Will leave this code here in case I implement it one day

*/
void initQuickPlay()
{
	for (int c = 0; c < MAX_CHN; c++)
	{
		for (int d = 0; d < MAX_SONGS; d++)
		{
			for (int p = 0; p < MAX_SONGLEN; p++)
			{
				songQuickPlay[d][c][p].updateCounter = -1;
			}
		}
	}
}

void setQuickPlay(int song, int channel, int patternOffset, GTOBJECT *gt, int updateCounter)
{
	songQuickPlay[song][channel][patternOffset].updateCounter = updateCounter;
	for (int i = 0;i < maxSIDChannels;i++)
	{
		memcpy((char*)&songQuickPlay[song][channel][patternOffset].chn[i], &gt->chn[i], sizeof(CHN));
	}
}

int getQuickPlayChannels(int song, int channel, int patternOffset, GTOBJECT *gt, int updateCounter)
{
	if (songQuickPlay[song][channel][patternOffset].updateCounter == updateCounter)
	{
		for (int i = 0;i < maxSIDChannels;i++)
		{
			memcpy(&gt->chn[i], (char*)&songQuickPlay[song][channel][patternOffset].chn[i], sizeof(CHN));
		}
		return 1;
	}
	return 0;
}




