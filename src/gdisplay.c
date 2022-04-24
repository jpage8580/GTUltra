//
// GTUltra screen display routines
//

#define GDISPLAY_C

#include "goattrk2.h"

char *notename[] =
{ "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
 "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
 "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
 "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
 "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
 "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
 "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
 "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "...", "---", "+++" };


char *notenameTableView[] =
{ "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
 "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
 "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
 "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
 "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
 "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
 "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
 "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
 "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
 "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
 "???", "???", "???", "???", "???", "???", "???", "???" };



char timechar[] = { ':', ' ' };

int UIUnderline = 0;


char debugtext[256];


void printmainscreen(GTOBJECT *gt)
{
	clearscreen(getColor(7, CGENERAL_BACKGROUND));
	printstatus(gt);
	fliptoscreen();
}

void displayupdate(GTOBJECT *gt)
{
	if (cursorflashdelay >= 6)
	{
		cursorflashdelay %= 6;
		cursorflash++;
		cursorflash &= 3;
	}


	printstatus(gt);

	if (transportShowKeyboard)
	{
		resetKeyboardDisplay();
		displayNotes(&gtObject);
		displayKeyboard();
	}

	fliptoscreen();

}


void printstatus(GTOBJECT *gt)
{
	int c, d, color, color2;
	int cc = cursorcolortable[cursorflash];
	menu = 0;

	if ((mouseb > MOUSEB_LEFT) && (mousey <= 1) && (!eamode)) menu = 1;

	printblankc(0, 0, getColor(15, 1), MAX_COLUMNS);

	int menuInfoXOffset = 38;
	if (!menu)
	{
		if (!strlen(loadedsongfilename))
			sprintf(textbuffer, "%s", programname);
		else
			sprintf(textbuffer, "%s - %s", programname, loadedsongfilename);
		textbuffer[57] = 0;
		printtext(0, 0, getColor(15, 1), textbuffer);

//		if (monomode)
//			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "M");
//		else
//			printtext(menuInfoXOffset + 20, 0, getColor(0, 1), "M");

		menuInfoXOffset += 2;

		if (usefinevib)
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "FV");
		else
			printtext(menuInfoXOffset + 20, 0, getColor(0, 1), "FV");

		menuInfoXOffset += 3;
		if (optimizepulse)
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "PO");
		else
			printtext(menuInfoXOffset + 20, 0, getColor(0, 1), "PO");
		menuInfoXOffset += 3;

		if (optimizerealtime)
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "RO");
		else
			printtext(menuInfoXOffset + 20, 0, getColor(0, 1), "RO");

		menuInfoXOffset += 3;
		if (ntsc)
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "NTSC");
		else
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), " PAL");

		menuInfoXOffset += 5;
		if (!sidmodel)
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "6581");
		else
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "8580");

		menuInfoXOffset += 5;
		sprintf(textbuffer, "HR:%04X", adparam);
		printtext(menuInfoXOffset + 20, 0, getColor(15, 1), textbuffer);

		menuInfoXOffset += 3;
		if (eamode)
			printbg(menuInfoXOffset + 20 + eacolumn, getColor(0, 0), cc << 8, 1);

		menuInfoXOffset += 5;


		if (multiplier)
		{
			sprintf(textbuffer, "%2dX", multiplier);
			printtext(menuInfoXOffset + 20, 0, getColor(15, 1), textbuffer);
		}
		else printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "25Hz");

		menuInfoXOffset += 5;
		printtext(menuInfoXOffset + 20, 0, getColor(15, 1), "F12=HELP");
	}
	else
	{
		printtext(0, 0, getColor(15, 1), " PLAY | PLAYPOS | PLAYPATT | STOP | LOAD | SAVE | PACK/RL | HELP | CLEAR | QUIT |");
	}

	displayTransportBar(gt);

	if ((followplay) && (isplaying(gt)))
	{
		for (c = 0; c < maxSIDChannels; c++)
		{
			int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
			int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song

			int newpos = gt->chn[c2].pattptr / 4;
			if (gt->chn[c2].advance)
				gt->editorInfo[c2].epnum = gt->chn[c2].pattnum;

			if (newpos > pattlen[gt->editorInfo[c2].epnum])
				newpos = pattlen[gt->editorInfo[c2].epnum];

			if (c == editorInfo.epchn)
			{
				editorInfo.eppos = newpos;
				editorInfo.epview = newpos - VISIBLEPATTROWS / 2;
			}

			newpos = gt->chn[c2].songptr;
			newpos--;
			if (newpos < 0)
				newpos = 0;
			if (newpos > songlen[playingSong][c2])
				newpos = songlen[playingSong][c2];


			gt->editorInfo[c2].espos = gt->chn[c2].songptr - 1;

			if ((c2 == gt->masterLoopChannel) && (gt->chn[c2].advance) && (playingSong == editorInfo.esnum))
			{
				editorInfo.eseditpos = newpos;
				if (newpos - editorInfo.esview < 0)
				{
					editorInfo.esview = newpos;
				}
				if (newpos - editorInfo.esview >= VISIBLEORDERLIST)
				{
					editorInfo.esview = newpos - VISIBLEORDERLIST + 1;
				}
			}


		}
	}

	displayPattern(gt);

	printtext(4, 4, getColor(CEDIT, 0), debugtext);


	int lockPatternColor = getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND);	//0xe;

	sprintf(textbuffer, "CHN  ");
	printtext(40 + 20, getColor(2, 0), lockPatternColor, textbuffer);

	lockPatternColor = getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND);	//0xe;

	sprintf(textbuffer, "ORDERLIST ");
	printtext(40 + 20 + 5, getColor(2, 0), lockPatternColor, textbuffer);


	//	if (!editPaletteMode)
	sprintf(textbuffer, "(SUBTUNE % 02X, POS % 02X)   ", editorInfo.esnum, editorInfo.eseditpos);
	//	else
	//		sprintf(textbuffer, "(PALETTE % 02X, POS % 02X)   ", currentPalettePreset, editorInfo.eseditpos);
	printtext(40 + 20 + 15, getColor(2, 0), getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);

	//	if (!editPaletteMode)
	displayOrderList(gt, cc);
	//	else
	//		displayPaletteInfo(cc);

	sprintf(textbuffer, "                  ", instrumentCount[editorInfo.einum]);
	printtext(60 + 20, 7 + 3 + 5, getColor(0xe, 0), textbuffer);
	sprintf(textbuffer, "USE COUNT: %d", instrumentCount[editorInfo.einum]);
	printtext(60 + 20, 7 + 3 + 5, getColor(0xe, 0), textbuffer);


	sprintf(textbuffer, "INSTRUMENT NUM. %02X  %-18s", editorInfo.einum, instr[editorInfo.einum].name);
	printtext(40 + 20, 7 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);

	sprintf(textbuffer, "Attack/Decay    %02X  ", instr[editorInfo.einum].ad);
	if (editorInfo.eipos == 0) color = CEDIT; else color = CNORMAL;
	printtext(40 + 20, 8 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Sustain/Release %02X  ", instr[editorInfo.einum].sr);
	if (editorInfo.eipos == 1) color = CEDIT; else color = CNORMAL;
	printtext(40 + 20, 9 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Wavetable Pos   %02X  ", instr[editorInfo.einum].ptr[WTBL]);
	if (editorInfo.eipos == 2) color = CEDIT; else color = CNORMAL;
	printtext(40 + 20, 10 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Pulsetable Pos  %02X  ", instr[editorInfo.einum].ptr[PTBL]);
	if (editorInfo.eipos == 3) color = CEDIT; else color = CNORMAL;
	printtext(40 + 20, 11 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Filtertable Pos %02X  ", instr[editorInfo.einum].ptr[FTBL]);
	if (editorInfo.eipos == 4) color = CEDIT; else color = CNORMAL;
	printtext(40 + 20, 12 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Vibrato Param   %02X", instr[editorInfo.einum].ptr[STBL]);
	if (editorInfo.eipos == 5) color = CEDIT; else color = CNORMAL;
	printtext(60 + 20, 8 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "Vibrato Delay   %02X", instr[editorInfo.einum].vibdelay);
	if (editorInfo.eipos == 6) color = CEDIT; else color = CNORMAL;
	printtext(60 + 20, 9 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "HR/Gate Timer   %02X", instr[editorInfo.einum].gatetimer);
	if (editorInfo.eipos == 7) color = CEDIT; else color = CNORMAL;
	printtext(60 + 20, 10 + 3, getColor(color, 0), textbuffer);

	sprintf(textbuffer, "1stFrame Wave   %02X", instr[editorInfo.einum].firstwave);
	if (editorInfo.eipos == 8) color = CEDIT; else color = CNORMAL;
	printtext(60 + 20, 11 + 3, getColor(color, 0), textbuffer);

	if (editorInfo.editmode == EDIT_INSTRUMENT)
	{
		if (editorInfo.eipos < 9)
		{
			if (!eamode) printbg(56 + 20 + editorInfo.eicolumn + 20 * (editorInfo.eipos / 5), 8 + 3 + (editorInfo.eipos % 5), cc << 8, 1);
		}
		else
		{
			if (!eamode) printbg(60 + 20 + strlen(instr[editorInfo.einum].name), 7 + 3, cc << 8, 1);
		}
	}


	displayTables();

	printtext(40 + 20, 21 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "NAME   ");
	sprintf(textbuffer, "%-31s", songname);
	printtext(47 + 20, 21 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);

	printtext(40 + 20, 22 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "AUTHOR ");
	sprintf(textbuffer, "%-32s", authorname);
	printtext(47 + 20, 22 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);

	printtext(40 + 20, 23 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "COPYR. ");
	sprintf(textbuffer, "%-32s", copyrightname);
	printtext(47 + 20, 23 + 3 + 9, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);

	if ((editorInfo.editmode == EDIT_NAMES) && (!eamode))
	{
		switch (editorInfo.nameIndex)
		{
		case 0:
			printbg(47 + 20 + strlen(songname), 21 + 3 + 9, cc << 8, 1);
			break;
		case 1:
			printbg(47 + 20 + strlen(authorname), 22 + 3 + 9, cc << 8, 1);
			break;
		case 2:
			printbg(47 + 20 + strlen(copyrightname), 23 + 3 + 9, cc << 8, 1);
			break;
		}
	}


	switch (autoadvance)
	{
	case 0:
		color = getColor(CINFO_FOREGROUND, CGENERAL_BACKGROUND);	// 10
		break;

	case 1:
		color = getColor(14, CGENERAL_BACKGROUND);	//14;
		break;

	case 2:
		color = getColor(12, CGENERAL_BACKGROUND);	//12;
		break;
	}
}

void resettime(GTOBJECT *gt)
{
	gt->timemin = 0;
	gt->timesec = 0;
	gt->timeframe = 0;
}

void setSongLengthTime(GTOBJECT *gt)
{
	gt->totalFrame = gt->timeframe;
	gt->totalSec = gt->timesec;
	gt->totalMin = gt->timemin;
}

void incrementtime(GTOBJECT *gt)
{
	{
		gt->timeframe++;
		if (!ntsc)
		{
			if (((multiplier) && (gt->timeframe >= PALFRAMERATE * multiplier))
				|| ((!multiplier) && (gt->timeframe >= PALFRAMERATE / 2)))
			{
				gt->timeframe = 0;
				gt->timesec++;
			}
		}
		else
		{
			if (((multiplier) && (gt->timeframe >= NTSCFRAMERATE * multiplier))
				|| ((!multiplier) && (gt->timeframe >= NTSCFRAMERATE / 2)))
			{
				gt->timeframe = 0;
				gt->timesec++;
			}
		}
		if (gt->timesec == 60)
		{
			gt->timesec = 0;
			gt->timemin++;
			gt->timemin %= 60;
		}
	}
}


void clearOrderListDisplay()
{

	sprintf(textbuffer, "RST");
	for (int c = 0; c < MAX_CHN; c++)
	{
		for (int d = 0; d < VISIBLEORDERLIST; d++)
		{
			printtext(44 + 20 + d * 3, 3 + c, getColor(1, 0), textbuffer);
		}
	}
}

void displayOrderList(GTOBJECT *gt, int cc)
{
	int color;

	for (int c = 0; c < MAX_CHN; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song

		printtext(40 + 20 + 3, 3 + c, getColor(15, 0), "  ");

		sprintf(textbuffer, " %d ", c2);	// c + 1);
		printtext(40 + 20, 3 + c, getColor(15, 0), textbuffer);

		if (c2 == gt->masterLoopChannel)
		{
			printbyte(40 + 23, 3 + c, getColor(14, 0), 19);
			printbyte(40 + 23 + 1, 3 + c, getColor(14, 0), 20);
		}

		for (int d = 0; d < VISIBLEORDERLIST; d++)
		{
			int p = editorInfo.esview + d;
			color = CNORMAL;
			if (isplaying(gt))
			{
				int chnpos = gt->chn[c2].songptr;
				chnpos--;
				if (chnpos < 0)
					chnpos = 0;

				if ((p == chnpos) && (gt->chn[c2].advance))
				{
					if (editorInfo.esnum == playingSong)
						color = CPLAYING;
				}

			}
			if (p == gt->editorInfo[c2].espos)
				color = CEDIT;

			if ((gt->editorInfo[c2].esend) && (p == gt->editorInfo[c2].esend))
				color = CEDIT;

			if (c2 >= maxSIDChannels)
				color = 0;	// CMUTE;


			if ((p < 0) || (p > (songlen[editorInfo.esnum][c] + 1)) || (p > MAX_SONGLEN + 1))
			{
				sprintf(textbuffer, "   ");
			}
			else
			{
				if (songorder[editorInfo.esnum][c][p] < LOOPSONG)
				{
					if ((songorder[editorInfo.esnum][c][p] < REPEAT) || (p >= songlen[editorInfo.esnum][c]))
					{
						if (songorder[editorInfo.esnum][c][p] >= 0xd0)
						{
							sprintf(textbuffer, "Error! p=%x songlen=%x c=%x", p, songlen[editorInfo.esnum][c], c);
							printtext(70, 36, 0xe, textbuffer);
						}
						sprintf(textbuffer, "%02X ", songorder[editorInfo.esnum][c][p]);
						if ((p >= songlen[editorInfo.esnum][c]) && (color == CNORMAL)) color = CCOMMAND;
					}
					else
					{
						if (songorder[editorInfo.esnum][c][p] >= TRANSUP)
						{
							sprintf(textbuffer, "+%01X ", songorder[editorInfo.esnum][c][p] & 0xf);
							if (color == CNORMAL) color = CCOMMAND;
						}
						else
						{
							if (songorder[editorInfo.esnum][c][p] >= TRANSDOWN)
							{
								sprintf(textbuffer, "-%01X ", 16 - (songorder[editorInfo.esnum][c][p] & 0x0f));
								if (color == CNORMAL) color = CCOMMAND;
							}
							else
							{
								sprintf(textbuffer, "R%01X ", (songorder[editorInfo.esnum][c][p] + 1) & 0x0f);
								if (color == CNORMAL) color = CCOMMAND;
							}
						}
					}
				}
				if (songorder[editorInfo.esnum][c][p] == LOOPSONG)
				{
					sprintf(textbuffer, "RST");
					if (color == CNORMAL) color = CCOMMAND;
				}
			}
			printtext(45 + 20 + d * 3, 3 + c, getColor(color, 0), textbuffer);
			if (c == editorInfo.esmarkchn)
			{
				if (editorInfo.esmarkstart <= editorInfo.esmarkend)
				{
					if ((p >= editorInfo.esmarkstart) && (p <= editorInfo.esmarkend))
					{
						if (p != editorInfo.esmarkend)
							printbg(45 + 20 + d * 3, 3 + c, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 3);
						else
							printbg(45 + 20 + d * 3, 3 + c, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					}
				}
				else
				{
					if ((p <= editorInfo.esmarkstart) && (p >= editorInfo.esmarkend))
					{
						if (p != editorInfo.esmarkstart)
							printbg(45 + 20 + d * 3, 3 + c, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 3);
						else
							printbg(45 + 20 + d * 3, 3 + c, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					}
				}
			}

			// Flash cursor
			if ((p == editorInfo.eseditpos) && (editorInfo.editmode == EDIT_ORDERLIST) && (editorInfo.eschn == c))
			{
				if (!eamode) printbg(45 + 20 + d * 3 + editorInfo.escolumn, 3 + c, cc << 8, 1);
			}
		}
	}
}


void displayPaletteInfo(int cc)
{
	int color;

	for (int c = 0; c < MAX_CHN; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song
		sprintf(textbuffer, " %d ", c2);	// c + 1);
		printtext(40 + 20, 3 + c, getColor(15, 0), textbuffer);


		for (int d = 0; d < VISIBLEORDERLIST; d++)
		{
			int p = editorInfo.esview + d;
			color = CNORMAL;


			if ((p < 0) || (p >= MAX_PALETTE_ENTRIES) || c > 2)
			{
				sprintf(textbuffer, "   ");
			}
			else
			{
				sprintf(textbuffer, "%02X ", songorder[editorInfo.esnum][c][p]);
			}
			printtext(45 + 20 + d * 3, 3 + c, getColor(color, 0), textbuffer);

			// draw cursor
			if ((p == editorInfo.eseditpos) && (editorInfo.editmode == EDIT_ORDERLIST) && (editorInfo.eschn == c))
			{
				if (!eamode) printbg(45 + 20 + d * 3 + editorInfo.escolumn, 3 + c, cc < 8, 1);
			}
		}
	}
}


void displayPattern(GTOBJECT *gt)
{
	printbytecol(PATTERN_X + 59, PATTERN_Y - 1, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf5, VISIBLEPATTROWS + 2);

	printbyterow(PATTERN_X, PATTERN_Y - 1, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_FOREGROUND), 0x20, 59);

	int cc = cursorcolortable[cursorflash];
	int maxpattlen = 0;
	UIUnderline = UNDERLINE_MASK;

	int highlightPatternLoop = 0;

	int maxChan = MAX_CHN;
	if ((editorInfo.esnum & 1 && maxSIDChannels == 9) || maxSIDChannels == 3)
		maxChan = 3;

	for (int c = 0; c < maxChan; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12

		if (pattlen[gt->editorInfo[c2].epnum] > maxpattlen)
			maxpattlen = pattlen[gt->editorInfo[c2].epnum];
	}

	if (gt->editorInfo[editorInfo.highlightLoopChannel].epnum == editorInfo.highlightLoopPatternNumber)
		highlightPatternLoop = 1;

	printtext(PATTERN_X + 1, PATTERN_Y, getColor(CTITLES_FOREGROUND, CTRANSPORT_FOREGROUND), "POS");
	for (int d = 0; d < VISIBLEPATTROWS; d++)
	{
		int p = editorInfo.epview + d;
		int color = getColor(CPATTERN_FOREGROUND1, CPATTERN_BACKGROUND1);
		if ((p % (stepsize * 2)) < stepsize)
		{
			if ((p% stepsize) == 0)
				color = getColor(CPATTERN_FIRST_FOREGROUND2, CPATTERN_FIRST_BACKGROUND2);
			else
				color = getColor(CPATTERN_FOREGROUND2, CPATTERN_BACKGROUND2);
		}
		else if ((p% stepsize) == 0)
			color = getColor(CPATTERN_FIRST_FOREGROUND1, CPATTERN_FIRST_BACKGROUND1);

		color &= 0xff00;
		color |= CPATTERN_DIVIDER_LINE;

		int colort = getColor(CUNUSED_MUTED_FOREGROUND, CUNUSED_MUTED_BACKGROUND);
		printbyte(PATTERN_X + (MAX_CHN * 9) + 4, PATTERN_Y + 1 + d, colort, 0xff);

		color &= 0xff00;
		//		if ((p% stepsize) == 0)
		//			color |= CPATTERN_INDEX_HIGHLIGHT;
		//		else
		color |= CPATTERN_NOTE_FOREGROUND;

		if ((p < 0) || (p > maxpattlen))
		{
			color = getColor(CUNUSED_MUTED_FOREGROUND, CUNUSED_MUTED_BACKGROUND);
			sprintf(textbuffer, "   ");
			printbyte(PATTERN_X, PATTERN_Y + 1 + d, color, 0xff);
			printbyte(PATTERN_X + (maxChan * 9) + 4, PATTERN_Y + 1 + d, color, 0xff);
		}
		else
		{
			printbyte(PATTERN_X, PATTERN_Y + 1 + d, color, 0xff);
			printbyte(PATTERN_X + (maxChan * 9) + 4, PATTERN_Y + 1 + d, color, 0xff);


			color &= 0xff00;
			if ((p% stepsize) == 0)
				color |= CPATTERN_INDEX_HIGHLIGHT;
			else
				color |= CPATTERN_NOTE_FOREGROUND;

			if (!(patterndispmode & 1))
			{
				if (p < 100)
					sprintf(textbuffer, " %02d", p);
				else
					sprintf(textbuffer, "%03d", p);
			}
			else
				sprintf(textbuffer, " %02X", p);
		}

		printtext(PATTERN_X + 1, PATTERN_Y + 1 + d, color, textbuffer);


		if (d == 0)
		{
			color = CPATTERN_DIVIDER_LINE | (CUNUSED_MUTED_BACKGROUND << 8);
			printbyte(PATTERN_X, PATTERN_Y, color, 0xff);
			printbyte(PATTERN_X + (9 * (MAX_CHN)) + 4, PATTERN_Y, color, 0xff);	// header sepearator 
		}
	}

	for (int c = 0; c < MAX_CHN; c++)
	{
		for (int d = 0; d < VISIBLEPATTROWS; d++)
		{
			int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
			int p = editorInfo.epview + d;

			int color = getColor(CPATTERN_FOREGROUND1, CPATTERN_BACKGROUND1);
			if ((p % (stepsize * 2)) < stepsize)
			{
				if ((p% stepsize) == 0)
					color = getColor(CPATTERN_FIRST_FOREGROUND2, CPATTERN_FIRST_BACKGROUND2);
				else
					color = getColor(CPATTERN_FOREGROUND2, CPATTERN_BACKGROUND2);
			}
			else if ((p% stepsize) == 0)
				color = getColor(CPATTERN_FIRST_FOREGROUND1, CPATTERN_FIRST_BACKGROUND1);

			if ((p < 0) || (p > pattlen[gt->editorInfo[c2].epnum]) || c2 >= maxSIDChannels || c >= maxChan)
			{
				color = getColor(CUNUSED_MUTED_FOREGROUND, CUNUSED_MUTED_BACKGROUND);

			}

			int cl = color & 0xff00;
			cl |= CPATTERN_DIVIDER_LINE;

			printbyte(PATTERN_X + 4 + (c * 9), PATTERN_Y + 1 + d, cl, 0xff);
			if (d == 0)
				printbyte(PATTERN_X + 4 + (c * 9), PATTERN_Y, getColor(CPATTERN_DIVIDER_LINE, CUNUSED_MUTED_BACKGROUND), 0xff);
		}
	}

	for (int c = 0; c < MAX_CHN; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song


		int invalidColumn = 0;
		if (c >= maxChan)
			invalidColumn = 1;

		sprintf(textbuffer, " CH%01X   %02X", c2 + 1, gt->editorInfo[c2].epnum);

		int headerColor = getColor(CTITLES_FOREGROUND, CTRANSPORT_FOREGROUND);
		if (c >= maxChan)
			headerColor = getColor(CUNUSED_MUTED_FOREGROUND, CTRANSPORT_FOREGROUND);

		printtext(PATTERN_X + 4 + c * 9, PATTERN_Y, headerColor, textbuffer);

		if (getFilterOnOff(gt, c2))
			headerColor = getColor(CCOLOR_RED, CGENERAL_BACKGROUND);
		printbyte(PATTERN_X + 9 + c * 9, PATTERN_Y, headerColor, 0xf3);	// Filter on/off marker


		headerColor = getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND);
		if (!(c % 3))
		{
			sprintf(textbuffer, "LO BN HI  CUT:%02X  RES:%01X", getFilterCutoff(gt, c2), getFilterResonance(gt, c2));
			printtext(PATTERN_X + 6 + c * 9, PATTERN_Y - 1, headerColor, textbuffer);

			for (int i = 0;i < 2;i++)
			{
				printbyte(PATTERN_X + 6 + i + c * 9, PATTERN_Y - 1, headerColor, 0xe0 + i);
				printbyte(PATTERN_X + 9 + i + c * 9, PATTERN_Y - 1, headerColor, 0xe2 + i);
				printbyte(PATTERN_X + 12 + i + c * 9, PATTERN_Y - 1, headerColor, 0xe4 + i);
			}

			int filterTypeOffColor = getColor(CUNUSED_MUTED_FOREGROUND, CTRANSPORT_FOREGROUND);
			int filterEnabledType = getFilterType(gt, c2);

			for (int i = 0;i < 2;i++)
			{
				for (int j = 0;j < 3;j++)
				{
					if (!(filterEnabledType&(1 << j)))
					{
						printbg(PATTERN_X + 6 + i + (j * 3) + c * 9, PATTERN_Y - 1, filterTypeOffColor, 2);
					}
				}
			}

		}


		int colorNoChange = 0;



		for (int d = 0; d < VISIBLEPATTROWS; d++)
		{
			int notEmpty = 0;
			colorNoChange = 0;

			int p = editorInfo.epview + d;

			int color = getColor(CPATTERN_FOREGROUND1, CPATTERN_BACKGROUND1);
			if ((p % (stepsize * 2)) < stepsize)
			{
				if ((p% stepsize) == 0)
					color = getColor(CPATTERN_FIRST_FOREGROUND2, CPATTERN_FIRST_BACKGROUND2);
				else
					color = getColor(CPATTERN_FOREGROUND2, CPATTERN_BACKGROUND2);
			}
			else if ((p% stepsize) == 0)
				color = getColor(CPATTERN_FIRST_FOREGROUND1, CPATTERN_FIRST_BACKGROUND1);

			if (gt->chn[c2].lastpattptr == 0x7ffffffff)
			{
				// Not sure what this will do in this case, so let's see what the screen shows...
				color = 2;
				colorNoChange = 1;
			}

			if ((gt->editorInfo[c2].epnum == gt->chn[c2].lastpattnum) && (isplaying(gt)))
			{
				int chnrow = gt->chn[c2].lastpattptr / 4;
				if (chnrow > pattlen[gt->chn[c2].lastpattnum])
					chnrow = pattlen[gt->chn[c2].lastpattnum];
				if (chnrow == p - 1 && !invalidColumn)
				{
					color = getColor(CPATTERN_HIGHLIGHT_PLAYING_LINE_FOREGROUND, CPATTERN_HIGHLIGHT_PLAYING_LINE_BACKGROUND);
				}
			}

			if (gt->chn[c2].mute)
			{
				color = getColor(CUNUSED_MUTED_FOREGROUND, CUNUSED_MUTED_BACKGROUND);
				colorNoChange = 1;
			}


			if ((p < 0) || (p > pattlen[gt->editorInfo[c2].epnum]) || c2 >= maxSIDChannels || invalidColumn)
			{
				color = getColor(CUNUSED_MUTED_FOREGROUND, CUNUSED_MUTED_BACKGROUND);
				colorNoChange = 1;
				sprintf(textbuffer, "        ");
			}
			else
			{
				if (pattern[gt->editorInfo[c2].epnum][p * 4] == ENDPATT)
				{
					sprintf(textbuffer, "PATT.END");
					if (colorNoChange == 0)
					{
						color &= 0xff00;	// keep background (stripes)
						color |= CPATTERN_NOTE_FOREGROUND;
						colorNoChange++;
						//				notEmpty++;
					}
				}
				else
				{
					sprintf(textbuffer, "%s%02X%01X%02X",
						notename[pattern[gt->editorInfo[c2].epnum][p * 4] - FIRSTNOTE],
						pattern[gt->editorInfo[c2].epnum][p * 4 + 1],
						pattern[gt->editorInfo[c2].epnum][p * 4 + 2],
						pattern[gt->editorInfo[c2].epnum][p * 4 + 3]);

					if (patterndispmode & 2)
					{
						if (!pattern[gt->editorInfo[c2].epnum][p * 4 + 1])
							memset(&textbuffer[3], '.', 2);
						if (!pattern[gt->editorInfo[c2].epnum][p * 4 + 2])
							memset(&textbuffer[5], '.', 3);
					}
				}
			}

			int displayCursor = 0;
			if (p == editorInfo.eppos && !invalidColumn)
			{
				displayCursor++;
				color = (color & 0xff00) | CPATTERN_HIGHLIGHT_FOREGROUND;
			}

			int color3 = color;
			int color2 = color;
			int color4 = color;
			int color5 = color;


			if (colorNoChange == 0)
			{
				int n = pattern[gt->editorInfo[c2].epnum][p * 4] - FIRSTNOTE;

				int noteColor = color & 0xff00;	// keep background (stripes)
				noteColor |= CPATTERN_NOTE_FOREGROUND;
				int commandColor = color & 0xff00;	// keep background (stripes)
				commandColor |= CPATTERN_COMMAND_FOREGROUND;

				int instrumentColor = color & 0xff00;	// keep background (stripes)
				instrumentColor |= CPATTERN_INSTRUMENT_FOREGROUND;

				if (n != 93)
				{
					color2 = noteColor; // note
					notEmpty++;
				}

				if (pattern[gt->editorInfo[c2].epnum][p * 4 + 1] != 0)
				{
					color3 = instrumentColor;	// instrument
					notEmpty++;
				}
				if (pattern[gt->editorInfo[c2].epnum][p * 4 + 2] != 0)
				{
					notEmpty++;
					color4 = commandColor;	// command

					if (pattern[gt->editorInfo[c2].epnum][p * 4 + 2] != 0)
						color5 = noteColor;		// data
				}
			}

			int dispCursorLine = 1;

			// Display highlight line where cursor is
			if (p == editorInfo.eppos && dispCursorLine && !invalidColumn && !followplay)
			{
				color2 &= 0xff;
				color2 |= (CPATTERN_HIGHLIGHT_BACKGROUND << 8);
				color3 &= 0xff;
				color3 |= (CPATTERN_HIGHLIGHT_BACKGROUND << 8);
				color4 &= 0xff;
				color4 |= (CPATTERN_HIGHLIGHT_BACKGROUND << 8);
				color5 &= 0xff;
				color5 |= (CPATTERN_HIGHLIGHT_BACKGROUND << 8);
			}
			else
			{
				if (!notEmpty)
				{
					if (isMatchingRGB(currentPalettePreset, color))
					{
						sprintf(textbuffer, "        ");
					}
				}
			}

			printtext(PATTERN_X + 5 + c * 9, PATTERN_Y + 1 + d, color2, textbuffer);
			printtext(PATTERN_X + 8 + c * 9, PATTERN_Y + 1 + d, color3, &textbuffer[3]);
			printtext(PATTERN_X + 10 + c * 9, PATTERN_Y + 1 + d, color4, &textbuffer[5]);
			printtext(PATTERN_X + 11 + c * 9, PATTERN_Y + 1 + d, color5, &textbuffer[6]);

			if (c == editorInfo.epmarkchn)
			{
				if (editorInfo.epmarkstart <= editorInfo.epmarkend)
				{
					if ((p >= editorInfo.epmarkstart) && (p <= editorInfo.epmarkend))
					{
						printbg(PATTERN_X + 5 + c * 9, PATTERN_Y + 1 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 8);

						if (transportLoopPattern)
						{
							printbg(PATTERN_X + 4 + c * 9, PATTERN_Y + 1 + d, getColor(CPATTERN_LOOP_MARKER_FOREGROUND, CPATTERN_LOOP_MARKER_BACKGROUND), 1);
							printbg(PATTERN_X + 4 + 9 + c * 9, PATTERN_Y + 1 + d, getColor(CPATTERN_LOOP_MARKER_FOREGROUND, CPATTERN_LOOP_MARKER_BACKGROUND), 1);
						}
					}

				}
				else
				{
					if ((p <= editorInfo.epmarkstart) && (p >= editorInfo.epmarkend))
					{
						printbg(PATTERN_X + 5 + c * 9, PATTERN_Y + 1 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 8);

						if (transportLoopPattern)
						{
							printbg(PATTERN_X + 4 + c * 9, PATTERN_Y + 1 + d, getColor(CPATTERN_LOOP_MARKER_FOREGROUND, CPATTERN_LOOP_MARKER_BACKGROUND), 1);
							printbg(PATTERN_X + 4 + 9 + c * 9, PATTERN_Y + 1 + d, getColor(CPATTERN_LOOP_MARKER_FOREGROUND, CPATTERN_LOOP_MARKER_BACKGROUND), 1);
						}
					}

				}
			}

			if ((displayCursor) && (editorInfo.editmode == EDIT_PATTERN) && (editorInfo.epchn == c))
			{
				switch (editorInfo.epcolumn)
				{
				case 0:
					if (!eamode) printbg(PATTERN_X + 5 + c * 9, PATTERN_Y + 1 + d, cc << 8, 3);
					break;

				default:
					if (!eamode) printbg(PATTERN_X + 5 + c * 9 + 2 + editorInfo.epcolumn, PATTERN_Y + 1 + d, cc << 8, 1);
					break;
				}
			}
		}
	}

	UIUnderline = 0;

}



void displayTransportBar(GTOBJECT *gt)
{
	for (int i = 0;i < 59;i++)
	{
		printbyte(i, TRANSPORT_BAR_Y - 1, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_FOREGROUND), 0x20);
		printbyte(i, TRANSPORT_BAR_Y, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_FOREGROUND), 0x20);
		printbyte(i, TRANSPORT_BAR_Y + 1, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_FOREGROUND), 0x20);
		printbyte(i, TRANSPORT_BAR_Y + 2, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_FOREGROUND), 0x20);
	}

	for (int i = 0;i < 59;i++)
	{
		printbyte(i, TRANSPORT_BAR_Y, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND), 0xbd);
		printbyte(i, TRANSPORT_BAR_Y + 1, getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND), 0xdd);

	}

	printbyterow(0, TRANSPORT_BAR_Y + 3, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf6, 59);

	displayTransportBarSkinning(0, TRANSPORT_BAR_Y);
	displayTransportBarSIDCount(4, TRANSPORT_BAR_Y);
	displayTransportBarMasterVolume(8, TRANSPORT_BAR_Y);

	int x = TRANSPORT_BAR_X-2;	// +1;
	displayTransportBarOctave(x, TRANSPORT_BAR_Y);
	x += 5;
	displayTransportBarFollow(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarLoopPattern(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarRewind(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarRecord(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarPlaying(gt, x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarFastForward(x, TRANSPORT_BAR_Y);
	x += 5;
	displayTransportBarPolyChannels(x, TRANSPORT_BAR_Y);
	x += 5;
	displayTransportBarKeyboard(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarDetune(x, TRANSPORT_BAR_Y);
	x += 4;
	displayTransportBarMonoStereo(x, TRANSPORT_BAR_Y);



	int tdiv = 25 * multiplier;

	if (multiplier && ntsc)
		tdiv = 30 * multiplier;
	else if (!multiplier && !ntsc)
		tdiv = gt->timeframe / 13;
	else if (!multiplier && ntsc)
		tdiv = gt->timeframe / 15;

	int v = 0;
	if (tdiv)
		v = gt->timeframe / tdiv & 1;

	sprintf(textbuffer, " %02d%c%02d:%02d ", gt->timemin, timechar[v], gt->timesec, gt->timeframe);
	printtext(TRANSPORT_BAR_X + 5, TRANSPORT_BAR_Y - 1, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), textbuffer);
	sprintf(textbuffer, "/ %02d:%02d:%02d ", gtEditorObject.totalMin, gtEditorObject.totalSec, gtEditorObject.totalFrame);
	printtext(TRANSPORT_BAR_X + 5 + 10, TRANSPORT_BAR_Y - 1, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), textbuffer);

	displayWaveformInfo(TRANSPORT_BAR_X - 5, TRANSPORT_BAR_Y - 1);


	printtext(0, TRANSPORT_BAR_Y + 2, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), "INFO:");
	if (editPaletteMode)
	{
		int maxPaletteText = getPaletteTextArraySize();
		int index = editorInfo.eseditpos * 2 + editorInfo.escolumn;
		jcounter = maxPaletteText;

		if (index < maxPaletteText)
		{
			sprintf(textbuffer, "%d %s", FIRST_UI_COLOR + index, paletteText[index]);
			printtext(6, TRANSPORT_BAR_Y + 2, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), textbuffer);
		}
	}
	else
	{
		printtext(6, TRANSPORT_BAR_Y + 2, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), infoTextBuffer);
	}

}

