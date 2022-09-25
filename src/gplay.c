//
// GTUltra Play routine (currently a modifieid GOATTRACKER v2 playroutine)
//

#define GPLAY_C

#include "goattrk2.h"

//#define JP_NEW_FEATURES

unsigned char freqtbllo[] = {
  0x17,0x27,0x39,0x4b,0x5f,0x74,0x8a,0xa1,0xba,0xd4,0xf0,0x0e,
  0x2d,0x4e,0x71,0x96,0xbe,0xe8,0x14,0x43,0x74,0xa9,0xe1,0x1c,
  0x5a,0x9c,0xe2,0x2d,0x7c,0xcf,0x28,0x85,0xe8,0x52,0xc1,0x37,
  0xb4,0x39,0xc5,0x5a,0xf7,0x9e,0x4f,0x0a,0xd1,0xa3,0x82,0x6e,
  0x68,0x71,0x8a,0xb3,0xee,0x3c,0x9e,0x15,0xa2,0x46,0x04,0xdc,
  0xd0,0xe2,0x14,0x67,0xdd,0x79,0x3c,0x29,0x44,0x8d,0x08,0xb8,
  0xa1,0xc5,0x28,0xcd,0xba,0xf1,0x78,0x53,0x87,0x1a,0x10,0x71,
  0x42,0x89,0x4f,0x9b,0x74,0xe2,0xf0,0xa6,0x0e,0x33,0x20,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

unsigned char freqtblhi[] = {
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x04,
  0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
  0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1f,0x20,
  0x22,0x24,0x27,0x29,0x2b,0x2e,0x31,0x34,0x37,0x3a,0x3e,0x41,
  0x45,0x49,0x4e,0x52,0x57,0x5c,0x62,0x68,0x6e,0x75,0x7c,0x83,
  0x8b,0x93,0x9c,0xa5,0xaf,0xb9,0xc4,0xd0,0xdd,0xea,0xf8,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

int getClosestNote(int freq)
{
	int diff = 9999999;
	int closestNote = 9999;
	for (int i = 0;i < 96;i++)
	{
		int f = (freqtblhi[i] << 8) + freqtbllo[i];
		int d = freq - f;
		if (d < 0)
			d = -d;
		if (d > diff)
			return closestNote;
		diff = d;
		closestNote = i;
	}
	return closestNote;
}


GTOBJECT gtObject;
GTOBJECT gtEditorObject;
GTOBJECT gtLoopObject;
GTOBJECT gtEditorLoopObject;

int jpd = 0;
int releasetimes[16] = { 6, 24, 48, 72, 114, 168, 204, 240, 300, 750, 1500, 2400, 3000, 9000, 15000, 24000 };


void sequencer(int c, CHN *cptr, GTOBJECT *gt);

void initchannels(GTOBJECT *gt)
{
	int c;
	CHN *cptr = &gt->chn[0];

	memset(gt->chn, 0, sizeof gt->chn);

	for (c = 0; c < MAX_PLAY_CH; c++)
	{
		gt->chn[c].trans = 0;
		gt->chn[c].instr = 1;
		if (multiplier)
			cptr->tempo = 6 * multiplier - 1;
		else
			cptr->tempo = 6 - 1;
		cptr++;
	}

	if (multiplier)
	{
		gt->funktable[0] = 9 * multiplier - 1;
		gt->funktable[1] = 6 * multiplier - 1;
	}
	else
	{
		gt->funktable[0] = 9 - 1;
		gt->funktable[1] = 6 - 1;
	}
}

void initsong(int num, int mode, GTOBJECT *gt)
{
	int loopChannel = 0;
	if (editorInfo.editmode == EDIT_PATTERN)
		loopChannel = editorInfo.epchn;
	else if (editorInfo.editmode == EDIT_ORDERLIST)
		loopChannel = editorInfo.eschn;

	loopChannel = getActualChannel(editorInfo.esnum, loopChannel);	// 1.1.7 FIX (need 0-11 for masterloop) 3/5/2022
	gt->masterLoopChannel = loopChannel;
	gt->masterLoopSubSong = editorInfo.esnum;

	gt->loopEnabledFlag = 0;

	sound_suspend();
	gt->songinit = PLAY_STOPPED;
	gt->psnum = num;
	gt->songinit = mode;
	gt->disableLoopSearch = 0;
	gt->startpattpos = 0;

	for (int i = 0;i < MAX_PLAY_CH;i++)
	{
		gt->loopEndChn[i].pattptr = -1;
	}

	sound_flush();
}

void initsongpos(int num, int mode, int pattpos, GTOBJECT *gt)
{
	gt->loopEnabledFlag = 0;

	sound_suspend();
	gt->songinit = PLAY_STOPPED;
	gt->psnum = num;
	gt->songinit = mode;
	gt->startpattpos = pattpos;
	gt->disableLoopSearch = 0;
	sound_flush();
}

void stopsong(GTOBJECT *gt)
{
	// JP removed this. Allows F4 to stop jamming notes with long release
//	if (gt->songinit != PLAY_STOPPED)
//	{
	sound_suspend();
	gt->songinit = PLAY_STOP;
	sound_flush();

	for (int c = 0; c < MAX_PLAY_CH; c++)
	{
		gt->chn[c].instr = 0;
		gt->chn[c].releaseTime = 0;
	}
	//	}
}

void rewindsong(GTOBJECT *gt)
{
	if (gt->lastsonginit == PLAY_BEGINNING)
		gt->lastsonginit = PLAY_POS;
	initsong(gt->psnum, gt->lastsonginit, gt);
}

void playtestnote(int note, int ins, int chnnum, GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, chnnum);	// 0-12

	c2 = chnnum;


	if (note == KEYON) return;
	if ((note == REST) || (note == KEYOFF))
	{
		releasenote(chnnum, gt);
		return;
	}


	if (!(instr[ins].gatetimer & 0x40))
	{
		gt->chn[c2].gate = 0xfe; // Keyoff
	//	gt->chn[c2].gate = 0xff;
		if (!(instr[ins].gatetimer & 0x80))
		{
			if (!gt->noSIDWrites)
			{
				int i = c2 / 3;
				gt->sidreg[i][0x5 + (c2 % 3) * 7] = (adparam >> 8); // Hardrestart
				gt->sidreg[i][0x6 + (c2 % 3) * 7] = (adparam & 0xff);

			}
		}
	}

	gt->chn[c2].requestKeyOff = 0xff;
	gt->chn[c2].instr = ins;
	gt->chn[c2].newnote = note;
	gt->chn[c2].pan = 0x7;

	if (gt->songinit == PLAY_STOPPED)
	{
		gt->chn[c2].releaseTime = 0;
		gt->chn[c2].tick = (instr[ins].gatetimer & 0x3f) + 1;
		gt->chn[c2].gatetimer = instr[ins].gatetimer & 0x3f;
	}
}

void releasenote(int chnnum, GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, chnnum);	// 0-12
	gt->chn[c2].gate = 0xfe;
	gt->chn[c2].requestKeyOff = 0xfe;
}

