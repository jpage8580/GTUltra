//
// GTUltra pattern editor
//

#define GPATTERN_C

#include "goattrk2.h"


unsigned char notekeytbl1[] = { KEY_Z, KEY_S, KEY_X, KEY_D, KEY_C, KEY_V,
  KEY_G, KEY_B, KEY_H, KEY_N, KEY_J, KEY_M, KEY_COMMA, KEY_L, KEY_COLON };

unsigned char notekeytbl2[] = { KEY_Q, KEY_2, KEY_W, KEY_3, KEY_E, KEY_R,
  KEY_5, KEY_T, KEY_6, KEY_Y, KEY_7, KEY_U, KEY_I, KEY_9, KEY_O, KEY_0, KEY_P };

unsigned char dmckeytbl[] = { KEY_A, KEY_W, KEY_S, KEY_E, KEY_D, KEY_F,
  KEY_T, KEY_G, KEY_Y, KEY_H, KEY_U, KEY_J, KEY_K, KEY_O, KEY_L, KEY_P };

unsigned char jankokeytbl1[] = { KEY_Z, KEY_S, KEY_X, KEY_D, KEY_C, KEY_F, KEY_V,
  KEY_G, KEY_B, KEY_H, KEY_N, KEY_J, KEY_M, KEY_K, KEY_COMMA, KEY_L, KEY_COLON };

unsigned char jankokeytbl2[] = { KEY_Q, KEY_2, KEY_W, KEY_3, KEY_E, KEY_4, KEY_R,
  KEY_5, KEY_T, KEY_6, KEY_Y, KEY_7, KEY_U, KEY_8, KEY_I, KEY_9, KEY_O, KEY_0, KEY_P };

unsigned char patterncopybuffer[MAX_PATTROWS * 4 + 4];
unsigned char cmdcopybuffer[MAX_PATTROWS * 4 + 4];
int patterncopyrows = 0;
int cmdcopyrows = 0;

EDITOR_INFO editorInfo;
EDITOR_INFO editorInfoBackup;
int disableEnterToReturnToLastPos;