void displayTransportBarFastForward(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xaf + i);
		printbyte(x + i, y + 1, color, 0xcf + i);
	}
}

void displayTransportBarRewind(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xa6 + i);
		printbyte(x + i, y + 1, color, 0xc6 + i);
	}
}

void displayTransportBarPolyChannels(int x, int y)
{
	for (int p = 0;p < 4;p++)
	{
		int imageOffset = 0x12;
		int yoffset = p / 2;
		int xoffset = (p & 1) * 2;

		int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
		if (transportPolySIDEnabled[p])
		{
			imageOffset = 0x19;
			color = getColor(CTRANSPORT_ENABLED, CTRANSPORT_FOREGROUND);
		}

		for (int i = 0;i < 02;i++)
		{
			printbyte(x + i + xoffset, y + yoffset, color, 0xa0 + (32 * yoffset) + i + imageOffset + xoffset);
		}
	}
}

void displayTransportBarKeyboard(int x, int y)
{
	int color = getColor(CTRANSPORT_BACKGROUND, CTRANSPORT_FOREGROUND);
	if (transportShowKeyboard)
		color = getColor(CTRANSPORT_ENABLED, CTRANSPORT_FOREGROUND);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, i + 2);
		printbyte(x + i, y + 1, color, 1);
	}
}