void mutechannel(int chnnum, GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, chnnum);	// 0-12

	gt->chn[c2].mute ^= 1;
}

int isplaying(GTOBJECT *gt)
{
	return (gt->songinit != PLAY_STOPPED);
}

void playroutine(GTOBJECT *gt)
{
	INSTR *iptr;
	CHN *cptr = &gt->chn[0];
	int c;

	if ((gt->songinit > 0) && (gt->songinit < PLAY_STOPPED))
	{
		gt->lastsonginit = gt->songinit;

		for (int i = 0;i < 4;i++)
		{
			gt->filterInfo[i].filterctrl = 0;
			gt->filterInfo[i].filterptr = 0;
		}

		resettime(gt);

		if ((gt->songinit == 0x02) || (gt->songinit == 0x03))
		{
			for (c = 0; c < maxSIDChannels; c++)
			{
				int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
				int songNum = getActualSongNumber(editorInfo.esnum, c2);
				int c3 = c % 6;


				if (editorInfo.expandOrderListView == 0)
				{
					if (gt->editorUndoInfo.editorInfo[c2].espos >= songlen[songNum][c3])
						gt->songinit = 0x01;
				}
				else
				{
					if (gt->editorUndoInfo.editorInfo[c2].espos >= songOrderLength[songNum][c3])
						gt->songinit = 0x01;
				}

			}
		}

		for (c = 0; c < MAX_PLAY_CH; c++)
		{
			
			cptr->instr = 0;
			cptr->nextPatternTriggered = 0;
			cptr->releaseTime = 0;
			cptr->requestKeyOff = 0xff;
			cptr->portCounter = 0;
			cptr->songptr = 0;
			cptr->command = 0;
			cptr->cmddata = 0;
			cptr->newcommand = 0;
			cptr->newcmddata = 0;
			cptr->advance = 1;
			cptr->wave = 0;
			cptr->loopCount = 0;
			cptr->ptr[WTBL] = 0;
			cptr->newnote = 0;
			cptr->repeat = 0;
			if (multiplier)
				cptr->tick = 6 * multiplier - 1;
			else
				cptr->tick = 6 - 1;
			cptr->gatetimer = instr[1].gatetimer & 0x3f;
			cptr->pattptr = 0x7fffffff;

			cptr->lastpattptr = cptr->pattptr;
			cptr->lastsongptr = cptr->songptr;

			if (cptr->tempo < 2) cptr->tempo = 0;

			if (c < maxSIDChannels)
			{
				switch (gt->songinit)
				{
				case PLAY_BEGINNING:
					if (multiplier)
					{
						gt->funktable[0] = 9 * multiplier - 1;
						gt->funktable[1] = 6 * multiplier - 1;
						cptr->tempo = 6 * multiplier - 1;
					}
					else
					{
						gt->funktable[0] = 9 - 1;
						gt->funktable[1] = 6 - 1;
						cptr->tempo = 6 - 1;
					}
					if ((instr[MAX_INSTR - 1].ad >= 2) && (!(instr[MAX_INSTR - 1].ptr[WTBL])))
						cptr->tempo = instr[MAX_INSTR - 1].ad - 1;
					cptr->trans = 0;
					cptr->instr = 1;
					sequencer(c, cptr, gt);
					break;

				case PLAY_PATTERN:
					cptr->advance = 0;
					cptr->pattptr = gt->startpattpos * 4;
					cptr->pattnum = gt->editorUndoInfo.editorInfo[c].epnum;
					if (cptr->pattptr >= (pattlen[cptr->pattnum] * 4))
						cptr->pattptr = 0;
					break;

				case PLAY_POS:
					cptr->songptr = gt->editorUndoInfo.editorInfo[c].espos;
					sequencer(c, cptr, gt);
					break;
				}
			}
			cptr++;
		}
		if (gt->songinit != PLAY_STOP)
			gt->songinit = 0;
		else
			gt->songinit = PLAY_STOPPED;

		int baseSong = getActualSongNumber(gt->psnum, 0);

		for (int i = 0;i < maxSIDChannels;i++)
		{

			int j = i / 6;
			int k = i % 6;

			if (editorInfo.expandOrderListView == 0)
			{
				if (!songlen[baseSong + j][k])
				{
					gt->songinit = PLAY_STOPPED; // Zero length song
					break;
				}
			}
			else
			{
				if (!songOrderLength[baseSong + j][k])
				{
					gt->songinit = PLAY_STOPPED; // Zero length song
					break;
				}
			}
		}

		gt->startpattpos = 0;


		return;	// JP Added this. Just to make sure nothing futher causes problems 22Aug2022
	}
	else
	{

		for (int i = 0;i < 4;i++)
		{
			if (gt->filterInfo[i].filterptr)
			{
				// Filter jump
				if (ltable[FTBL][gt->filterInfo[i].filterptr - 1] == 0xff)
				{
					gt->filterInfo[i].filterptr = rtable[FTBL][gt->filterInfo[i].filterptr - 1];
					if (!gt->filterInfo[i].filterptr) goto FILTERSTOP;
				}

				if (!gt->filterInfo[i].filtertime)
				{
					// Filter set
					if (ltable[FTBL][gt->filterInfo[i].filterptr - 1] >= 0x80)
					{
						gt->filterInfo[i].filtertype = ltable[FTBL][gt->filterInfo[i].filterptr - 1] & 0x70;
						gt->filterInfo[i].filterctrl = rtable[FTBL][gt->filterInfo[i].filterptr - 1];
						gt->filterInfo[i].filterptr++;
						// Can be combined with cutoff set
						if (ltable[FTBL][gt->filterInfo[i].filterptr - 1] == 0x00)
						{
							gt->filterInfo[i].filtercutoff = rtable[FTBL][gt->filterInfo[i].filterptr - 1];
							gt->filterInfo[i].filterptr++;
						}
					}
					else
					{
						// New modulation step
						if (ltable[FTBL][gt->filterInfo[i].filterptr - 1])
							gt->filterInfo[i].filtertime = ltable[FTBL][gt->filterInfo[i].filterptr - 1];
						else
						{
							// Cutoff set
							gt->filterInfo[i].filtercutoff = rtable[FTBL][gt->filterInfo[i].filterptr - 1];
							gt->filterInfo[i].filterptr++;
						}
					}
				}
				// Filter modulation
				if (gt->filterInfo[i].filtertime)
				{
					gt->filterInfo[i].filtercutoff += rtable[FTBL][gt->filterInfo[i].filterptr - 1];
					gt->filterInfo[i].filtertime--;
					if (!gt->filterInfo[i].filtertime) gt->filterInfo[i].filterptr++;
				}
			}
		FILTERSTOP:

			if (!gt->noSIDWrites)
			{
				gt->sidreg[i][0x15] = 0x00;
				gt->sidreg[i][0x16] = gt->filterInfo[i].filtercutoff;
				gt->sidreg[i][0x17] = gt->filterInfo[i].filterctrl;
				gt->sidreg[i][0x18] = gt->filterInfo[i].filtertype | gt->masterfader;
			}
		}


		for (c = 0; c < MAX_PLAY_CH; c++)
		{
			int sidIndex = c / 3;

			iptr = &instr[cptr->instr];


			// JP Check for max playing SID count here!
			// (we process 12 channels to allow for poly mode, but only want to process, say, 3 channels for playing song)

			// Reset tempo in jammode
			// JP 22Aug2022 - Fix .. Only had one & 
			if ((gt->songinit == PLAY_STOPPED && cptr->tempo < 2) || c >= maxSIDChannels)// JP FEB 17 added maxSIDChannel check
			{
				if (multiplier)
					cptr->tempo = 6 * multiplier - 1;
				else
					cptr->tempo = 6 - 1;
			}

			// Decrease tick

			cptr->tick--;
			if (!cptr->tick)
				goto TICK0;

			// Tick N
			// Reload counter
			if (cptr->tick >= 0x80)		// gone negative
			{
				if (cptr->tempo >= 2)
					cptr->tick = cptr->tempo;
				else
				{
					// Set funktempo, switch between 2 values
					cptr->tick = gt->funktable[cptr->tempo];
					cptr->tempo ^= 1;
				}
				// Check for illegally high gatetimer and stop the song in this case
				if (gt->chn[c].gatetimer > cptr->tick)		// JP FIXED THE CODE IN THIS LINE
					stopsong(gt);
			}

			cptr->portCounter++;

			goto WAVEEXEC;

			// Tick 0
		TICK0:

			cptr->portCounter++;
			// Advance in sequencer
			sequencer(c, cptr, gt);

			// Get gatetimer compare-value
			cptr->gatetimer = iptr->gatetimer & 0x3f;

			// New note init
			if (cptr->newnote)
			{
				cptr->note = cptr->newnote - FIRSTNOTE;
				cptr->command = 0;
				cptr->vibdelay = iptr->vibdelay;
				cptr->cmddata = iptr->ptr[STBL];
				if (cptr->newcommand != CMD_TONEPORTA)
				{
					if (iptr->firstwave)
					{
						if (iptr->firstwave >= 0xfe)
							cptr->gate = iptr->firstwave;
						else
						{
							cptr->wave = iptr->firstwave;
							cptr->gate = 0xff;
						}
					}

					cptr->ptr[WTBL] = iptr->ptr[WTBL];
					int n = cptr->note & 0x7f;
					cptr->freq = freqtbllo[n] | (freqtblhi[n] << 8);

					if (cptr->ptr[WTBL])
					{
						// Stop the song in case of jumping into a jump
						if (ltable[WTBL][cptr->ptr[WTBL] - 1] == 0xff)
							stopsong(gt);
					}
					if (iptr->ptr[PTBL])
					{
						cptr->ptr[PTBL] = iptr->ptr[PTBL];
						cptr->pulsetime = 0;
						if (cptr->ptr[PTBL])
						{
							// Stop the song in case of jumping into a jump
							if (ltable[PTBL][cptr->ptr[PTBL] - 1] == 0xff)
								stopsong(gt);
						}
					}
					if (iptr->ptr[FTBL])
					{
						//	int i = c / 3;	// 0-3	// JP - Removed and replaced with sidIndex 22Aug2022
						gt->filterInfo[sidIndex].filterptr = iptr->ptr[FTBL];
						gt->filterInfo[sidIndex].filtertime = 0;
						if (gt->filterInfo[sidIndex].filterptr)
						{
							// Stop the song in case of jumping into a jump
							if (ltable[FTBL][gt->filterInfo[sidIndex].filterptr - 1] == 0xff)
								stopsong(gt);
						}

					}
					if (!gt->noSIDWrites)
					{
						gt->sidreg[sidIndex][0x5 + 7 * (c % 3)] = iptr->ad;
						gt->sidreg[sidIndex][0x6 + 7 * (c % 3)] = iptr->sr;
					}

					int rIndex = iptr->sr & 0xf;
					cptr->releaseTime = releasetimes[rIndex];
				}
			}

			// Tick 0 effects

			switch (cptr->newcommand)
			{
			case CMD_DONOTHING:
				cptr->command = 0;
				cptr->cmddata = iptr->ptr[STBL];
				break;

			case CMD_PORTAUP:
			case CMD_PORTADOWN:
				cptr->vibtime = 0;
				cptr->command = cptr->newcommand;
				cptr->cmddata = cptr->newcmddata;
				break;

			case CMD_TONEPORTA:
			case CMD_VIBRATO:
				cptr->command = cptr->newcommand;
				cptr->cmddata = cptr->newcmddata;
				break;

			case CMD_SETAD:
				if (!gt->noSIDWrites)
				{
					gt->sidreg[sidIndex][0x5 + 7 * (c % 3)] = cptr->newcmddata;
				}
				break;

			case CMD_SETSR:
				if (!gt->noSIDWrites)
				{
					gt->sidreg[sidIndex][0x6 + 7 * (c % 3)] = cptr->newcmddata;
					break;
				}

			case CMD_SETWAVE:
				cptr->wave = cptr->newcmddata;
				break;

			case CMD_SETWAVEPTR:
				cptr->ptr[WTBL] = cptr->newcmddata;
				cptr->wavetime = 0;
				if (cptr->ptr[WTBL])
				{
					// Stop the song in case of jumping into a jump
					if (ltable[WTBL][cptr->ptr[WTBL] - 1] == 0xff)
						stopsong(gt);
				}
				break;

			case CMD_SETPULSEPTR:
				cptr->ptr[PTBL] = cptr->newcmddata;
				cptr->pulsetime = 0;
				if (cptr->ptr[PTBL])
				{
					// Stop the song in case of jumping into a jump
					if (ltable[PTBL][cptr->ptr[PTBL] - 1] == 0xff)
						stopsong(gt);
				}
				break;

			case CMD_SETFILTERPTR:
				gt->filterInfo[sidIndex].filterptr = cptr->newcmddata;
				gt->filterInfo[sidIndex].filtertime = 0;
				if (gt->filterInfo[sidIndex].filterptr)
				{
					// Stop the song in case of jumping into a jump
					if (ltable[FTBL][gt->filterInfo[sidIndex].filterptr - 1] == 0xff)
						stopsong(gt);
				}

				break;

			case CMD_SETFILTERCTRL:
				gt->filterInfo[sidIndex].filterctrl = cptr->newcmddata;
				if (!gt->filterInfo[sidIndex].filterctrl)
					gt->filterInfo[sidIndex].filterptr = 0;
				break;

			case CMD_SETFILTERCUTOFF:
				gt->filterInfo[sidIndex].filtercutoff = cptr->newcmddata;

				break;

			case CMD_SETMASTERVOL:
				if (cptr->newcmddata < 0x10)
					gt->masterfader = cptr->newcmddata;
				break;

			case CMD_FUNKTEMPO:
				if (cptr->newcmddata)
				{
					gt->funktable[0] = ltable[STBL][cptr->newcmddata - 1] - 1;
					gt->funktable[1] = rtable[STBL][cptr->newcmddata - 1] - 1;
				}
				{
					int d;
					for (d = 0; d < maxSIDChannels; d++)
						gt->chn[d].tempo = 0;
				}
				break;

			case CMD_SETTEMPO:
			{
				unsigned char newtempo = cptr->newcmddata & 0x7f;

				if (newtempo >= 3)
					newtempo--;
				if (cptr->newcmddata >= 0x80)
					cptr->tempo = newtempo;
				else
				{
					int d;
					for (d = 0; d < maxSIDChannels; d++)
						gt->chn[d].tempo = newtempo;
				}
			}
			break;

			}
			if (cptr->newnote)
			{
				cptr->newnote = 0;
				if (cptr->newcommand != CMD_TONEPORTA) goto NEXTCHN;
			}

		WAVEEXEC:

			if (cptr->ptr[WTBL])
			{
				unsigned char wave = ltable[WTBL][cptr->ptr[WTBL] - 1];
				unsigned char note = rtable[WTBL][cptr->ptr[WTBL] - 1];

				if (wave > WAVELASTDELAY)
				{
					// Normal waveform values
					if (wave < WAVESILENT) cptr->wave = wave;	// | 1;
					// Values without waveform selected
					if ((wave >= WAVESILENT) && (wave <= WAVELASTSILENT)) cptr->wave = wave & 0xf;
					// Command execution from wavetable
					if ((wave >= WAVECMD) && (wave <= WAVELASTCMD))
					{
						unsigned char param = rtable[WTBL][cptr->ptr[WTBL] - 1];
						switch (wave & 0xf)
						{

						case CMD_DONOTHING:
#ifdef JP_NEW_FEATURES
							// JP - GTUltra Special command $F0.
							// Data byte dictates function (save pitch / restore pitch...)

							if (param < 3)
							{
								cptr->ptr[WTBL]++;
								// Wavetable jump
								if (ltable[WTBL][cptr->ptr[WTBL] - 1] == 0xff)
								{
									cptr->ptr[WTBL] = rtable[WTBL][cptr->ptr[WTBL] - 1];
								}

								if (param == 1)
								{
									if (jpd < 30)
									{
										if (c == 0)
										{
											//	sprintf(textbuffer, "+ %x", cptr->freq);
											//	printtext(60 + jpd, 36, 0xe, textbuffer);
											jpd += 5;
											jpd %= 30;
										}
									}

									cptr->freqBackup = cptr->freq;

								}
								else if (param == 2)
								{

									cptr->freq = cptr->freqBackup;
								}
								goto WAVEEXEC;
							}
#else
							stopsong(gt);
#endif
							break;

						case CMD_SETWAVEPTR:
						case CMD_FUNKTEMPO:
							stopsong(gt);
							break;

						case CMD_PORTAUP:
						{
							unsigned short speed = 0;
							if (param)
							{
								speed = (ltable[STBL][param - 1] << 8) | rtable[STBL][param - 1];
							}
							if (speed >= 0x8000)
							{
								speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
								speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
								speed >>= rtable[STBL][param - 1];
							}
							cptr->freq += speed;
						}
						break;

						case CMD_PORTADOWN:
						{
							unsigned short speed = 0;
							if (param)
							{
								speed = (ltable[STBL][param - 1] << 8) | rtable[STBL][param - 1];
							}
							if (speed >= 0x8000)
							{
								speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
								speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
								speed >>= rtable[STBL][param - 1];
							}
							cptr->freq -= speed;
						}
						break;

						case CMD_TONEPORTA:
						{
							unsigned short targetfreq = freqtbllo[cptr->note] | (freqtblhi[cptr->note] << 8);
							unsigned short speed = 0;

							if (!param)
							{
								cptr->freq = targetfreq;
								cptr->vibtime = 0;
							}
							else
							{
								speed = (ltable[STBL][param - 1] << 8) | rtable[STBL][param - 1];
								if (speed >= 0x8000)
								{
									speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
									speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
									speed >>= rtable[STBL][param - 1];
								}
								if (cptr->freq < targetfreq)
								{
									cptr->freq += speed;
									if (cptr->freq > targetfreq)
									{
										cptr->freq = targetfreq;
										cptr->vibtime = 0;
									}
								}
								if (cptr->freq > targetfreq)
								{
									cptr->freq -= speed;
									if (cptr->freq < targetfreq)
									{
										cptr->freq = targetfreq;
										cptr->vibtime = 0;
									}
								}
							}
						}
						break;

						case CMD_VIBRATO:
						{
							unsigned short speed = 0;
							unsigned char cmpvalue = 0;

							if (param)
							{
								cmpvalue = ltable[STBL][param - 1];
								speed = rtable[STBL][param - 1];
							}
							if (cmpvalue >= 0x80)
							{
								cmpvalue &= 0x7f;
								speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
								speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
								speed >>= rtable[STBL][param - 1];
							}

							if ((cptr->vibtime < 0x80) && (cptr->vibtime > cmpvalue))
								cptr->vibtime ^= 0xff;
							cptr->vibtime += 0x02;
							if (cptr->vibtime & 0x01)
								cptr->freq -= speed;
							else
								cptr->freq += speed;
						}
						break;

						case CMD_SETAD:
							if (!gt->noSIDWrites)
							{
								gt->sidreg[sidIndex][0x5 + 7 * (c % 3)] = param;
							}
							break;

						case CMD_SETSR:
							if (!gt->noSIDWrites)
							{
								gt->sidreg[sidIndex][0x6 + 7 * (c % 3)] = param;
							}
							break;

						case CMD_SETWAVE:
							cptr->wave = param;
							break;

						case CMD_SETPULSEPTR:
							cptr->ptr[PTBL] = param;
							cptr->pulsetime = 0;
							if (cptr->ptr[PTBL])
							{
								// Stop the song in case of jumping into a jump
								if (ltable[PTBL][cptr->ptr[PTBL] - 1] == 0xff)
									stopsong(gt);
							}
							break;

						case CMD_SETFILTERPTR:
							gt->filterInfo[sidIndex].filterptr = param;
							gt->filterInfo[sidIndex].filtertime = 0;
							if (gt->filterInfo[sidIndex].filterptr)
							{
								// Stop the song in case of jumping into a jump
								if (ltable[FTBL][gt->filterInfo[sidIndex].filterptr - 1] == 0xff)
									stopsong(gt);
							}
							break;

						case CMD_SETFILTERCTRL:
							gt->filterInfo[sidIndex].filterctrl = param;
							if (!gt->filterInfo[sidIndex].filterctrl)
								gt->filterInfo[sidIndex].filterptr = 0;
							break;

						case CMD_SETFILTERCUTOFF:
							gt->filterInfo[sidIndex].filtercutoff = param;

							break;

						case CMD_SETMASTERVOL:
							if (param < 0x10)
								gt->masterfader = param;
							break;
						}
					}
				}
				else
				{
					// Wavetable delay
					if (cptr->wavetime != wave)
					{
						cptr->wavetime++;
						goto TICKNEFFECTS;
					}
				}

				cptr->wavetime = 0;
				cptr->ptr[WTBL]++;
				// Wavetable jump
				if (ltable[WTBL][cptr->ptr[WTBL] - 1] == 0xff)
				{
					cptr->ptr[WTBL] = rtable[WTBL][cptr->ptr[WTBL] - 1];
				}

				if ((wave >= WAVECMD) && (wave <= WAVELASTCMD))
					goto PULSEEXEC;

				if (note != 0x80)
				{
					if (note < 0x80)
						note += cptr->note;	// relative offset (arp chord)
					note &= 0x7f;
					cptr->freq = freqtbllo[note] | (freqtblhi[note] << 8);
					cptr->vibtime = 0;
					cptr->lastnote = note;
					goto PULSEEXEC;
				}
			}

			// Tick N command
		TICKNEFFECTS:
			if ((!optimizerealtime) || (cptr->tick))
			{
				switch (cptr->command)
				{
				case CMD_PORTAUP:
				{
					unsigned short speed = 0;
					if (cptr->cmddata)
					{
						speed = (ltable[STBL][cptr->cmddata - 1] << 8) | rtable[STBL][cptr->cmddata - 1];
					}
					if (speed >= 0x8000)
					{
						speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
						speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
						speed >>= rtable[STBL][cptr->cmddata - 1];
					}
					cptr->freq += speed;

					cptr->freqBackup += speed;
				}
				break;

				case CMD_PORTADOWN:
				{
					unsigned short speed = 0;
					if (cptr->cmddata)
					{
						speed = (ltable[STBL][cptr->cmddata - 1] << 8) | rtable[STBL][cptr->cmddata - 1];
					}
					if (speed >= 0x8000)
					{
						speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
						speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
						speed >>= rtable[STBL][cptr->cmddata - 1];
					}
					cptr->freq -= speed;
					cptr->freqBackup -= speed;
				}
				break;

				case CMD_DONOTHING:
					if ((!cptr->cmddata) || (!cptr->vibdelay))
						break;
					if (cptr->vibdelay > 1)
					{
						cptr->vibdelay--;
						break;
					}
				case CMD_VIBRATO:
				{
					unsigned short speed = 0;
					unsigned char cmpvalue = 0;

					if (cptr->cmddata)
					{
						cmpvalue = ltable[STBL][cptr->cmddata - 1];
						speed = rtable[STBL][cptr->cmddata - 1];
					}
					if (cmpvalue >= 0x80)
					{
						cmpvalue &= 0x7f;
						speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
						speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
						speed >>= rtable[STBL][cptr->cmddata - 1];
					}

					if ((cptr->vibtime < 0x80) && (cptr->vibtime > cmpvalue))
						cptr->vibtime ^= 0xff;
					cptr->vibtime += 0x02;
					if (cptr->vibtime & 0x01)
						cptr->freq -= speed;
					else
						cptr->freq += speed;
				}
				break;

				case CMD_TONEPORTA:
				{
					unsigned short targetfreq = freqtbllo[cptr->note] | (freqtblhi[cptr->note] << 8);
					unsigned short speed = 0;

					if (!cptr->cmddata)
					{
						cptr->freq = targetfreq;
						cptr->vibtime = 0;
					}
					else
					{
						speed = (ltable[STBL][cptr->cmddata - 1] << 8) | rtable[STBL][cptr->cmddata - 1];
						if (speed >= 0x8000)
						{
							speed = freqtbllo[cptr->lastnote + 1] | (freqtblhi[cptr->lastnote + 1] << 8);
							speed -= freqtbllo[cptr->lastnote] | (freqtblhi[cptr->lastnote] << 8);
							speed >>= rtable[STBL][cptr->cmddata - 1];
						}
						if (cptr->freq < targetfreq)
						{
							cptr->freq += speed;
							if (cptr->freq > targetfreq)
							{
								cptr->freq = targetfreq;
								cptr->vibtime = 0;
							}
						}
						if (cptr->freq > targetfreq)
						{
							cptr->freq -= speed;
							if (cptr->freq < targetfreq)
							{
								cptr->freq = targetfreq;
								cptr->vibtime = 0;
							}
						}
					}
				}
				break;
				}
			}



		PULSEEXEC:
			if (optimizepulse)
			{
				if (c < maxSIDChannels)
				{
					if ((gt->songinit != PLAY_STOPPED) && (cptr->tick == cptr->gatetimer))
						goto GETNEWNOTES;
				}
			}

			if (cptr->ptr[PTBL])
			{
				// Skip pulse when sequencer has been executed
				if (optimizepulse)
				{
					if ((!cptr->tick) && (!cptr->pattptr)) goto NEXTCHN;
				}

				// Pulsetable jump
				if (ltable[PTBL][cptr->ptr[PTBL] - 1] == 0xff)
				{
					cptr->ptr[PTBL] = rtable[PTBL][cptr->ptr[PTBL] - 1];
					if (!cptr->ptr[PTBL]) goto PULSEEXEC;
				}

				if (!cptr->pulsetime)
				{
					// Set pulse
					if (ltable[PTBL][cptr->ptr[PTBL] - 1] >= 0x80)
					{
						cptr->pulse = (ltable[PTBL][cptr->ptr[PTBL] - 1] & 0xf) << 8;
						cptr->pulse |= rtable[PTBL][cptr->ptr[PTBL] - 1];
						cptr->ptr[PTBL]++;
					}
					else
					{
						cptr->pulsetime = ltable[PTBL][cptr->ptr[PTBL] - 1];
					}
				}
				// Pulse modulation
				if (cptr->pulsetime)
				{
					unsigned char speed = rtable[PTBL][cptr->ptr[PTBL] - 1];
					if (speed < 0x80)
					{
						cptr->pulse += speed;
						cptr->pulse &= 0xfff;
					}
					else
					{
						cptr->pulse += speed;
						cptr->pulse -= 0x100;
						cptr->pulse &= 0xfff;
					}
					cptr->pulsetime--;
					if (!cptr->pulsetime) cptr->ptr[PTBL]++;
				}
			}

			// JP Check for max playing SID count here!
			// (we process 12 channels to allow for poly mode, but only want to process, say, 3 channels for playing song)

			if ((gt->songinit == PLAY_STOPPED) || (cptr->tick != cptr->gatetimer) || (c >= maxSIDChannels))
				goto NEXTCHN;

			// New notes processing
		GETNEWNOTES:
			{
				unsigned char newnote;

				cptr->lastpattptr = cptr->pattptr;
				cptr->lastsongptr = cptr->songptr;
				cptr->lastpattnum = cptr->pattnum;

				newnote = pattern[cptr->pattnum][cptr->pattptr];
				if (pattern[cptr->pattnum][cptr->pattptr + 1])
					cptr->instr = pattern[cptr->pattnum][cptr->pattptr + 1];
				cptr->newcommand = pattern[cptr->pattnum][cptr->pattptr + 2];
				cptr->newcmddata = pattern[cptr->pattnum][cptr->pattptr + 3];

				cptr->pattptr += 4;

				if (pattern[cptr->pattnum][cptr->pattptr] == ENDPATT)
					cptr->pattptr = 0x7fffffff;

				if (newnote == KEYOFF)
					cptr->gate = 0xfe;
				if (newnote == KEYON)
					cptr->gate = 0xff;
				if (newnote <= LASTNOTE)
				{
					cptr->requestKeyOff = 0xff;
					playingChannelOnKey[c] = -1;

					cptr->newnote = newnote + cptr->trans;

					if ((cptr->newcommand) != CMD_TONEPORTA)
					{
						if (!(instr[cptr->instr].gatetimer & 0x40))
						{
							cptr->gate = 0xfe;
							if (!(instr[cptr->instr].gatetimer & 0x80))
							{
								if (!gt->noSIDWrites)
								{
									gt->sidreg[sidIndex][0x5 + 7 * (c % 3)] = adparam >> 8;
									gt->sidreg[sidIndex][0x6 + 7 * (c % 3)] = adparam & 0xff;

									if (stereoMode == 2)	// TrueStereo panning enabled
									{
										// set pan
										int panMin = instr[cptr->instr].pan >> 4;
										int panMax = instr[cptr->instr].pan & 0xf;

										if (panMax > panMin)
											cptr->pan = panMin += rand() % (panMax - panMin);
										else if (panMin > panMax)
											cptr->pan = panMax += rand() % (panMin - panMax);
										else
											cptr->pan = panMin;
									}
									else if (monomode == 1)	// mono
										cptr->pan = 0x7;
									else
									{
										int sidChip = c / 3;
										int pan = SID_StereoPanPositions[(maxSIDChannels / 3) - 1][sidChip];
										cptr->pan = pan;

										/*
																				if (sidChip == 0 || sidChip == 2)
																					cptr->pan = 0x0;	// hard left
																				else
																					cptr->pan = 0xe;	// hard right

																				if (maxSIDChannels == 9 && sidChip == 2)	// 3 SIDS? 3rd SID = center
																					cptr->pan = 0x7;
										*/
									}
								}
							}
						}
					}
				}
			}
		NEXTCHN:
			if (cptr->mute)
			{
				if (!gt->noSIDWrites)
				{
					gt->sidreg[sidIndex][0x4 + 7 * (c % 3)] = cptr->wave = 0x08;
				}
				cptr->releaseTime = 0;
			}
			else
			{
				if (!gt->noSIDWrites)
				{
					gt->sidreg[sidIndex][0x0 + 7 * (c % 3)] = cptr->freq & 0xff;
					gt->sidreg[sidIndex][0x1 + 7 * (c % 3)] = cptr->freq >> 8;
					gt->sidreg[sidIndex][0x2 + 7 * (c % 3)] = cptr->pulse & 0xfe;
					gt->sidreg[sidIndex][0x3 + 7 * (c % 3)] = (cptr->pulse >> 8) | (cptr->pan << 4);

					gt->sidreg[sidIndex][0x4 + 7 * (c % 3)] = cptr->wave & cptr->gate & cptr->requestKeyOff;

					cptr->releaseTime -= 50 * 2;
					if (cptr->releaseTime < 0)
						cptr->releaseTime = 0;
				}
			}
			cptr++;
		}
	}

	if (gt->songinit != PLAY_STOPPED)
		incrementtime(gt);


	/*
	jdebug[15]++;

	if (gtObject.interPatternLoopEnabledFlag && gt->disableLoopSearch == 0)	// && gt->loopEnabledFlag)
	{
		sprintf(textbuffer, "interloop %x %x", jdebug[15], useOriginalGTFunctionKeys);
		printtext(70, 12, 0xe, textbuffer);
	}
	*/

	if (gt->noSIDWrites == 0 && gt->loopEnabledFlag  && gt->disableLoopSearch == 0 && gtObject.interPatternLoopEnabledFlag && transportLoopPatternSelectArea)
	{
		int c3 = ((gtObject.interPatternLoopEnabledFlag >> 8) & 0xff) - 1;
		int markEnd = (gtObject.interPatternLoopEnabledFlag & 0xff);
		gtObject.interPatternLoopEnabledFlag = 0;

		if (gt->chn[c3].pattptr != (markEnd) * 4)
		{
			for (int i = 0; i < maxSIDChannels; i++)
			{
				gt->chn[i].pattnum = gt->patternLoopStartChn[i].pattnum;
				gt->chn[i].songptr = gt->patternLoopStartChn[i].songptr;
				gt->chn[i].pattptr = gt->patternLoopStartChn[i].pattptr;
				gt->chn[i].tick = gt->patternLoopStartChn[i].tick;
				gt->chn[i].repeat = gt->patternLoopStartChn[i].repeat;
				gt->chn[i].tempo = gt->patternLoopStartChn[i].tempo;
				gt->chn[i].advance = gt->patternLoopStartChn[i].advance;
			}

			// Reset playing timer
			gt->timemin = gt->looptimemin;
			gt->timesec = gt->looptimesec;
			gt->timeframe = gt->looptimeframe;

		}
	}
	else if (gt->noSIDWrites == 0 && gt->loopEnabledFlag  && gt->disableLoopSearch == 0 && transportLoopPattern)
	{
		// Check to see if playback has reached the end of the pattern that is being tracked by the Master Loop Channel
		// If so - and if looping is enabled, reset all channels to their correct positions.
		// This syncs, taking into consideration that patterns can be different lengths, or have different tempos, etc.

		for (int i = 0; i < maxSIDChannels; i++)
		{
			if (gt->chn[i].pattptr == gt->loopEndChn[i].pattptr && gt->chn[i].songptr == gt->loopEndChn[i].songptr && gt->loopEndChn[i].tick == gt->chn[i].tick)
			{
				// Only copy specific parts, as we don't want to reset instrument info when looping (so, no blanket memcpy)
				//			memcpy(&gt->chn[i], &gt->loopStartChn[i], sizeof(CHN));
				gt->chn[i].pattnum = gt->loopStartChn[i].pattnum;
				gt->chn[i].songptr = gt->loopStartChn[i].songptr;
				gt->chn[i].pattptr = gt->loopStartChn[i].pattptr;
				gt->chn[i].tick = gt->loopStartChn[i].tick;
				gt->chn[i].repeat = gt->loopStartChn[i].repeat;
				gt->chn[i].tempo = gt->loopStartChn[i].tempo;
				gt->chn[i].advance = gt->loopStartChn[i].advance;

				// Reset playing timer
				gt->timemin = gt->looptimemin;
				gt->timesec = gt->looptimesec;
				gt->timeframe = gt->looptimeframe;


				int jnum = getActualSongNumber(gt->psnum, i);
				int c2 = getActualChannel(jnum, i);

				if (editorInfo.expandOrderListView)
				{

					int t = songOrderTranspose[jnum][c2][gt->chn[i].songptr - 1];
					if (t & 0x80)
					{
						t &= 0x7f;
						gt->chn[i].trans = -t;
					}
					else
					{
						gt->chn[i].trans = t;
					}
				}
			}
		}

	}


	if (gt->noSIDWrites == 0 && gt->disableLoopSearch == 0)	// only the PLAYING GTObject should check for looping. Otherwise, we end up in recursive hell
	{
		if (transportLoopPattern && transportLoopPatternSelectArea)
		{
			int c3 = getActualChannel(gt->psnum, editorInfo.epmarkchn);

			if (editorInfo.epmarkchn >= 0)
			{
				int plen = (pattlen[gt->chn[c3].pattnum] - 1);	// *4;
//				sprintf(textbuffer, "%x hit loop %x  ", jdebug[15]++, editorInfo.epmarkchn);
	//			printtext(70, 12, 0xe, textbuffer);

				if (editorInfo.epmarkend > plen || editorInfo.epmarkstart > plen)
				{
					editorInfo.epmarkstart = editorInfo.epmarkend = 0;
					editorInfo.epmarkchn = -1;
				}
			}

			int markStart = editorInfo.epmarkstart;
			int markEnd = editorInfo.epmarkend;

			//	sprintf(textbuffer, "%x hit loop %x %x %x  ", jdebug[15]++, markStart, markEnd, plen);
			//	printtext(70, 12, 0xe, textbuffer);

			if (markStart != markEnd && editorInfo.epmarkchn >= 0)// 1.1.7 FIX (only check for inter-pattern looping if area is selected) 3/5/2022
			{
				if (markEnd < markStart)
				{
					markStart = markEnd;
					markEnd = editorInfo.epmarkstart;
				}

				if (gt->chn[editorInfo.epmarkchn].pattptr == markEnd * 4)
				{
					//					if (calcStartofInterPatternLoop(gt->psnum, c3, gt->chn[c3].songptr - 1, &gtLoopObject))
					if (calcStartofInterPatternLoop(gt->psnum, c3, gt->chn[editorInfo.epmarkchn].songptr - 1, &gtLoopObject))
					{
						gtObject.interPatternLoopEnabledFlag = (markEnd)+((editorInfo.epmarkchn + 1) << 8);	// c3+1 - just to make sure we don't get a 0 flag value
					}
				}
			}
		}

		//-------------


		// Wait until playback of the loop master channel is at the very end of the pattern before recalculating loop start positions
		// This will then take into consideration any changes made to the pattern length or changes to the order list during playback

//		if (gt->chn[gt->masterLoopChannel].songptr != gt->chn[gt->masterLoopChannel].songLoopPtr
//			&& gt->chn[gt->masterLoopChannel].pattptr == ((pattlen[gt->chn[gt->masterLoopChannel].pattnum] - 1) * 4))


		if (gt->chn[gt->masterLoopChannel].pattptr == ((pattlen[gt->chn[gt->masterLoopChannel].pattnum] - 1) * 4))
		{
			gt->chn[gt->masterLoopChannel].songLoopPtr = gt->chn[gt->masterLoopChannel].songptr;

	//		sprintf(textbuffer, "end song %x mlc %x ec %x em %x %d", gt->chn[gt->masterLoopChannel].songLoopPtr, gt->masterLoopChannel, editorInfo.epchn, editorInfo.editmode, jpd++);
	//		printtext(PANEL_NAMES_X + 24, PANEL_NAMES_Y + 3, 0xe, textbuffer);
			//	printtext(70, 36, 0xe, textbuffer);
			calculateLoopInfo2(gt->psnum, gt->masterLoopChannel, gt->chn[gt->masterLoopChannel].songptr - 1, &gtLoopObject);
			gtObject.loopEnabledFlag = 1;
		}

	}

}