void patterncommands(GTOBJECT *gt, int midiNote)
{
	int jrawkey = rawkey;
	int jkey = key;

	if (midiNote >= 0)
	{
		jrawkey = 0;
		jkey = 0;
	}

	int ret;
	int c, scrrep;

	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12

	if (c2 >= maxSIDChannels)
		return;

	int songNum = getActualSongNumber(editorInfo.esnum, c2);
	int c3 = c2 % 6;

	switch (jkey)
	{
	case '<':
	case '(':
	case '[':
		prevpattern(gt);
		break;

	case '>':
	case ')':
	case ']':
		nextpattern(gt);
		break;
	}
	{


		int newnote = -1;
		if (jkey && midiNote == -1)
		{
			switch (keypreset)
			{
			case KEY_TRACKER:
				for (c = 0; c < sizeof(notekeytbl1); c++)
				{
					if ((jrawkey == notekeytbl1[c]) && (!editorInfo.epcolumn) && (!shiftpressed))
					{
						newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
					}
				}
				for (c = 0; c < sizeof(notekeytbl2); c++)
				{
					if ((jrawkey == notekeytbl2[c]) && (!editorInfo.epcolumn) && (!shiftpressed))
					{
						newnote = FIRSTNOTE + c + (editorInfo.epoctave + 1) * 12;
					}
				}
				break;

			case KEY_DMC:
				for (c = 0; c < sizeof(dmckeytbl); c++)
				{
					if ((jrawkey == dmckeytbl[c]) && (!editorInfo.epcolumn) && (!shiftpressed))
					{
						newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
					}
				}
				break;

			case KEY_JANKO:
				for (c = 0; c < sizeof(jankokeytbl1); c++)
				{
					if ((jrawkey == jankokeytbl1[c]) && (!editorInfo.epcolumn) && (!shiftpressed))
					{
						newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
					}
				}
				for (c = 0; c < sizeof(jankokeytbl2); c++)
				{
					if ((jrawkey == jankokeytbl2[c]) && (!editorInfo.epcolumn) && (!shiftpressed))
					{
						newnote = FIRSTNOTE + c + (editorInfo.epoctave + 1) * 12;
					}
				}
				break;
			}
		}
		else
			newnote = midiNote;


		if (newnote > LASTNOTE) newnote = -1;
		if ((jrawkey == KEY_BACKSPACE) && (!editorInfo.epcolumn)) newnote = REST;
		if ((jrawkey == 0x14) && (!editorInfo.epcolumn)) newnote = KEYOFF;
		if (jrawkey == KEY_ENTER)
		{
			switch (editorInfo.epcolumn)
			{
			case 0:
				if (shiftpressed)
					newnote = KEYON;
				else
					newnote = KEYOFF;
				break;

			case 1:
			case 2:
				if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1])
				{
					gotoinstr(pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1]);
					return;
				}
				break;

			default:
				switch (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])
				{
				case CMD_SETWAVEPTR:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						allowEnterToReturnToPosition();
						gototable(WTBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
						return;
					}
					else
					{
						if (shiftpressed)
						{
							int pos = gettablelen(WTBL);
							if (pos >= MAX_TABLELEN - 1) pos = MAX_TABLELEN - 1;
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
							allowEnterToReturnToPosition();
							gototable(WTBL, pos);
							return;
						}
					}
					break;

				case CMD_SETPULSEPTR:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						allowEnterToReturnToPosition();
						gototable(PTBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
						return;
					}
					else
					{
						if (shiftpressed)
						{
							int pos = gettablelen(PTBL);
							if (pos >= MAX_TABLELEN - 1) pos = MAX_TABLELEN - 1;
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
							allowEnterToReturnToPosition();
							gototable(PTBL, pos);
							return;
						}
					}
					break;

				case CMD_SETFILTERPTR:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						allowEnterToReturnToPosition();
						gototable(FTBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
						return;
					}
					else
					{
						if (shiftpressed)
						{
							int pos = gettablelen(FTBL);
							if (pos >= MAX_TABLELEN - 1) pos = MAX_TABLELEN - 1;
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
							allowEnterToReturnToPosition();
							gototable(FTBL, pos);
							return;
						}
					}
					break;

				case CMD_FUNKTEMPO:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						if (!shiftpressed)
						{
							allowEnterToReturnToPosition();
							gototable(STBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
							return;
						}
						else
						{
							int pos = makespeedtable(pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3], MST_FUNKTEMPO, 1);
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
						}
					}
					else
					{
						if (shiftpressed)
						{
							int pos = findfreespeedtable();
							if (pos >= 0)
							{
								pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
								allowEnterToReturnToPosition();
								gototable(STBL, pos);
								return;
							}
						}
					}
					break;

				case CMD_PORTAUP:
				case CMD_PORTADOWN:
				case CMD_TONEPORTA:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						if (!shiftpressed)
						{

							allowEnterToReturnToPosition();
							gototable(STBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
							return;
						}
						else
						{
							int pos = makespeedtable(pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3], MST_PORTAMENTO, 1);
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
						}
					}
					else
					{
						if (shiftpressed)
						{
							int pos = findfreespeedtable();
							if (pos >= 0)
							{
								pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
								allowEnterToReturnToPosition();
								gototable(STBL, pos);
								return;
							}
						}
					}
					break;

				case CMD_VIBRATO:
					if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3])
					{
						if (!shiftpressed)
						{
							allowEnterToReturnToPosition();
							gototable(STBL, pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] - 1);
							return;
						}
						else
						{
							int pos = makespeedtable(pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3], finevibrato, 1);
							pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
						}
					}
					else
					{
						if (shiftpressed)
						{
							int pos = findfreespeedtable();
							if (pos >= 0)
							{
								pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
								allowEnterToReturnToPosition();
								gototable(STBL, pos);
								return;
							}
						}
					}
					break;
				}
				break;
			}
			if ((autoadvance < 2) && (editorInfo.epcolumn))
			{
				editorInfo.eppos++;
				if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
				{
					editorInfo.eppos = 0;
				}
			}
		}

		if (newnote >= 0)
		{
			if ((recordmode) && (editorInfo.eppos < pattlen[gt->editorInfo[c2].epnum]))
			{
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4] = newnote;
				if (newnote < REST)
				{
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] = editorInfo.einum;
				}
				else
				{
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] = 0;
				}
				if ((shiftpressed) && (newnote == REST))
				{
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = 0;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0;
				}

			}
			if (recordmode)
			{
				if (autoadvance < 2)
				{
					editorInfo.eppos++;
					if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
					{
						editorInfo.eppos = 0;
					}
				}

				playtestnote(newnote, editorInfo.einum, editorInfo.epchn, gt);
			}



		}
	}

	switch (jrawkey)
	{


	case KEY_UP:
	case KEY_DOWN:
	case KEY_LEFT:
	case KEY_RIGHT:
	case KEY_DEL:
	case KEY_INS:
	case KEY_BACKSPACE:
		win_enableKeyRepeat();
		break;

	case KEY_Z:
		if (ctrlpressed)
			win_enableKeyRepeat();	// Undo works with key repeat
		else if (!enablekeyrepeat)
			win_disableKeyRepeat();
		break;
	default:

		if (!enablekeyrepeat)
			win_disableKeyRepeat();
	}

	switch (jrawkey)
	{

	case KEY_Y:	//0x5c:		// hash/tilde
		if (shiftpressed)
		{

			undoAreaSetCheckForChange(UNDO_AREA_TABLES + STBL, 0, UNDO_AREA_DIRTY_CHECK);	// left table
			undoAreaSetCheckForChange(UNDO_AREA_TABLES + STBL, 1, UNDO_AREA_DIRTY_CHECK);	// right table

			autoPitchbendToNextNote(gt);

			refreshVariables();
		}

		break;

	case KEY_O:
		if (shiftpressed) shrinkpattern(gt);
		break;

	case KEY_P:
		if (shiftpressed) expandpattern(gt);
		break;

	case KEY_J:
		if (shiftpressed) joinpattern(gt);
		break;

	case KEY_K:
		if (shiftpressed) splitpattern(gt);
		break;

	case KEY_Z:
		if (shiftpressed)
		{
			if (!ctrlpressed)
			{
				autoadvance++;
				if (autoadvance > 2) autoadvance = 0;
				if (keypreset == KEY_TRACKER)
				{
					if (autoadvance == 1) autoadvance = 2;
				}
			}
		}
		break;

	case KEY_E:
		if (shiftpressed)
		{
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				if (editorInfo.epmarkstart < editorInfo.epmarkend)
				{
					int d = 0;
					for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						cmdcopybuffer[d * 4 + 2] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 2];
						cmdcopybuffer[d * 4 + 3] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 3];
						d++;
					}
					cmdcopyrows = d;
				}
				else
				{
					int d = 0;
					for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						cmdcopybuffer[d * 4 + 2] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 2];
						cmdcopybuffer[d * 4 + 3] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 3];
						d++;
					}
					cmdcopyrows = d;
				}
				editorInfo.epmarkchn = -1;
			}
			else
			{
				if (editorInfo.eppos < pattlen[gt->editorInfo[c2].epnum])
				{
					cmdcopybuffer[2] = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2];
					cmdcopybuffer[3] = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3];
					cmdcopyrows = 1;
				}
			}
		}
		break;

	case KEY_R:
		if (shiftpressed)
		{
			for (c = 0; c < cmdcopyrows; c++)
			{
				if (editorInfo.eppos >= pattlen[gt->editorInfo[c2].epnum]) break;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = cmdcopybuffer[c * 4 + 2];
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = cmdcopybuffer[c * 4 + 3];
				editorInfo.eppos++;
			}
		}
		break;

	case KEY_I:
		if (shiftpressed)
		{
			int d, e;
			char temp;
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				if (editorInfo.epmarkstart <= editorInfo.epmarkend)
				{
					e = editorInfo.epmarkend;
					for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						for (d = 0; d < 4; d++)
						{
							temp = pattern[gt->editorInfo[mc2].epnum][c * 4 + d];
							pattern[gt->editorInfo[mc2].epnum][c * 4 + d] = pattern[gt->editorInfo[mc2].epnum][e * 4 + d];
							pattern[gt->editorInfo[mc2].epnum][e * 4 + d] = temp;
						}
						e--;
						if (e < c) break;
					}
				}
				else
				{
					e = editorInfo.epmarkstart;
					for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						for (d = 0; d < 4; d++)
						{
							temp = pattern[gt->editorInfo[mc2].epnum][c * 4 + d];
							pattern[gt->editorInfo[mc2].epnum][c * 4 + d] = pattern[gt->editorInfo[mc2].epnum][e * 4 + d];
							pattern[gt->editorInfo[mc2].epnum][e * 4 + d] = temp;
						}
						e--;
						if (e < c) break;
					}
				}
			}
			else
			{
				e = pattlen[gt->editorInfo[c2].epnum] - 1;
				for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)
				{
					for (d = 0; d < 4; d++)
					{
						temp = pattern[gt->editorInfo[c2].epnum][c * 4 + d];
						pattern[gt->editorInfo[c2].epnum][c * 4 + d] = pattern[gt->editorInfo[c2].epnum][e * 4 + d];
						pattern[gt->editorInfo[c2].epnum][e * 4 + d] = temp;
					}
					e--;
					if (e < c) break;
				}
			}
		}
		break;

	case KEY_Q:
		if (shiftpressed)
		{
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				if (editorInfo.epmarkstart <= editorInfo.epmarkend)
				{
					for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] < LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
							pattern[gt->editorInfo[mc2].epnum][c * 4]++;
					}
				}
				else
				{
					for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] < LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
							pattern[gt->editorInfo[mc2].epnum][c * 4]++;
					}
				}
			}
			else
			{
				for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)
				{
					if ((pattern[gt->editorInfo[c2].epnum][c * 4] < LASTNOTE) &&
						(pattern[gt->editorInfo[c2].epnum][c * 4] >= FIRSTNOTE))
						pattern[gt->editorInfo[c2].epnum][c * 4]++;
				}
			}
		}
		break;

	case KEY_A:
		if (shiftpressed)
		{
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				if (editorInfo.epmarkstart <= editorInfo.epmarkend)
				{
					for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] > FIRSTNOTE))
							pattern[gt->editorInfo[mc2].epnum][c * 4]--;
					}
				}
				else
				{
					for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] > FIRSTNOTE))
							pattern[gt->editorInfo[mc2].epnum][c * 4]--;
					}
				}
			}
			else
			{
				for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)
				{
					if ((pattern[gt->editorInfo[c2].epnum][c * 4] <= LASTNOTE) &&
						(pattern[gt->editorInfo[c2].epnum][c * 4] > FIRSTNOTE))
						pattern[gt->editorInfo[c2].epnum][c * 4]--;
				}
			}
		}
		break;

	case KEY_W:
		if (shiftpressed)
		{
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				if (editorInfo.epmarkstart <= editorInfo.epmarkend)
				{
					for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
						{
							pattern[gt->editorInfo[mc2].epnum][c * 4] += 12;
							if (pattern[gt->editorInfo[mc2].epnum][c * 4] > LASTNOTE)
								pattern[gt->editorInfo[mc2].epnum][c * 4] = LASTNOTE;
						}
					}
				}
				else
				{
					for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
					{
						if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
						if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
							(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
						{
							pattern[gt->editorInfo[mc2].epnum][c * 4] += 12;
							if (pattern[gt->editorInfo[mc2].epnum][c * 4] > LASTNOTE)
								pattern[gt->editorInfo[mc2].epnum][c * 4] = LASTNOTE;
						}
					}
				}
			}
			else
			{
				for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)
				{
					if ((pattern[gt->editorInfo[c2].epnum][c * 4] <= LASTNOTE) &&
						(pattern[gt->editorInfo[c2].epnum][c * 4] >= FIRSTNOTE))
					{
						pattern[gt->editorInfo[c2].epnum][c * 4] += 12;
						if (pattern[gt->editorInfo[c2].epnum][c * 4] > LASTNOTE)
							pattern[gt->editorInfo[c2].epnum][c * 4] = LASTNOTE;
					}
				}
			}
		}
		break;

	case KEY_S:
		if (shiftpressed)
		{
			if (!ctrlpressed)
			{
				if (editorInfo.epmarkchn != -1)
				{
					int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

					if (editorInfo.epmarkstart <= editorInfo.epmarkend)
					{
						for (c = editorInfo.epmarkstart; c <= editorInfo.epmarkend; c++)
						{
							if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
							if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
								(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
							{
								pattern[gt->editorInfo[mc2].epnum][c * 4] -= 12;
								if (pattern[gt->editorInfo[mc2].epnum][c * 4] < FIRSTNOTE)
									pattern[gt->editorInfo[mc2].epnum][c * 4] = FIRSTNOTE;
							}
						}
					}
					else
					{
						for (c = editorInfo.epmarkend; c <= editorInfo.epmarkstart; c++)
						{
							if (c >= pattlen[gt->editorInfo[mc2].epnum]) break;
							if ((pattern[gt->editorInfo[mc2].epnum][c * 4] <= LASTNOTE) &&
								(pattern[gt->editorInfo[mc2].epnum][c * 4] >= FIRSTNOTE))
							{
								pattern[gt->editorInfo[mc2].epnum][c * 4] -= 12;
								if (pattern[gt->editorInfo[mc2].epnum][c * 4] < FIRSTNOTE)
									pattern[gt->editorInfo[mc2].epnum][c * 4] = FIRSTNOTE;
							}
						}
					}
				}
				else
				{
					for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)
					{
						if ((pattern[gt->editorInfo[c2].epnum][c * 4] <= LASTNOTE) &&
							(pattern[gt->editorInfo[c2].epnum][c * 4] >= FIRSTNOTE))
						{
							pattern[gt->editorInfo[c2].epnum][c * 4] -= 12;
							if (pattern[gt->editorInfo[c2].epnum][c * 4] < FIRSTNOTE)
								pattern[gt->editorInfo[c2].epnum][c * 4] = FIRSTNOTE;
						}
					}
				}
			}
		}
		break;

	case KEY_M:
		if (shiftpressed)
		{
			stepsize++;
			if (stepsize > MAX_PATTROWS) stepsize = MAX_PATTROWS;
		}
		break;

	case KEY_N:
		if (shiftpressed)
		{
			stepsize--;
			if (stepsize < 2) stepsize = 2;
		}
		break;

	case KEY_H:
		if (shiftpressed)
		{
			switch (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])
			{
			case CMD_PORTAUP:
			case CMD_PORTADOWN:
			case CMD_VIBRATO:
			case CMD_TONEPORTA:
				if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] == CMD_TONEPORTA)
					c = editorInfo.eppos - 1;
				else
					c = editorInfo.eppos;
				for (; c >= 0; c--)
				{
					if ((pattern[gt->editorInfo[c2].epnum][c * 4] >= FIRSTNOTE) &&
						(pattern[gt->editorInfo[c2].epnum][c * 4] <= LASTNOTE))
					{
						int delta;
						int pitch1;
						int pitch2;
						int pos;
						int note = pattern[gt->editorInfo[c2].epnum][c * 4] - FIRSTNOTE;
						int right = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] & 0xf;
						int left = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] >> 4;

						if (note > MAX_NOTES - 1) note--;
						pitch1 = freqtbllo[note] | (freqtblhi[note] << 8);
						pitch2 = freqtbllo[note + 1] | (freqtblhi[note + 1] << 8);
						delta = pitch2 - pitch1;

						while (left--) delta <<= 1;
						while (right--) delta >>= 1;

						if (pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] == CMD_VIBRATO)
						{
							if (delta > 0xff) delta = 0xff;
						}
						pos = makespeedtable(delta, MST_RAW, 1);
						pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = pos + 1;
						break;
					}
				}
				break;
			}
		}
		break;

	case KEY_L:
		if (ctrlpressed)
		{

		}
		else if (shiftpressed)
		{
			if (editorInfo.epmarkchn == -1)
			{
				editorInfo.epmarkchn = editorInfo.epchn;
				editorInfo.epmarkstart = 0;
				editorInfo.epmarkend = pattlen[gt->editorInfo[c2].epnum] - 1;
			}
			else editorInfo.epmarkchn = -1;
		}
		break;

	case KEY_C:
	case KEY_X:

		if (shiftpressed)
		{
			if (editorInfo.epmarkchn != -1)
			{
				int mc2 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);	// 0-12

				int start = editorInfo.epmarkstart;
				int end = editorInfo.epmarkend;
				if (editorInfo.epmarkstart >= editorInfo.epmarkend)
				{
					start = end;
					end = editorInfo.epmarkstart;
				}

				int d = 0;
				for (c = start; c <= end; c++)
				{
					if (c >= pattlen[gt->editorInfo[mc2].epnum])
						break;

					int offset = d * 4;
					patterncopybuffer[offset] = pattern[gt->editorInfo[mc2].epnum][c * 4];
					patterncopybuffer[offset + 1] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 1];
					patterncopybuffer[offset + 2] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 2];
					patterncopybuffer[offset + 3] = pattern[gt->editorInfo[mc2].epnum][c * 4 + 3];

					if (jrawkey == KEY_X)
					{
						pattern[gt->editorInfo[mc2].epnum][c * 4] = REST;
						pattern[gt->editorInfo[mc2].epnum][c * 4 + 1] = 0;
						pattern[gt->editorInfo[mc2].epnum][c * 4 + 2] = 0;
						pattern[gt->editorInfo[mc2].epnum][c * 4 + 3] = 0;
					}
					d++;
				}
				patterncopyrows = d;
				editorInfo.epmarkchn = -1;
				return;
			}
			else
			{
				if (ctrlpressed)
				{
					int c = editorInfo.eppos;
					patterncopybuffer[0] = pattern[gt->editorInfo[c2].epnum][c * 4];
					patterncopybuffer[1] = pattern[gt->editorInfo[c2].epnum][c * 4 + 1];
					patterncopybuffer[2] = pattern[gt->editorInfo[c2].epnum][c * 4 + 2];
					patterncopybuffer[3] = pattern[gt->editorInfo[c2].epnum][c * 4 + 3];
					if (jrawkey == KEY_X)
					{
						pattern[gt->editorInfo[c2].epnum][c * 4] = REST;
						pattern[gt->editorInfo[c2].epnum][c * 4 + 1] = 0;
						pattern[gt->editorInfo[c2].epnum][c * 4 + 2] = 0;
						pattern[gt->editorInfo[c2].epnum][c * 4 + 3] = 0;
					}
					else if (pattern[gt->editorInfo[c2].epnum][c * 4 + 1])
						editorInfo.einum = pattern[gt->editorInfo[c2].epnum][c * 4 + 1];
					patterncopyrows = 1;
				}
				else // no marked selection
				{
					int d = 0;
					for (c = 0; c < pattlen[gt->editorInfo[c2].epnum]; c++)		// shift-c = copy whole pattern. ctrl-c = copy current row
					{
						patterncopybuffer[d * 4] = pattern[gt->editorInfo[c2].epnum][c * 4];
						patterncopybuffer[d * 4 + 1] = pattern[gt->editorInfo[c2].epnum][c * 4 + 1];
						patterncopybuffer[d * 4 + 2] = pattern[gt->editorInfo[c2].epnum][c * 4 + 2];
						patterncopybuffer[d * 4 + 3] = pattern[gt->editorInfo[c2].epnum][c * 4 + 3];
						if (jrawkey == KEY_X)
						{
							pattern[gt->editorInfo[c2].epnum][c * 4] = REST;
							pattern[gt->editorInfo[c2].epnum][c * 4 + 1] = 0;
							pattern[gt->editorInfo[c2].epnum][c * 4 + 2] = 0;
							pattern[gt->editorInfo[c2].epnum][c * 4 + 3] = 0;
						}
						d++;
					}
					patterncopyrows = d;
				}
			}
		}
		break;

	case KEY_V:
		if ((shiftpressed) && (patterncopyrows))
		{
			for (c = 0; c < patterncopyrows; c++)
			{
				if (editorInfo.eppos >= pattlen[gt->editorInfo[c2].epnum]) break;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4] = patterncopybuffer[c * 4];
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] = patterncopybuffer[c * 4 + 1];
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = patterncopybuffer[c * 4 + 2];
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = patterncopybuffer[c * 4 + 3];
				editorInfo.eppos++;
			}
		}
		break;

	case KEY_DEL:
		if (editorInfo.epmarkchn == editorInfo.epchn) editorInfo.epmarkchn = -1;
		if ((pattlen[gt->editorInfo[c2].epnum] - editorInfo.eppos) * 4 - 4 >= 0)
		{
			memmove(&pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4],
				&pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 4],
				(pattlen[gt->editorInfo[c2].epnum] - editorInfo.eppos) * 4 - 4);
			pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 4] = REST;
			pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 3] = 0x00;
			pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 2] = 0x00;
			pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 1] = 0x00;
		}
		else
		{
			if (editorInfo.eppos == pattlen[gt->editorInfo[c2].epnum])
			{
				if (pattlen[gt->editorInfo[c2].epnum] > 1)
				{
					pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 4] = ENDPATT;
					pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 3] = 0x00;
					pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 2] = 0x00;
					pattern[gt->editorInfo[c2].epnum][pattlen[gt->editorInfo[c2].epnum] * 4 - 1] = 0x00;
					countthispattern(gt);
					editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
				}
			}
		}
		break;

	case KEY_INS:
		if (editorInfo.epmarkchn == editorInfo.epchn) editorInfo.epmarkchn = -1;
		if ((pattlen[gt->editorInfo[c2].epnum] - editorInfo.eppos) * 4 - 4 >= 0)
		{
			memmove(&pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 4],
				&pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4],
				(pattlen[gt->editorInfo[c2].epnum] - editorInfo.eppos) * 4 - 4);
			pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4] = REST;
			pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] = 0x00;
			pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = 0x00;
			pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0x00;
		}
		else
		{
			if (editorInfo.eppos == pattlen[gt->editorInfo[c2].epnum])
			{
				if (pattlen[gt->editorInfo[c2].epnum] < MAX_PATTROWS)
				{
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4] = REST;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] = 0x00;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = 0x00;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0x00;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 4] = ENDPATT;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 5] = 0x00;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 6] = 0x00;
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 7] = 0x00;
					countthispattern(gt);
					editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
				}
			}
		}
		break;

	case KEY_SPACE:
		if (!shiftpressed)
		{
			recordmode ^= 1;
		}
		else
		{
			handleShiftSpace(gt, c2, editorInfo.eppos * 4, 0, 1);
		}
		break;

	case KEY_RIGHT:
		if (!shiftpressed)
		{
			int maxCh = 6;
			if (maxSIDChannels == 3 || (maxSIDChannels == 9 && (editorInfo.esnum & 1)))
				maxCh = 3;

			editorInfo.epcolumn++;
			if (editorInfo.epcolumn >= 6)
			{
				editorInfo.epcolumn = 0;
				editorInfo.epchn++;
				if (editorInfo.epchn >= maxCh)
					editorInfo.epchn = 0;
				if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
					editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];

				setMasterLoopChannel(gt);
			}

		}
		else
		{
			nextpattern(gt);
		}
		break;

	case KEY_LEFT:
		if (!shiftpressed)
		{
			editorInfo.epcolumn--;
			if (editorInfo.epcolumn < 0)
			{
				int maxCh = 6;
				if (maxSIDChannels == 3 || (maxSIDChannels == 9 && (editorInfo.esnum & 1)))
					maxCh = 3;

				editorInfo.epcolumn = 5;
				editorInfo.epchn--;
				if (editorInfo.epchn < 0) editorInfo.epchn = maxCh - 1;
				if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum]) editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
				setMasterLoopChannel(gt);

			}

		}
		else
		{
			prevpattern(gt);
		}
		break;

	case KEY_HOME:
		while (editorInfo.eppos != 0) patternup(gt);

		break;

	case KEY_END:
		while (editorInfo.eppos != pattlen[gt->editorInfo[c2].epnum]) patterndown(gt);

		break;

	case KEY_PGUP:
		for (scrrep = PGUPDNREPEAT; scrrep; scrrep--)
			patternup(gt);

		break;

	case KEY_PGDN:
		for (scrrep = PGUPDNREPEAT; scrrep; scrrep--)
			patterndown(gt);

		break;

	case KEY_UP:
		ret = patternup(gt);
		if (ret && lmanMode)
		{
			int songPat = songorder[editorInfo.esnum][c3][gt->editorInfo[c2].espos];	// pattern number at currently select order list channel
			if (songPat == gt->editorInfo[c2].epnum)	// pattern matches the editing pattern number.. So move to the next pattern
			{
				if (gt->editorInfo[c2].espos == 0)
				{
					gt->editorInfo[c2].espos = songlen[editorInfo.esnum][c3];
				}

				editorInfo.eseditpos = gt->editorInfo[c2].espos - 1;

				orderSelectPatternsFromSelected(gt);
				editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
				if (editorInfo.eseditpos < editorInfo.esview)
					editorInfo.esview = editorInfo.eseditpos;
				if (editorInfo.eseditpos - editorInfo.esview >= VISIBLEORDERLIST)
				{
					editorInfo.esview = editorInfo.eseditpos - VISIBLEORDERLIST + 1;
				}
			}
		}

		break;

	case KEY_DOWN:

		ret = patterndown(gt);
		if (ret && lmanMode)
		{
			int songPat = songorder[editorInfo.esnum][c3][gt->editorInfo[c2].espos];	// pattern number at currently select order list channel
			if (songPat == gt->editorInfo[c2].epnum)	// pattern matches the editing pattern number.. So move to the next pattern
			{
				if (gt->editorInfo[c2].espos == songlen[editorInfo.esnum][c3] - 1)
				{
					gt->editorInfo[c2].espos = -1;
				}

				editorInfo.eseditpos = gt->editorInfo[c2].espos + 1;

				orderSelectPatternsFromSelected(gt);
				if (editorInfo.eseditpos < editorInfo.esview)
					editorInfo.esview = editorInfo.eseditpos;
				if (editorInfo.eseditpos - editorInfo.esview >= VISIBLEORDERLIST)
				{
					editorInfo.esview = editorInfo.eseditpos - VISIBLEORDERLIST + 1;
				}
			}

		}

		break;

	case KEY_APOST2:
		if (!shiftpressed)
		{
			editorInfo.epchn++;
			if (editorInfo.epchn >= MAX_CHN) editorInfo.epchn = 0;
			if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum]) editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
		}
		else
		{
			editorInfo.epchn--;
			if (editorInfo.epchn < 0) editorInfo.epchn = MAX_CHN - 1;
			if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum]) editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
		}

		break;

	case KEY_1:
	case KEY_2:
	case KEY_3:
	case KEY_4:
	case KEY_5:
	case KEY_6:
		if (shiftpressed)
			mutechannel(jrawkey - KEY_1, gt);
		break;
	}
	if ((keypreset == KEY_DMC) && (hexnybble >= 0) && (hexnybble <= 7) && (!editorInfo.epcolumn))
	{
		int oldbyte = pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4];
		editorInfo.epoctave = hexnybble;
		if ((oldbyte >= FIRSTNOTE) && (oldbyte <= LASTNOTE))
		{
			int newbyte;
			int oldnote = (oldbyte - FIRSTNOTE) % 12;

			if (recordmode)
			{
				newbyte = oldnote + editorInfo.epoctave * 12 + FIRSTNOTE;
				if (newbyte <= LASTNOTE)
				{
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4] = newbyte;
				}
			}
			if ((recordmode) && (autoadvance < 1))
			{
				editorInfo.eppos++;
				if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
				{
					editorInfo.eppos = 0;
				}
			}
		}

	}

	if ((hexnybble >= 0) && (editorInfo.epcolumn) && (recordmode))
	{
		if (editorInfo.eppos < pattlen[gt->editorInfo[c2].epnum])
		{
			switch (editorInfo.epcolumn)
			{
			case 1:
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] &= 0x0f;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] |= hexnybble << 4;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] &= (MAX_INSTR - 1);
				break;

			case 2:
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] &= 0xf0;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] |= hexnybble;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 1] &= (MAX_INSTR - 1);
				break;

			case 3:
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2] = hexnybble;
				if (!pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0;
				break;

			case 4:
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] &= 0x0f;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] |= hexnybble << 4;
				if (!pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0;
				break;

			case 5:
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] &= 0xf0;
				pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] |= hexnybble;
				if (!pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 2])
					pattern[gt->editorInfo[c2].epnum][editorInfo.eppos * 4 + 3] = 0;
				break;
			}
		}
		if (autoadvance < 2)
		{
			editorInfo.eppos++;
			if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
			{
				editorInfo.eppos = 0;
			}
		}
	}
	editorInfo.epview = editorInfo.eppos - VISIBLEPATTROWS / 2;
}