void displayTransportBarPlaying(GTOBJECT *gt, int x, int y)
{
	int imageOffset = 12;
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	if (gt->songinit == PLAY_PLAYING)
		imageOffset = 22;

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xa0 + i + imageOffset);
		printbyte(x + i, y + 1, color, 0xc0 + i + imageOffset);
	}
}
void displayTransportBarRecord(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	if (recordmode)
		color = getColor(CCOLOR_RED, CTRANSPORT_BACKGROUND);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xa0 + i + 9);
		printbyte(x + i, y + 1, color, 0xc0 + i + 9);
	}
}


void displayTransportBarMasterVolume(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 21 + i);
		printbyte(x + i, y + 1, color, 0xe9 + i);
	}

	sprintf(textbuffer, "%.1f", masterVolume);
	printtext(x, y + 1, color, textbuffer);

}

void displayTransportBarOctave(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xe6 + i);
		printbyte(x + i, y + 1, color, 0xe9 + i);
	}
	sprintf(textbuffer, "%1d", editorInfo.epoctave);
	printtext(x + 1, y + 1, color, textbuffer);

}

void displayTransportBarDetune(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0x9d + i);
		printbyte(x + i, y + 1, color, 0xe9 + i);
	}
	sprintf(textbuffer, "%.1f", detuneCent + 1);
	printtext(x + 0, y + 1, color, textbuffer);

}