void sequencer(int c, CHN *cptr, GTOBJECT *gt)
{
	if (c >= maxSIDChannels)
		return;

	if ((gt->songinit != PLAY_STOPPED) && (cptr->pattptr == 0x7fffffff))
	{
		cptr->pattptr = gt->startpattpos * 4;

		if (!cptr->advance) goto SEQDONE;

		if (editorInfo.expandOrderListView == 0)
			updateCompressedSeq(c, cptr, gt);
		else
			updateExpandedSeq(c, cptr, gt);
	}
SEQDONE: {}
}

void updateCompressedSeq(int c, CHN *cptr, GTOBJECT *gt)
{
	int jnum = getActualSongNumber(gt->psnum, c);
	int c2 = c % MAX_CHN;
	// Song loop
	if (songorder[jnum][c2][cptr->songptr] == LOOPSONG)
	{
		cptr->loopCount++;
		cptr->songptr = songorder[jnum][c2][cptr->songptr + 1];
		if (cptr->songptr >= songlen[jnum][c2])
		{
			stopsong(gt);
			cptr->songptr = 0;
			goto SEQDONE;
		}
	}
	// Transpose
	if ((songorder[jnum][c2][cptr->songptr] >= TRANSDOWN) && (songorder[jnum][c2][cptr->songptr] < LOOPSONG))
	{
		cptr->trans = songorder[jnum][c2][cptr->songptr] - TRANSUP;
		cptr->songptr++;
	}
	// Repeat
	if ((songorder[jnum][c2][cptr->songptr] >= REPEAT) && (songorder[jnum][c2][cptr->songptr] < TRANSDOWN))
	{
		cptr->repeat = songorder[jnum][c2][cptr->songptr] - REPEAT;
		cptr->songptr++;
	}
	// Pattern number
	cptr->pattnum = songorder[jnum][c2][cptr->songptr];
	if (cptr->repeat)
		cptr->repeat--;
	else
		cptr->songptr++;

	// Check for illegal pattern now
	if (cptr->pattnum >= MAX_PATT)
	{
		stopsong(gt);
		cptr->pattnum = 0;
	}
	if (cptr->pattptr >= (pattlen[cptr->pattnum] * 4))
		cptr->pattptr = 0;

	// Check for playback endpos
	if ((gt->lastsonginit != PLAY_BEGINNING) && (gt->editorUndoInfo.editorInfo[c].esend > 0) && (gt->editorUndoInfo.editorInfo[c].esend > gt->editorUndoInfo.editorInfo[c].espos) && (cptr->songptr > gt->editorUndoInfo.editorInfo[c].esend) && (gt->editorUndoInfo.editorInfo[c].espos < songlen[jnum][c2]))
		cptr->songptr = gt->editorUndoInfo.editorInfo[c].espos;

SEQDONE: {}
}