int patterndown(GTOBJECT *gt)
{
	int ret = 0;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	if (shiftpressed)
	{
		if ((editorInfo.epmarkchn != editorInfo.epchn) || (editorInfo.eppos != editorInfo.epmarkend))
		{
			editorInfo.epmarkchn = editorInfo.epchn;
			editorInfo.epmarkstart = editorInfo.epmarkend = editorInfo.eppos;
		}
	}
	editorInfo.eppos++;
	if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
	{
		editorInfo.eppos = 0;
		ret = 1;
	}
	if (shiftpressed)
	{
		editorInfo.epmarkend = editorInfo.eppos;
		if (editorInfo.epmarkend == editorInfo.epmarkstart)
			editorInfo.epmarkchn = -1;

		editorInfo.highlightLoopChannel = 999;			// remove from display
		gt->interPatternLoopEnabledFlag = 0;		// disable in player
		editorInfo.highlightLoopPatternNumber = -1;
	}
	return ret;
}

int patternup(GTOBJECT *gt)
{
	int ret = 0;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	if (shiftpressed)
	{
		if ((editorInfo.epmarkchn != editorInfo.epchn) || (editorInfo.eppos != editorInfo.epmarkend))
		{
			editorInfo.epmarkchn = editorInfo.epchn;
			editorInfo.epmarkstart = editorInfo.epmarkend = editorInfo.eppos;
		}
	}
	editorInfo.eppos--;
	if (editorInfo.eppos < 0)
	{
		editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
		ret = 1;
	}
	if (shiftpressed)
	{
		editorInfo.epmarkend = editorInfo.eppos;
		if (editorInfo.epmarkend == editorInfo.epmarkstart)
			editorInfo.epmarkchn = -1;

		editorInfo.highlightLoopChannel = 999;			// remove from display
		gt->interPatternLoopEnabledFlag = 0;		// disable in player
		editorInfo.highlightLoopPatternNumber = -1;
	}

	return ret;
}


void prevpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	if (gt->editorInfo[c2].epnum > 0)
	{
		gt->editorInfo[c2].epnum--;
		if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
			editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
	}
	if (editorInfo.epchn == editorInfo.epmarkchn)
	{
		editorInfo.highlightLoopChannel = 999;
		editorInfo.epmarkchn = -1;
		gt->interPatternLoopEnabledFlag = 0;
	}

}


void nextpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	if (gt->editorInfo[c2].epnum < MAX_PATT - 1)
	{
		gt->editorInfo[c2].epnum++;
		if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
			editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];
	}

	if (editorInfo.epchn == editorInfo.epmarkchn)
	{
		gt->interPatternLoopEnabledFlag = 0;
		editorInfo.highlightLoopChannel = 999;
		editorInfo.epmarkchn = -1;
	}
}


void shrinkpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	int c = gt->editorInfo[c2].epnum;
	int l = pattlen[c];
	int nl = l / 2;
	int d;

	if (pattlen[c] < 2) return;

	stopsong(gt);

	for (d = 0; d < nl; d++)
	{
		pattern[c][d * 4] = pattern[c][d * 2 * 4];
		pattern[c][d * 4 + 1] = pattern[c][d * 2 * 4 + 1];
		pattern[c][d * 4 + 2] = pattern[c][d * 2 * 4 + 2];
		pattern[c][d * 4 + 3] = pattern[c][d * 2 * 4 + 3];
	}

	pattern[c][nl * 4] = ENDPATT;
	pattern[c][nl * 4 + 1] = 0;
	pattern[c][nl * 4 + 2] = 0;
	pattern[c][nl * 4 + 3] = 0;

	editorInfo.eppos /= 2;

	countthispattern(gt);
}

void expandpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);

	int c = gt->editorInfo[c2].epnum;
	int l = pattlen[c];
	int nl = l * 2;
	int d;
	unsigned char temp[MAX_PATTROWS * 4 + 4];

	if (nl > MAX_PATTROWS) return;
	memset(temp, 0, sizeof temp);

	stopsong(gt);

	for (d = 0; d <= nl; d++)
	{
		if (d & 1)
		{
			temp[d * 4] = REST;
			temp[d * 4 + 1] = 0;
			temp[d * 4 + 2] = 0;
			temp[d * 4 + 3] = 0;
		}
		else
		{
			temp[d * 4] = pattern[c][d * 2];
			temp[d * 4 + 1] = pattern[c][d * 2 + 1];
			temp[d * 4 + 2] = pattern[c][d * 2 + 2];
			temp[d * 4 + 3] = pattern[c][d * 2 + 3];
		}
	}

	memcpy(pattern[c], temp, (nl + 1) * 4);

	editorInfo.eppos *= 2;

	countthispattern(gt);
}

void splitpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);

	int c = gt->editorInfo[c2].epnum;
	int l = pattlen[c];
	int d;

	if (editorInfo.eppos == 0) return;
	if (editorInfo.eppos == l) return;

	stopsong(gt);

	if (insertpattern(c, gt))
	{
		int oldesnum = editorInfo.esnum;
		int oldeschn = editorInfo.eschn;
		int oldeseditpos = editorInfo.eseditpos;

		for (d = editorInfo.eppos; d <= l; d++)
		{
			pattern[c + 1][(d - editorInfo.eppos) * 4] = pattern[c][d * 4];
			pattern[c + 1][(d - editorInfo.eppos) * 4 + 1] = pattern[c][d * 4 + 1];
			pattern[c + 1][(d - editorInfo.eppos) * 4 + 2] = pattern[c][d * 4 + 2];
			pattern[c + 1][(d - editorInfo.eppos) * 4 + 3] = pattern[c][d * 4 + 3];
		}
		pattern[c][editorInfo.eppos * 4] = ENDPATT;
		pattern[c][editorInfo.eppos * 4 + 1] = 0;
		pattern[c][editorInfo.eppos * 4 + 2] = 0;
		pattern[c][editorInfo.eppos * 4 + 3] = 0;

		countpatternlengths();

		for (editorInfo.esnum = 0; editorInfo.esnum < MAX_SONGS; editorInfo.esnum++)
		{
			for (editorInfo.eschn = 0; editorInfo.eschn < MAX_CHN; editorInfo.eschn++)
			{
				for (editorInfo.eseditpos = 0; editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]; editorInfo.eseditpos++)
				{
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] == c)
					{
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = c + 1;
						insertorder(c, gt);
					}
				}
			}
		}
		editorInfo.eschn = oldeschn;
		editorInfo.eseditpos = oldeseditpos;
		editorInfo.esnum = oldesnum;
	}
}