void displayTransportBarMonoStereo(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);

	int b = 0x195;
	if (monomode || maxSIDChannels == 3)
	{
		printbyte(x, y + 1, color, 0x20);
		printbyte(x + 1, y + 1, color, 0x9c);
		printbyte(x + 2, y + 1, color, 0x20);		
		
	}
	else
	{
		b = 0x198;
		printbyte(x, y + 1, color, 0x9b);
		printbyte(x + 1, y + 1, color, 0x20);
		printbyte(x + 2, y + 1, color, 0x9c);
	}

	// Display MON or STE text
	for (int i = 0;i < 3;i++)
	{
		printbyte(x+i, y, color, b);
		b++;
	}

//	sprintf(textbuffer, "%.1f", detuneCent + 1);
//	printtext(x + 0, y + 1, color, textbuffer);

}

void displayTransportBarSIDCount(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	printbyte(x, y, color, 0xbe);
	printbyte(x + 1, y, color, 0xbf);
	printbyte(x + 2, y, color, 0xde);
	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y + 1, color, 0xe9 + i);
	}
	sprintf(textbuffer, "%1X", maxSIDChannels / 3);
	printtext(x + 1, y + 1, color, textbuffer);

}

void displayTransportBarSkinning(int x, int y)
{

	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	if (editPaletteMode)
		color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_ENABLED);
	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xed + i);
		printbyte(x + i, y + 1, color, 0xf0 + i);
	}

	sprintf(textbuffer, "%1X", currentPalettePreset);
	printtext(x + 2, y + 1, color, textbuffer);

}