void updateExpandedSeq(int c, CHN *cptr, GTOBJECT *gt)
{
	int jnum = getActualSongNumber(gt->psnum, c);
	int c2 = c % MAX_CHN;
	// Song loop

	if (songOrderPatterns[jnum][c2][cptr->songptr] == LOOPSONG)
	{
		cptr->loopCount++;
		cptr->songptr = songOrderTranspose[jnum][c2][cptr->songptr];
		if (cptr->songptr >= songOrderLength[jnum][c2])
		{
			stopsong(gt);
			cptr->songptr = 0;
			goto SEQDONE;
		}
	}

	int t = songOrderTranspose[jnum][c2][cptr->songptr];
	if (t & 0x80)
	{
		t &= 0x7f;
		cptr->trans = -t;
	}
	else
	{
		cptr->trans = t;
	}


	/*
		// Transpose
		if ((songorder[jnum][c2][cptr->songptr] >= TRANSDOWN) && (songorder[jnum][c2][cptr->songptr] < LOOPSONG))
		{
			cptr->trans = songorder[jnum][c2][cptr->songptr] - TRANSUP;
			cptr->songptr++;
		}
		// Repeat
		if ((songorder[jnum][c2][cptr->songptr] >= REPEAT) && (songorder[jnum][c2][cptr->songptr] < TRANSDOWN))
		{
			cptr->repeat = songorder[jnum][c2][cptr->songptr] - REPEAT;
			cptr->songptr++;
		}
	*/
	// Pattern number
	cptr->pattnum = songOrderPatterns[jnum][c2][cptr->songptr];	// songorder[jnum][c2][cptr->songptr];
	cptr->songptr++;
	cptr->nextPatternTriggered++;
	/*
		if (cptr->repeat)
			cptr->repeat--;
		else
			cptr->songptr++;
	*/
	// Check for illegal pattern now
	if (cptr->pattnum >= MAX_PATT)
	{
		stopsong(gt);
		cptr->pattnum = 0;
	}
	if (cptr->pattptr >= (pattlen[cptr->pattnum] * 4))
		cptr->pattptr = 0;

	// Check for playback endpos
	if ((gt->lastsonginit != PLAY_BEGINNING) && (gt->editorUndoInfo.editorInfo[c].esend > 0) && (gt->editorUndoInfo.editorInfo[c].esend > gt->editorUndoInfo.editorInfo[c].espos) && (cptr->songptr > gt->editorUndoInfo.editorInfo[c].esend) && (gt->editorUndoInfo.editorInfo[c].espos < songOrderLength[jnum][c2]))
		cptr->songptr = gt->editorUndoInfo.editorInfo[c].espos;

SEQDONE: {}
}