void joinpattern(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);

	int c = gt->editorInfo[c2].epnum;
	int d;

	if (editorInfo.eschn != editorInfo.epchn) return;
	if (songorder[editorInfo.esnum][editorInfo.epchn][editorInfo.eseditpos] != c) return;
	d = songorder[editorInfo.esnum][editorInfo.epchn][editorInfo.eseditpos + 1];
	if (d >= MAX_PATT) return;
	if (pattlen[c] + pattlen[d] > MAX_PATTROWS) return;

	stopsong(gt);

	if (insertpattern(c, gt))
	{
		int oldesnum = editorInfo.esnum;
		int oldeschn = editorInfo.eschn;
		int oldeseditpos = editorInfo.eseditpos;
		int e, f;
		d++;

		for (e = 0; e < pattlen[c]; e++)
		{
			pattern[c + 1][e * 4] = pattern[c][e * 4];
			pattern[c + 1][e * 4 + 1] = pattern[c][e * 4 + 1];
			pattern[c + 1][e * 4 + 2] = pattern[c][e * 4 + 2];
			pattern[c + 1][e * 4 + 3] = pattern[c][e * 4 + 3];
		}
		for (f = 0; f < pattlen[d]; f++)
		{
			pattern[c + 1][e * 4] = pattern[d][f * 4];
			pattern[c + 1][e * 4 + 1] = pattern[d][f * 4 + 1];
			pattern[c + 1][e * 4 + 2] = pattern[d][f * 4 + 2];
			pattern[c + 1][e * 4 + 3] = pattern[d][f * 4 + 3];
			e++;
		}
		pattern[c + 1][e * 4] = ENDPATT;
		pattern[c + 1][e * 4 + 1] = 0;
		pattern[c + 1][e * 4 + 2] = 0;
		pattern[c + 1][e * 4 + 3] = 0;

		countpatternlengths();

		for (editorInfo.esnum = 0; editorInfo.esnum < MAX_SONGS; editorInfo.esnum++)
		{
			for (editorInfo.eschn = 0; editorInfo.eschn < MAX_CHN; editorInfo.eschn++)
			{
				for (editorInfo.eseditpos = 0; editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]; editorInfo.eseditpos++)
				{
					if ((songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] == c) && (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos + 1] == d))
					{
						deleteorder(gt);
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = c + 1;
					}
				}
			}
		}
		editorInfo.eschn = oldeschn;
		editorInfo.eseditpos = oldeseditpos;
		editorInfo.esnum = oldesnum;

		findusedpatterns();
		{
			int del1 = pattused[c];
			int del2 = pattused[d];

			if (!del1)
			{
				deletepattern(c, gt);
				if (d > c) d--;
			}
			if (!del2)
				deletepattern(d, gt);
		}
	}
}