void displayTransportBarFollow(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	if (followplay)
		color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_ENABLED);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xa0 + i);
		printbyte(x + i, y + 1, color, 0xc0 + i);
	}
}

void displayTransportBarLoopPattern(int x, int y)
{
	int color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_BACKGROUND);
	if (transportLoopPattern)
		color = getColor(CTRANSPORT_FOREGROUND, CTRANSPORT_ENABLED);

	for (int i = 0;i < 0x3;i++)
	{
		printbyte(x + i, y, color, 0xa0 + i + 3);
		printbyte(x + i, y + 1, color, 0xc0 + i + 3);
	}
}


struct NOTEDISPLAYPLAYING
{
	int note;
	int charIndex;
	int bitmask;

};


// Will do this for each octave,
// note = note number 0-11 (one octave)
// charIndex = charIndex array offset
// bitmask is offset to "or" in char indexArray offset
// We then look up noteChars[noteNumber][indexArray] to get the right image


struct NOTEDISPLAYPLAYING displayKeyboardTopRowOctaveInfo[] = {
 {.note = 0,.charIndex = 0,.bitmask = 1},		// c
 {.note = 1,.charIndex = 0,.bitmask = 2},		// c#
{.note = 1,.charIndex = 1,.bitmask = 1},		// c#
{.note = 2,.charIndex = 1,.bitmask = 2},		// d
{.note = 3,.charIndex = 1,.bitmask = 4},		// d#
{.note = 3,.charIndex = 2,.bitmask = 1},		// d#
{.note = 4,.charIndex = 2,.bitmask = 2},		// e
{.note = 5,.charIndex = 3,.bitmask = 1},		// f
{.note = 6,.charIndex = 3,.bitmask = 2},		// f#
{.note = 6,.charIndex = 4,.bitmask = 1},		// f#
{.note = 7,.charIndex = 4,.bitmask = 2},		// g
{.note = 8,.charIndex = 4,.bitmask = 4},		// g#
{.note = 8,.charIndex = 5,.bitmask = 1},		// g#
{.note = 9,.charIndex = 5,.bitmask = 2},		// a
{.note = 10,.charIndex = 5,.bitmask = 4},		// a#
{.note = 10,.charIndex = 6,.bitmask = 1},		// a#
{.note = 11,.charIndex = 6,.bitmask = 2},		// b
{.note = -1,.charIndex = -1,.bitmask = 0}		// END
};

// Top Row char numbers, based on bitpattern for keyon in charIndexArrayTopRow
char keyboardTopRow_Char0[] = { 2,6,7,8 };	// c + c#
char keyboardTopRow_Char1[] = { 3,9,10,11,12,13,14,15 };		// c#,d,d#
char keyboardTopRow_Char2[] = { 4,16,17,18 };	// d#,e
char keyboardTopRow_Char3[] = { 2,6,7,8 };	// f + f#
char keyboardTopRow_Char4[] = { 3,9,10,11,12,13,14,15 };		// f#,g,g#
char keyboardTopRow_Char5[] = { 3,9,10,11,12,13,14,15 };		// g#,a,a#
char keyboardTopRow_Char6[] = { 4,16,17,18 };	// a#,b

char *keyboardTopRowPtrs[] = {
	(char*)keyboardTopRow_Char0,
	(char*)keyboardTopRow_Char1,
	(char*)keyboardTopRow_Char2,
	(char*)keyboardTopRow_Char3,
	(char*)keyboardTopRow_Char4,
	(char*)keyboardTopRow_Char5,
	(char*)keyboardTopRow_Char6 };

char keyboardBottomRow_Char0[] = { 1,5 };

int keyboardDisplayNoteIndexArray[92];

int charIndexArrayTopRow[59];	// 8 octave. 7 different char arrays (see .charIndex above)
int charOnOffBottomRow[59];	// 8 octave. 7 keys in major scale

char majorKeyOn[12] = { 0,-1,1,-1,2,3,-1,4,-1,5,-1,6 };

// Call this once per frame. Prior to doing the setNote
void resetKeyboardDisplay()
{
	for (int i = 0;i < 49;i++)
	{
		charIndexArrayTopRow[i] = 0;
		charOnOffBottomRow[i] = 0;
	}
}


void displayNotes(GTOBJECT *gt)
{
	for (int i = 0;i < MAX_PLAY_CH;i++)
	{
		if (!gt->chn[i].mute)
		{
			if (gt->chn[i].instr)	// not 0.. So playing
			{
				if (gt->chn[i].releaseTime)
				{
					int closestNote = getClosestNote(gt->chn[i].freq);
					//					closestNote = gt->chn[i].lastnote;
					setNote(closestNote);
				}
			}
		}
	}
}

void setNote(int noteNumber)
{
	int octave = noteNumber / 12;
	int note = noteNumber % 12;

	int index = 0;
	while (displayKeyboardTopRowOctaveInfo[index].note != -1)
	{
		if (displayKeyboardTopRowOctaveInfo[index].note == note)
		{
			int noteCharIndex = displayKeyboardTopRowOctaveInfo[index].charIndex;	// 0-6
			noteCharIndex += octave * 7;
			charIndexArrayTopRow[noteCharIndex] |= displayKeyboardTopRowOctaveInfo[index].bitmask;
		}
		index++;
	};
	if (majorKeyOn[note] >= 0)
	{
		int majorNote = majorKeyOn[note] + (octave * 7);
		charOnOffBottomRow[majorNote] = 1;
	}
}



void displayKeyboard()
{
	int x = 0;
	int y = 3;
	for (int i = 0;i < 59;i++)
	{
		char* cp = keyboardTopRowPtrs[i % 7];
		char charToDisplay = cp[charIndexArrayTopRow[i]];
		printbyte(x + i, y, getColor(15, 0), charToDisplay);
		printbyte(x + i, y + 1, getColor(15, 0), charToDisplay);

		charToDisplay = keyboardBottomRow_Char0[charOnOffBottomRow[i]];
		printbyte(x + i, y + 2, getColor(15, 0), charToDisplay);
	}
}


int getTableTitleColour(int c)
{
	if (editorInfo.editTableMode == EDIT_TABLE_NONE)
		return getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND);

	if (editorInfo.editTableMode == c)
		return getColor(0xe, CGENERAL_BACKGROUND);

	sprintf(textbuffer, "        ");
	return getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND);
}