/*
Returns BaseSong (channel<6) or BaseSong+1 (channel>=6)
Used for accessing songorder[n] etc.
*/
int getActualSongNumber(int currentSong, int channel)
{
	if (maxSIDChannels > 6)
	{
		int baseSong = currentSong & 0xfffffffe;	// base song = 0,2,4,6,8....
		int nextSong = baseSong + 1;	// next song = 1,3,5,9....

		if (channel < 6)
			return baseSong;
		return nextSong;
	}
	else
		return currentSong;
}
/*
Returns channel number based on Song number (0-5 or 6-11)
Used for accessing chn[n] array
*/
int getActualChannel(int currentSong, int channel)
{
	if (maxSIDChannels > 6)
	{
		// Return 0-11
		int baseSong = currentSong & 1;		// 1
		int chAdd = 6 * baseSong;			// 6
		channel %= 6;						// 5
		channel += chAdd;					// 6
		return channel;
	}
	else
		return channel;	// 0-5

}

void initSID(GTOBJECT *gt)
{

	gt->sidreg[0] = &sidreg[0];
	gt->sidreg[1] = &sidreg2[0];
	gt->sidreg[2] = &sidreg3[0];
	gt->sidreg[3] = &sidreg4[0];

}

// ch 0-11
// returns 0 (filter OFF) or 1 (filter ON)
int getFilterOnOff(GTOBJECT *gt, int ch)
{
	int filterOnOffBitmask = gt->filterInfo[ch / 3].filterctrl;
	ch %= 3;
	filterOnOffBitmask >>= ch;	// bottom 3 bits = Filt3, Filt2, Filt1
	filterOnOffBitmask &= 1;
	return filterOnOffBitmask;
}

int getFilterResonance(GTOBJECT *gt, int ch)
{
	int filterRes = gt->filterInfo[ch / 3].filterctrl;
	filterRes >>= 4;
	filterRes &= 0xf;
	return filterRes;
}

int getFilterCutoff(GTOBJECT *gt, int ch)
{
	return gt->filterInfo[ch / 3].filtercutoff;
}

int getFilterType(GTOBJECT *gt, int ch)
{
	int filterTypeBitmask = gt->filterInfo[ch / 3].filtertype;
	filterTypeBitmask >>= 4;
	filterTypeBitmask &= 0x7;
	return filterTypeBitmask;
}