void handleShiftSpace(GTOBJECT *gt, int c2, int startPatternPos, int follow, int enableLoop)
{
	// If the current patterns in pattern view match the patterns in the order list, sync to song order list
	// (This preserves the previously keyed on instruments)
	// However, if any of the patterns in the pattern editor do not match the order list, use the originan PLAY_PATTERN option.
	// 
	if (enableLoop)
	{
		for (int c = 0; c < maxSIDChannels; c++)
		{
			int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
			int pat = gt->editorInfo[c2].epnum;	// pattern number displayed along the top of the pattern
			if (pat != songorder[editorInfo.esnum][c2 % 6][gt->editorInfo[c2].espos])
			{

				initsongpos(editorInfo.esnum, PLAY_PATTERN, editorInfo.eppos, gt);
				followplay = follow;
				return;
			}
		}
	}

	orderPlayFromPosition(gt, startPatternPos, editorInfo.eseditpos, editorInfo.eschn);
	gt->loopEnabledFlag = enableLoop;	// Set this AFTER play starts, as init clears it.
	followplay = follow;
}

unsigned char keyStillDown[SDL_NUM_SCANCODES] = { 0 };
unsigned char keyNoteDown[SDL_NUM_SCANCODES] = { 0 };
int polyChannelOffset = 0;


int playingChannelOnKey[KEYBOARD_POLYPHONY] = { -1 };
int playingChannelStartTime[KEYBOARD_POLYPHONY] = { 0 };
int playingChannelNote[KEYBOARD_POLYPHONY];
int playingGTChannel[KEYBOARD_POLYPHONY];
int polyTimer = 0;
char keyOffsetText[100];	// = { 0 };
int noteOffsets[100];
int noteHeldLength;

int lastNoteDown = 0;

int MIDINotesHeld = 0;

int handleMIDIPolykeyboard(GTOBJECT *gt, MIDI_MESSAGE midiData)
{
	int i = 0;
	for (int c = 0;c < midiData.size / 3;c++)
	{
		unsigned char midiInstruction = midiData.message[i];
		unsigned char midiNote = midiData.message[i + 1];
		unsigned char midiVel = midiData.message[i + 2];
		i += 3;

		if (midiInstruction == 0x90 && midiVel > 0)	// key on
		{
			int note = midiNote + FIRSTNOTE;
			if (note <= LASTNOTE)
			{
				keyOn(midiNote, note, gt);
				MIDINotesHeld++;
			}


			return 0;
		}
		else if ((midiInstruction == 0x80) || (midiInstruction == 0x90 && midiVel == 0))
		{
			int note = midiNote + FIRSTNOTE;
			if (note <= LASTNOTE)
			{
				keyOff(note, gt);
				MIDINotesHeld--;
			}
			return 1;
		}
	}
}

int handlePolyphonicKeyboard(GTOBJECT *gt)
{

	int noKeysPressed = 1;
	int newnote = -1;
	int c = 0;

	if (recordmode)
		return noKeysPressed;

	if (shiftpressed)
		return noKeysPressed;


	if (editorInfo.editmode == EDIT_PATTERN)
	{
		for (c = 0; c < SDL_NUM_SCANCODES; c++)
		{
			newnote = -1;
			if (win_keystate[c])
			{
				newnote = getNote(c);	// Is user pressing valid QWERTY note key?
				if (newnote != -1)
				{
					noKeysPressed = 0;
					if (keyNoteDown[c] == 0)
					{
						keyOn(c, newnote, gt);		// for MIDI, just pass newnote to both..
						keyNoteDown[c]++;
					}
				}
			}
			else if (keyNoteDown[c] != 0)
			{
				keyNoteDown[c] = 0;
				newnote = getNoteFromChannel(c);
				if (newnote > -1)
					keyOff(newnote, gt);
			}
		}

		sprintf(&keyOffsetText[0], "                        ");

		if (!noKeysPressed)
		{
			win_disableKeyRepeat();		// key pressed. So disable key repeat for jam mode 

		}
	}

	return noKeysPressed;
}

void keyOn(int qwertyKey, int note, GTOBJECT* gt)
{
	int ch = findFreePolyChannel(note);
	playingChannelOnKey[ch] = qwertyKey;
	playingChannelStartTime[ch] = polyTimer;
	polyTimer++;

	playingGTChannel[ch] = ch;

	playingChannelNote[ch] = note;
	playtestnote(note, editorInfo.einum, ch, gt);


}


int getNoteFromChannel(int c)
{
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (playingChannelOnKey[i] == c)
			return playingChannelNote[i];
	}
	return -1;
}

void keyOff(int note, GTOBJECT *gt)
{
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (playingChannelNote[i] == note)
		{
			clearPolyChannel(i, gt);
		}
	}
}

int checkAnyPolyPlaying()
{
	int c = 0;
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (playingChannelOnKey[i] >= 0)
			c++;
	}
	return c;
}


void initPolyKeyboard()
{
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		playingChannelOnKey[i] = -1;
	}
}

int clearPolyChannel(int i, GTOBJECT *gt)
{

	if (playingChannelOnKey[i] != -1)
	{
		releasenote(playingGTChannel[i], gt);
		playingChannelOnKey[i] = -1;
		clearInfoLine = 1;
	}

	return clearInfoLine;	// Used to clear Info if any keys were pressed and are now not

}



int cc;
void calculateNoteOffsets()
{
	int lowestNote = -1;
	int note;
	int firstNote = -1;
	noteHeldLength = 0;

	sprintf(&keyOffsetText[0], "CHORD:                  ");
	cc++;

	int c = 0;
	do {
		note = findNote(lowestNote);
		if (note > -1)
		{
			lowestNote = note;	// next time around, find a note thats higher than this one..

			if (firstNote == -1)
				firstNote = note;
			noteOffsets[noteHeldLength] = note - firstNote;

			if (c == 0)
				sprintf(&keyOffsetText[c + 6], " %02X   ", noteOffsets[noteHeldLength]);
			else
				sprintf(&keyOffsetText[c + 6], ",%02X   ", noteOffsets[noteHeldLength]);
			c += 3;
			noteHeldLength++;
		}
	} while (note != -1);

	if (checkAnyPolyPlaying() == 1)	// only have one note pressed. So don't display chord
		sprintf(&keyOffsetText[0], "Note: %02X                                             ", firstNote);
}


int findNote(int lowestNote)
{
	int note = 9999;

	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (playingChannelOnKey[i] > -1)
		{
			if (playingChannelNote[i] > lowestNote && playingChannelNote[i] < note)
				note = playingChannelNote[i];
		}
	}
	if (note == 9999)
		return -1;		// reached end of held notes

	return note;
}