int lastTableType = EDIT_TABLE_NONE;
void displayTables()
{
	if (editorInfo.editTableMode != lastTableType)
	{
		lastTableType = editorInfo.editTableMode;

		// Clear table panel
		for (int y = 0;y < VISIBLETABLEROWS;y++)
		{
			printbyterow(40 + 20, 15 + 3 + y, getColor(CTABLE_FOREGROUND1, CTABLE_BACKGROUND1), 32, 38);
		}

		if (editorInfo.editTableMode == EDIT_TABLE_WAVE)
		{
			editorInfo.etDetailedWaveTableColumn = 15;	// left table column offset
			if (editorInfo.etnum != EDIT_TABLE_SPEED - 1)
			{
				editorInfo.etnum = WTBL;
				if (editorInfo.etcolumn > 4)	// Not currently editing WAVE table? Move cursor to first column
					editorInfo.etcolumn = 0;
			}
		}
		else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
		{
			editorInfo.etDetailedWaveTableColumn = 8;	// left table column offset
			editorInfo.etnum = FTBL;

			if (editorInfo.etcolumn > 4)	// Not currently editing WAVE table? Move cursor to first column
				editorInfo.etcolumn = 0;
		}
		else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
		{
			editorInfo.etDetailedWaveTableColumn = 8;	// left table column offset
			editorInfo.etnum = PTBL;

			if (editorInfo.etcolumn > 4)	// Not currently editing WAVE table? Move cursor to first column
				editorInfo.etcolumn = 0;
		}
	}

	int cc = cursorcolortable[cursorflash];

	UIUnderline = UNDERLINE_FOREGROUND_MASK;
	sprintf(textbuffer, "WAVE TBL");
	printtext(40 + 20, 14 + 3, getTableTitleColour(EDIT_TABLE_WAVE), textbuffer);

	sprintf(textbuffer, "PULSETBL");
	printtext(40 + 20 + 10, 14 + 3, getTableTitleColour(EDIT_TABLE_PULSE), textbuffer);

	sprintf(textbuffer, "FILT.TBL");
	printtext(40 + 20 + 20, 14 + 3, getTableTitleColour(EDIT_TABLE_FILTER), textbuffer);

	if (editorInfo.editTableMode == EDIT_TABLE_WAVE || editorInfo.editTableMode == EDIT_TABLE_NONE)
	{
		sprintf(textbuffer, "SPEEDTBL");
		printtext(40 + 20 + 30, 14 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);
	}
	else
	{
		sprintf(textbuffer, "        ");
		printtext(40 + 20 + 30, 14 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), textbuffer);
	}

	printtext(40 + 20 + 8, 14 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "  ");
	printtext(40 + 20 + 10 + 8, 14 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "  ");
	printtext(40 + 20 + 20 + 8, 14 + 3, getColor(CTITLES_FOREGROUND, CGENERAL_BACKGROUND), "  ");
	UIUnderline = 0;

	if (editorInfo.editTableMode == EDIT_TABLE_NONE)
	{
		displayOriginalTableView(cc);
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_WAVE)
	{
		displayDetailedWaveTable(cc);
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
	{
		displayDetailedFilterTable(cc);
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
	{
		displayDetailedPulseTable(cc);
	}


	// Table bottom border
	printbyterow(40 + 20, 32, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf6, 38);
	printbytecol(40 + 20 + 38, 32 - 14, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf5, 0xe);

	// Instrument border
	printbyterow(40 + 20, 14 + 2, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf6, 38);
	printbytecol(40 + 20 + 38, 14 + 2 - 5, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf5, 5);

	// orderlist border
	printbyterow(40 + 20, 9, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf6, 38);
	printbytecol(40 + 20 + 38, 9 - 6, getColor(CGENERAL_HIGHLIGHT, CGENERAL_BACKGROUND), 0xf5, 6);


}

/*
Wavetable left side:   00    Leave waveform unchanged
					   01-0F Delay this step by 1-15 frames
					   10-DF Waveform values
					   E0-EF Inaudible waveform values $00-$0F
					   F0-FE Execute command 0XY-EXY. Right side is parameter.
					   FF    Jump. Right side tells position ($00 = stop)

*/
void displayDetailedWaveTable(int cc)
{
	for (int d = 0; d < VISIBLETABLEROWS; d++)
	{
		int p = editorInfo.etview[0] + d;
		int color = tableBackgroundColors[0][p];
		UIUnderline = 0;

		int v = ltable[0][p];
		int v2 = v;
		if (v == 0xff)
			UIUnderline = UNDERLINE_FOREGROUND_MASK;

		int r = rtable[0][p];
		int r2 = r;

		sprintf(textbuffer, "%02X:", p + 1);
		printtext(40 + 20, 15 + 3 + d, color, textbuffer);

		printtext(40 + 24, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), "-WDCJ");
		int highlightOffset = 0;
		int displayRightTable = 0;

		if (v == 0)	// skip
		{
			detailedTableBaseLValue[p] = -1;
			printtext(40 + 33, 15 + 3 + d, color, "---- ");
			displayRightTable = 1;
		}
		else if (v >= 1 && v <= 0x0f)
		{
			detailedTableLValue[p] = v;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xf;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 1;
			detailedTableBaseLValue[p] = 0;		// base value to add to current to get new tableR[p] value

			printtext(40 + 33, 15 + 3 + d, color, "DELY:");
			highlightOffset = 2;	// delay
			displayRightTable = 1;
		}
		else if (v >= 0x10 && v <= 0x0ef)
		{
			highlightOffset = 1;	// wave
			printtext(40 + 33, 15 + 3 + d, color, "WAVE:");
			if (v >= 0xe0)
				v2 -= 0xe0;	// waveforms 0-0xf

			// Will need to check ltable[p] to know how to set final value 
			detailedTableLValue[p] = v2;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xdf;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			displayRightTable = 1;
		}
		else if (v >= 0xf0 && v <= 0x0fe)
		{
			highlightOffset = 3;	// command
			printtext(40 + 33, 15 + 3 + d, color, "CMND:");
			v2 -= 0xf0;

			detailedTableLValue[p] = v2;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xe;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0xf0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			detailedTableRValue[p] = r;			// current L value. Modify this with HexInput
			detailedTableMaxRValue[p] = 0xff;		// maximum possible value (min is always 0)
			detailedTableMinRValue[p] = 0;
			detailedTableBaseRValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)


			displayRightTable = 2;
		}
		else
		{
			detailedTableBaseRValue[p] = -1;

			highlightOffset = 4;	// jump
			if (r)
				printtext(40 + 33, 15 + 3 + d, color, "JUMP:");
			else
				printtext(40 + 33, 15 + 3 + d, color, "STOP:");
			displayRightTable = 0;

			detailedTableLValue[p] = r;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xff;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			v2 = r;
		}

		if (v != 0)
		{
			sprintf(textbuffer, "%02X", v2);
			printtext(40 + 38, 15 + 3 + d, color, textbuffer);
		}
		else
			printtext(40 + 38, 15 + 3 + d, color, "--");


		/*
		Wavetable right side:  00-5F Relative notes
					   60-7F Negative relative notes (lower pitch)
					   80    Keep frequency unchanged
					   81-DF Absolute notes C#0 - B-7

		*/


		printtext(40 + 45, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), "   ");
		if (displayRightTable == 1)	// 1 = display Relative/Absolute / Note name (eg C#4)
		{

			int rhighlight = -1;
			printtext(40 + 30, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), "RA");
			if (r != 0x80)
			{
				if (r >= 0x81)	// && r <= 0xdf)
				{
					r2 -= 0x80;
					detailedTableRValue[p] = r2;
					detailedTableMaxRValue[p] = 0x5f;
					detailedTableBaseRValue[p] = 0x80;
					detailedTableMinRValue[p] = 1;

					rhighlight = 1;
					printtext(40 + 45, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), notenameTableView[r2]);
					sprintf(textbuffer, " %02X", r2);
					printtext(40 + 41, 15 + 3 + d, color, textbuffer);
				}
				else
				{
					rhighlight = 0;
					if (r >= 0x60 && r <= 0x7f)	// negative relative notes
					{
						r2 += 0x80;	// change negative values to 0xe0>0xff
						int r3 = -r2;
						r3 &= 0xff;

						detailedTableRValue[p] = r3;			// current R value. Modify this with HexInput
						detailedTableMaxRValue[p] = 0x20;		// maximum possible value (min is always 0)
						detailedTableBaseRValue[p] = 0x60;		// base value to add to current to get new tableR[p] value
						detailedTableMinRValue[p] = 1;

						sprintf(textbuffer, "-%02X", r3);
						printtext(40 + 41, 15 + 3 + d, color, textbuffer);
					}
					else
					{
						detailedTableRValue[p] = r2;
						detailedTableMaxRValue[p] = 0x5f;
						detailedTableBaseRValue[p] = 0x0;
						detailedTableMinRValue[p] = 0;

						sprintf(textbuffer, "+%02X", r2);
						printtext(40 + 41, 15 + 3 + d, color, textbuffer);
					}
					printbg(40 + 41, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);
				}
				//				if (rhighlight >= 0)
				printbg(40 + 30 + rhighlight, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);
			}
			else
			{
				detailedTableBaseRValue[p] = -1;	// -1 = User can't edit value
				printtext(40 + 41, 15 + 3 + d, color, " --");
			}
		}
		else if (displayRightTable == 2)	// Display command
		{
			sprintf(textbuffer, " %02X", r2);
			printtext(40 + 41, 15 + 3 + d, color, textbuffer);
		}
		else
			printtext(40 + 41, 15 + 3 + d, color, " --");

		UIUnderline = 0;

		printbg(40 + 24 + highlightOffset, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);


		if (editorInfo.etmarknum == 0)
		{
			if (editorInfo.etmarkstart <= editorInfo.etmarkend)
			{
				if ((p >= editorInfo.etmarkstart) && (p <= editorInfo.etmarkend))
				{
					printbg(40 + 38, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					printbg(40 + 38 - 5, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 5);
					printbg(40 + 42, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
				}
			}
			else
			{
				if ((p <= editorInfo.etmarkstart) && (p >= editorInfo.etmarkend))
				{
					printbg(40 + 38, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					printbg(40 + 38 - 5, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 5);
					printbg(40 + 42, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
				}
			}
		}
	}

	displayTable(EDIT_TABLE_SPEED - 1);
	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (!eamode)
		{
			if (editorInfo.etnum < EDIT_TABLE_SPEED - 1)
			{
				editorInfo.cursorX = 43 + 20 + (editorInfo.etcolumn & 1) + editorInfo.etDetailedWaveTableColumn + (editorInfo.etcolumn / 2) * 4;
				printbg(editorInfo.cursorX, 15 + 3 + editorInfo.etpos - editorInfo.etview[editorInfo.etnum], cc << 8, 1);
			}
			else
			{
				editorInfo.cursorX = 43 + 20 + editorInfo.etnum * 10 + (editorInfo.etcolumn & 1) + (editorInfo.etcolumn / 2) * 3;
				printbg(editorInfo.cursorX, 15 + 3 + editorInfo.etpos - editorInfo.etview[editorInfo.etnum], cc << 8, 1);
			}
		}
	}
}




/*
Filtertable left side: 00    Set cutoff, indicated by right side
					   01-7F Filter modulation step. Left side indicates time
							 and right side the speed (signed 8-bit value)
					   80-F0 Set filter parameters. Left side high nybble
							 tells the passband ($90 = lowpass, $A0 = bandpass
							 etc.) and right side tells resonance/channel
							 bitmask, as in command BXY.
					   FF    Jump. Right side tells position ($00 = stop)
*/

void displayDetailedFilterTable(int cc)
{
	int filterEnabledColor[3] = { 0,0,0 };
	int xpos = 40 + 29;


	for (int d = 0; d < VISIBLETABLEROWS; d++)
	{
		int p = editorInfo.etview[FTBL] + d;
		int color = tableBackgroundColors[FTBL][p];
		UIUnderline = 0;


		printtext(xpos + 15, 15 + 3 + d, getColor(CTABLE_FOREGROUND1, CTABLE_BACKGROUND1), "      ");


		int v = ltable[FTBL][p];
		int v2 = v;
		if (v == 0xff)
			UIUnderline = UNDERLINE_FOREGROUND_MASK;

		int r = rtable[FTBL][p];
		int r2 = r;

		sprintf(textbuffer, "%02X:", p + 1);
		printtext(40 + 20, 15 + 3 + d, color, textbuffer);

		printtext(40 + 24, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), "CMFJ");
		int highlightOffset = 0;
		int displayRightTable = 0;


		if (v == 0)	// cutoff
		{
			highlightOffset = 0;

			detailedTableBaseRValue[p] = -1;	// No right value to display
			displayRightTable = 0;

			printtext(xpos, 15 + 3 + d, color, "CUTOFF ");

			v++;
			v2 = r;	// display right value in left column
			detailedTableLValue[p] = r;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xff;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0;		// base value to add to current to get new tableR[p] value

		}
		else if (v >= 1 && v <= 0x7f)	// modify filter cutoff
		{
			highlightOffset = 1;
			detailedTableLValue[p] = v;			// Left value = time
			detailedTableMaxLValue[p] = 0x7f;		// maximum possible value
			detailedTableMinLValue[p] = 1;
			detailedTableBaseLValue[p] = 0;		// base value to add to current to get new tableR[p] value

			printtext(xpos, 15 + 3 + d, color, "FLT MOD");
			displayRightTable = 1;
		}
		else if (v >= 0x80 && v <= 0x0f0)
		{
			// Could display all of this within LEFT display code
			displayRightTable = 2;
			detailedTableRValue[p] = -1;

			highlightOffset = 2;	// Resonance, filter On/Off and filter type.
			printtext(xpos, 15 + 3 + d, color, "FLT SET");


			// Left side low nybble is not used.
			int filterType = (v >> 4) & 0xf;
			int filterResonance = (r >> 4) & 0xf;
			int filterOnOff = r & 0x7;

			v2 = filterResonance;

			// Will need to check ltable[p] to know how to set final value 
			detailedTableLValue[p] = v2;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xf;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			//jp123
			for (int i = 0;i < 3;i++)
			{
				int headerColor = getColor(CTABLE_UNUSED_FOREGROUND, 0);
				if (filterOnOff &(1 << i))
					headerColor = getColor(CCOLOR_RED, 0);
				printbyte(xpos + 11 + i, 15 + 3 + d, headerColor, 0xf3);	// Filter on/off marker

				filterEnabledColor[i] = headerColor;
			}

			for (int j = 0;j < 3;j++)
			{
				int headerColor = getColor(CTABLE_UNUSED_FOREGROUND, 0);
				if (filterType &(1 << j))
					headerColor = getColor(CINFO_FOREGROUND, 0);
				for (int i = 0;i < 2;i++)
				{
					int x = j * 2 + i;
					printbyte(xpos + 15 + x, 15 + 3 + d, headerColor, 0xe0 + (j * 2) + i);
				}
			}

		}
		else if (v >= 0xf1 && v <= 0xfe)
		{
			printtext(xpos, 15 + 3 + d, color, "??? ???");	// invalid command range. 
			detailedTableBaseLValue[p] = -1;
			detailedTableBaseRValue[p] = -1;
			displayRightTable = 0;
		}
		else
		{
			highlightOffset = 3;	// jump
			if (r)
				printtext(xpos, 15 + 3 + d, color, "JUMP:  ");
			else
				printtext(xpos, 15 + 3 + d, color, "STOP:  ");

			displayRightTable = 0;

			detailedTableBaseRValue[p] = -1;

			detailedTableLValue[p] = r;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xff;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			v2 = r;
		}

		if (v != 0)
		{
			sprintf(textbuffer, "%02X", v2);
			printtext(xpos + 8, 15 + 3 + d, color, textbuffer);
		}
		else
			printtext(xpos + 8, 15 + 3 + d, color, "--");


		printbg(40 + 24 + highlightOffset, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);

		if (displayRightTable == 1)	// 1 = display Modulation speed (signed 8bit)
		{
			if (r <= 0x7f)
			{
				detailedTableRValue[p] = r;
				detailedTableMaxRValue[p] = 0x7f;
				detailedTableBaseRValue[p] = 0x00;
				detailedTableMinRValue[p] = 0;

				sprintf(textbuffer, "+%02X", r);
				printtext(xpos + 11, 15 + 3 + d, color, textbuffer);
			}
			else
			{
				r2 = 0x100 - r;
				detailedTableRValue[p] = r2;
				detailedTableMaxRValue[p] = 0x7f;
				detailedTableBaseRValue[p] = 0x00;
				detailedTableMinRValue[p] = 1;

				sprintf(textbuffer, "-%02X", r2);
				printtext(xpos + 11, 15 + 3 + d, color, textbuffer);
			}
			printbg(xpos + 11, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);
		}
		else if (displayRightTable == 2)
		{

		}
		else
			printtext(xpos + 11, 15 + 3 + d, color, " --");

		UIUnderline = 0;


		if (editorInfo.etmarknum == FTBL)
		{
			if (editorInfo.etmarkstart <= editorInfo.etmarkend)
			{
				if ((p >= editorInfo.etmarkstart) && (p <= editorInfo.etmarkend))
				{
					printbg(40 + 37, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					printbg(40 + 37 - 8, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 7);
					if (displayRightTable != 2)
						printbg(40 + 41, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
				}
			}
			else
			{
				if ((p <= editorInfo.etmarkstart) && (p >= editorInfo.etmarkend))
				{
					printbg(40 + 37, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					printbg(40 + 37 - 8, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 7);
					if (displayRightTable != 2)
						printbg(40 + 41, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
				}
			}
		}

	}

	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (!eamode)
		{
			editorInfo.cursorX = xpos + (editorInfo.etcolumn & 1) + editorInfo.etDetailedWaveTableColumn + (editorInfo.etcolumn / 2) * 4;
			int ctype = 0;
			if (ltable[FTBL][editorInfo.etpos] < 0x80 || ltable[FTBL][editorInfo.etpos] == 0xff)
				ctype = 1;
			else if (ltable[FTBL][editorInfo.etpos] >= 0x80 && ltable[FTBL][editorInfo.etpos] <= 0xfe && editorInfo.etcolumn < 2)
				ctype = 1;

			if (ctype == 1)
			{


				printbg(editorInfo.cursorX, 15 + 3 + editorInfo.etpos - editorInfo.etview[editorInfo.etnum], cc << 8, 1);
			}
			else
			{
				int f = editorInfo.cursorX - (xpos + 11);

				int filterOnOff = rtable[FTBL][editorInfo.etpos];
				filterOnOff &= 0x7;

				cc <<= 8;
				cc &= 0xff00;
				if (filterOnOff&(1 << f))
					cc |= CCOLOR_RED;
			}
		}
	}
}

/*
Pulsetable left side:  01-7F Pulse modulation step. Left side indicates time
							 and right side the speed (signed 8-bit value).
					   8X-FX Set pulse width. X is the high 4 bits, right
							 side tells the 8 low bits.
					   FF    Jump. Right side tells position ($00 = stop)

*/
void displayDetailedPulseTable(int cc)
{

	int xpos = 40 + 29;


	for (int d = 0; d < VISIBLETABLEROWS; d++)
	{
		int p = editorInfo.etview[PTBL] + d;
		int color = tableBackgroundColors[PTBL][p];
		UIUnderline = 0;


		printtext(xpos + 15, 15 + 3 + d, getColor(CTABLE_FOREGROUND1, CTABLE_BACKGROUND1), "      ");


		int v = ltable[PTBL][p];
		int v2 = v;
		if (v == 0xff)
			UIUnderline = UNDERLINE_FOREGROUND_MASK;

		int r = rtable[PTBL][p];
		int r2 = r;

		sprintf(textbuffer, "%02X:", p + 1);
		printtext(40 + 20, 15 + 3 + d, color, textbuffer);

		printtext(40 + 24, 15 + 3 + d, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND), "SMJ");
		int highlightOffset = 0;
		int displayRightTable = 0;


		if (v >= 1 && v <= 0x7f)	// modify pulse width
		{
			highlightOffset = 1;
			detailedTableLValue[p] = v;			// Left value = time
			detailedTableMaxLValue[p] = 0x7f;		// maximum possible value
			detailedTableMinLValue[p] = 1;
			detailedTableBaseLValue[p] = 0;		// base value to add to current to get new tableR[p] value
			displayRightTable = 1;
			printtext(xpos, 15 + 3 + d, color, "PLS MOD");
		}
		else if (v >= 0x80 && v <= 0x0fe)
		{
			displayRightTable = 0;
			detailedTableRValue[p] = -1;

			highlightOffset = 0;	// Resonance, filter On/Off and filter type.
			printtext(xpos, 15 + 3 + d, color, "PLS SET");

			v2 &= 0xf;
			v2 <<= 8;
			v2 |= r;
			sprintf(textbuffer, "%03X", v2);
			printtext(xpos + 8, 15 + 3 + d, color, textbuffer);
			v = -1;	// don't display anything else for left column


			// Will need to check ltable[p] to know how to set final value 
			detailedTableLValue[p] = v2;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0x3ff;		// 0x3ff -- ignored anyway. maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)


		}
		else if (v == 0xff)
		{
			highlightOffset = 2;	// jump
			if (r)
				printtext(xpos, 15 + 3 + d, color, "JUMP:  ");
			else
				printtext(xpos, 15 + 3 + d, color, "STOP:  ");

			displayRightTable = 0;

			detailedTableBaseRValue[p] = -1;

			detailedTableLValue[p] = r;			// current L value. Modify this with HexInput
			detailedTableMaxLValue[p] = 0xff;		// maximum possible value (min is always 0)
			detailedTableMinLValue[p] = 0;
			detailedTableBaseLValue[p] = 0x0;		// base value to add to current to get new tableL[p] value (will need to check ltable[p] initially to see if we need to massage data further)

			v2 = r;
		}
		else
		{
			printtext(xpos, 15 + 3 + d, color, "??? ???");	// invalid command range. 
			detailedTableBaseLValue[p] = -1;
			detailedTableBaseRValue[p] = -1;
			displayRightTable = 0;
			v = 0;
		}


		if (v > 0)
		{
			sprintf(textbuffer, " %02X", v2);
			printtext(xpos + 8, 15 + 3 + d, color, textbuffer);
		}
		else if (v == 0)
			printtext(xpos + 8, 15 + 3 + d, color, " --");


		printbg(40 + 24 + highlightOffset, 15 + 3 + d, getColor(0xe, CTABLE_UNUSED_BACKGROUND), 1);


		if (displayRightTable == 1)	// 1 = display Modulation speed (signed 8bit)
		{
			if (r <= 0x7f)
			{
				detailedTableRValue[p] = r;
				detailedTableMaxRValue[p] = 0x7f;
				detailedTableBaseRValue[p] = 0x00;
				detailedTableMinRValue[p] = 0;

				sprintf(textbuffer, "+%02X", r);
				printtext(xpos + 12, 15 + 3 + d, color, textbuffer);
			}
			else
			{
				r2 = 0x100 - r;
				detailedTableRValue[p] = r2;
				detailedTableMaxRValue[p] = 0x7f;
				detailedTableBaseRValue[p] = 0x00;
				detailedTableMinRValue[p] = 1;

				sprintf(textbuffer, "-%02X", r2);
				printtext(xpos + 12, 15 + 3 + d, color, textbuffer);
			}

		}
		else //if (v >= 0)
		{
			int ut = UIUnderline;
			UIUnderline = 0;
			printtext(xpos + 12, 15 + 3 + d, color, " --");
			UIUnderline = ut;
		}
		printbg(xpos + 12, 15 + 3 + d, getColor(0xe, 0), 1);

		UIUnderline = 0;


		if (editorInfo.etmarknum == PTBL)
		{
			if (editorInfo.etmarkstart <= editorInfo.etmarkend)
			{
				if ((p >= editorInfo.etmarkstart) && (p <= editorInfo.etmarkend))
				{
					printbg(40 + 37, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 3);

					printbg(40 + 42, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);

					printbg(40 + 37 - 8, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 7);
				}
			}
			else
			{
				if ((p <= editorInfo.etmarkstart) && (p >= editorInfo.etmarkend))
				{
					printbg(40 + 37, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 3);

					printbg(40 + 42, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 2);
					printbg(40 + 37 - 8, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 7);
				}
			}
		}

	}

	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (!eamode)
		{
			if (editorInfo.etcolumn <= 2)
			{
				if (ltable[PTBL][editorInfo.etpos] < 0x80 || ltable[PTBL][editorInfo.etpos] >0xfe)
				{
					if (editorInfo.etcolumn == 0)
						editorInfo.etcolumn = 1;
				}
				editorInfo.cursorX = xpos + (editorInfo.etcolumn % 3) + editorInfo.etDetailedWaveTableColumn;
			}
			else
				editorInfo.cursorX = xpos + (1 + editorInfo.etcolumn & 1) + editorInfo.etDetailedWaveTableColumn + (editorInfo.etcolumn / 3) * 5;
			int ctype = 1;	// was 0

			if (ctype == 1)
			{
				printbg(editorInfo.cursorX, 15 + 3 + editorInfo.etpos - editorInfo.etview[editorInfo.etnum], cc << 8, 1);
			}
		}
	}
}




void displayOriginalTableView(int cc)
{
	for (int c = 0; c < MAX_TABLES; c++)
	{
		displayTable(c);
	}

	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (!eamode)
			printbg(43 + 20 + editorInfo.etnum * 10 + (editorInfo.etcolumn & 1) + (editorInfo.etcolumn / 2) * 3, 15 + 3 + editorInfo.etpos - editorInfo.etview[editorInfo.etnum], cc << 8, 1);
	}
}


void displayTable(int c)
{
	for (int d = 0; d < VISIBLETABLEROWS; d++)
	{
		int p = editorInfo.etview[c] + d;

		int color = tableBackgroundColors[c][p];

		UIUnderline = 0;
		if ((c != 3 && ltable[c][p] == 0xff))
			UIUnderline = UNDERLINE_FOREGROUND_MASK;


		if ((p == editorInfo.etpos) && (editorInfo.etnum == c))
			color = CEDIT;


		sprintf(textbuffer, "%02X:%02X %02X", p + 1, ltable[c][p], rtable[c][p]);
		printtext(40 + 20 + 10 * c, 15 + 3 + d, color, textbuffer);

		if (c < 3)
		{
			printtext(40 + 20 + (10 * (c + 1)) - 2, 15 + 3 + d, getColor(0, CTABLE_UNUSED_BACKGROUND), "  ");
		}

		if (editorInfo.etmarknum == c)
		{
			if (editorInfo.etmarkstart <= editorInfo.etmarkend)
			{
				if ((p >= editorInfo.etmarkstart) && (p <= editorInfo.etmarkend))
					printbg(40 + 20 + 10 * c + 3, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 5);
			}
			else
			{
				if ((p <= editorInfo.etmarkstart) && (p >= editorInfo.etmarkend))
					printbg(40 + 20 + 10 * c + 3, 15 + 3 + d, getColor(CSELECT_TO_COPY_FOREGROUND, CSELECT_TO_COPY_BACKGROUND), 5);
			}
		}
	}
}