int checkMonoMode()
{
	for (int i = 0;i < 4;i++)
	{
		if (transportPolySIDEnabled[i])
			return 0;
	}
	return 1;	// All SID chips disabled for poly playback. So play mono on selected editor channel
}

int lastFoundChannel = 0;
int findFreePolyChannel(int note)
{
	if (checkMonoMode())
		return editorInfo.epchn;


	// Found channel playing the same note? retrigger using this channel
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (transportPolySIDEnabled[i / 3])
		{
			if (playingChannelOnKey[i] != -1)
			{
				if (playingChannelNote[i] == note)
					return i;
			}
		}
	}

	int c = lastFoundChannel;
	// Find a free channel
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (transportPolySIDEnabled[c / 3])
		{
			if (playingChannelOnKey[c] == -1)
			{
				lastFoundChannel = c + 1;
				lastFoundChannel %= KEYBOARD_POLYPHONY;
				return c;
			}
		}
		c++;
		c %= KEYBOARD_POLYPHONY;
	}

	// all channels are playing. Find oldest channel
	int oldest = 999999999;
	int oldestChannel = 0;
	for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
	{
		if (transportPolySIDEnabled[i / 3])
		{
			if (playingChannelOnKey[i] > -1 && playingChannelStartTime[i] < oldest)
			{
				oldest = playingChannelStartTime[i];
				oldestChannel = i;
			}
		}
	}

	lastFoundChannel = oldestChannel + 1;
	lastFoundChannel %= KEYBOARD_POLYPHONY;
	return oldestChannel;
}


int getNote(int rawkey)
{
	int newnote;
	int c = 0;

	newnote = -1;

	switch (keypreset)
	{
	case KEY_TRACKER:
		for (c = 0; c < sizeof(notekeytbl1); c++)
		{
			if (rawkey == notekeytbl1[c])
			{
				newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
			}
		}
		for (c = 0; c < sizeof(notekeytbl2); c++)
		{
			if (rawkey == notekeytbl2[c])
			{
				newnote = FIRSTNOTE + c + (editorInfo.epoctave + 1) * 12;
			}
		}
		break;

	case KEY_DMC:
		for (c = 0; c < sizeof(dmckeytbl); c++)
		{
			if (rawkey == dmckeytbl[c])
			{
				newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
			}
		}
		break;

	case KEY_JANKO:
		for (c = 0; c < sizeof(jankokeytbl1); c++)
		{
			if (rawkey == jankokeytbl1[c])
			{
				newnote = FIRSTNOTE + c + editorInfo.epoctave * 12;
			}
		}
		for (c = 0; c < sizeof(jankokeytbl2); c++)
		{
			if (rawkey == jankokeytbl2[c])
			{
				newnote = FIRSTNOTE + c + (editorInfo.epoctave + 1) * 12;
			}
		}
		break;
	}
	return newnote;
}


short getNoteFreq(int noteIndex)
{
	short freqHi = freqtblhi[noteIndex];
	freqHi <<= 8;
	freqHi |= freqtbllo[noteIndex];
	return freqHi;
}

void autoPitchbendToNextNote(GTOBJECT *gt)
{

	GTOBJECT *gte = &gtEditorObject;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12
	int songNum = getActualSongNumber(editorInfo.esnum, c2);

	int displayIndex = 0;

	int note1 = pattern[gt->editorInfo[c2].epnum][(editorInfo.eppos * 4) + 0];

	int needToFindNote1 = 0;
	if (note1 < FIRSTNOTE || note1 > LASTNOTE)
	{
		note1 = -1;
		needToFindNote1 = 1;
	}

	int found = 0;
	int ep = editorInfo.eseditpos;
	int lastPattPtr = 9234029;

	int firstPatternPos = editorInfo.eppos + 1;
	if (needToFindNote1)
		firstPatternPos = 0;

	for (int i = firstPatternPos; i < pattlen[gt->editorInfo[c2].epnum]; i++)
	{
		if (i < editorInfo.eppos + 1)
		{
			if (needToFindNote1)	// get last note prior to cursor position if there's no note at cursor
			{
				int note = pattern[gt->editorInfo[c2].epnum][(i * 4) + 0];
				if (note >= FIRSTNOTE && note <= LASTNOTE)
				{
					note1 = note;
				}
			}
		}
		else
		{
			if (note1 == -1)
				return;	// No notes before the editor cursor position in this pattern.

			int note2 = pattern[gt->editorInfo[c2].epnum][(i * 4) + 0];
			if (note2 >= FIRSTNOTE && note2 <= LASTNOTE)
			{
				note1 -= FIRSTNOTE;
				note2 -= FIRSTNOTE;

				jcounter = note1;

				getPlayStartPosition(gte, songNum, c2, ep, editorInfo.eppos);
				int tickCount1 = gte->chn[c2].portCounter;
				getPlayStartPosition(gte, songNum, c2, ep, i);
				int tickCount2 = gte->chn[c2].portCounter;



				int tickDiff = tickCount2 - tickCount1;
				int freq1 = getNoteFreq(note1);
				int freq2 = getNoteFreq(note2);


				char portaInstruction = 1;   // pitch bend up
				int freqDiff = freq2 - freq1;
				if (freq1 > freq2)
				{
					freqDiff = freq1 - freq2;
					portaInstruction = 2;	// pitch bend down
				}

				int newShortValue2 = freqDiff;

				freqDiff /= tickDiff;


				unsigned short newShortValue = (unsigned short)freqDiff;
				if (newShortValue == 0)
					return;

				int speedTableIndex = findfreespeedtable();
				if (speedTableIndex < 0)
					return;

				ltable[STBL][speedTableIndex] = (freqDiff & 0xff00) >> 8;
				rtable[STBL][speedTableIndex] = freqDiff & 0xff;

				// clear instruction + data between notes
				for (int a = editorInfo.eppos;a < i;a++)
				{
					pattern[gt->editorInfo[c2].epnum][(a * 4) + 2] = portaInstruction;
					pattern[gt->editorInfo[c2].epnum][(a * 4) + 3] = speedTableIndex + 1;
				}

				pattern[gt->editorInfo[c2].epnum][i * 4 + 2] = 3;	// legato on end note
				pattern[gt->editorInfo[c2].epnum][i * 4 + 3] = 0;
				return;
			}
		}
	}
}


void getPlayStartPosition(GTOBJECT *gte, int songNum, int c2, int songPos, int patternPos)
{
	// Now get number of tickets between note1 and note2

	int found = 0;
	int lastPattPtr = 423423;


	initsong(songNum, PLAY_BEGINNING, gte);	// JP FEB
	do {
		playroutine(gte);
		if (gte->songinit == PLAY_STOPPED)	// Error in song data
			return;

		if ((gte->chn[c2].songptr - 1 == songPos) && (gte->chn[c2].pattptr == patternPos * 4))
			found = 1;

		else if (found == 1 && lastPattPtr != gte->chn[c2].pattptr)
		{
			break;
		}

		lastPattPtr = gte->chn[c2].pattptr;

	} while (1);
}