int getWaveforumColour(int bit, int value)
{
	int cOFF = getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND);
	int cON = getColor(0xe, CTABLE_UNUSED_BACKGROUND);

	if (value & bit)
		return cON;
	return cOFF;
}

void displayWaveformInfo(int x, int y)
{

	if (!waveformDisplayInfo.displayOnOff)
		return;

	int cdivider = getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND);

	int xs = 0;
	for (int xp = 0;xp < 9;xp++)
	{
		xs = x + (xp * 5);
		printbyte(xs, y, cdivider, 0xff);
	}


	for (int xp = 0;xp < 4;xp++)
	{
		int c = getWaveforumColour(0x80 >> xp, waveformDisplayInfo.value);

		xs = x + (xp * 5);
		printbyterow(xs + 1, y, c, 0x18 + xp, 4);
	}
	xs += 6;


	int c = getWaveforumColour(0x8, waveformDisplayInfo.value);
	printtext(xs, y, c, "TEST");
	xs += 5;
	c = getWaveforumColour(0x4, waveformDisplayInfo.value);
	printtext(xs, y, c, "RING");
	xs += 5;
	c = getWaveforumColour(0x2, waveformDisplayInfo.value);
	printtext(xs, y, c, "SYNC");
	xs += 5;
	c = getWaveforumColour(0x1, waveformDisplayInfo.value);
	printtext(xs, y, c, "GATE");
}








