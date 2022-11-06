//
// GTUltra table editor
//

#define GTABLE_C

#include "goattrk2.h"

unsigned char ltablecopybuffer[MAX_TABLELEN];
unsigned char rtablecopybuffer[MAX_TABLELEN];
int tablecopyrows = 0;


void tablecommands(GTOBJECT *gt)
{
	int c;

	switch (rawkey)
	{
	case KEY_UP:
	case KEY_DOWN:
	case KEY_LEFT:
	case KEY_RIGHT:
		win_enableKeyRepeat();
		break;
	default:
		if (!enablekeyrepeat)
			win_disableKeyRepeat();
	}


	switch (rawkey)
	{
	case KEY_Q:
		if ((shiftOrCtrlPressed) && (editorInfo.etnum == STBL))
		{
			int speed = (ltable[editorInfo.etnum][editorInfo.etpos] << 8) | rtable[editorInfo.etnum][editorInfo.etpos];
			speed *= 34716;
			speed /= 32768;
			if (speed > 65535) speed = 65535;

			ltable[editorInfo.etnum][editorInfo.etpos] = speed >> 8;
			rtable[editorInfo.etnum][editorInfo.etpos] = speed & 0xff;
		}
		break;

	case KEY_A:
		if ((shiftOrCtrlPressed) && (editorInfo.etnum == STBL))
		{
			int speed = (ltable[editorInfo.etnum][editorInfo.etpos] << 8) | rtable[editorInfo.etnum][editorInfo.etpos];
			speed *= 30929;
			speed /= 32768;

			ltable[editorInfo.etnum][editorInfo.etpos] = speed >> 8;
			rtable[editorInfo.etnum][editorInfo.etpos] = speed & 0xff;
		}
		break;

	case KEY_W:
		if ((shiftOrCtrlPressed) && (editorInfo.etnum == STBL))
		{
			int speed = (ltable[editorInfo.etnum][editorInfo.etpos] << 8) | rtable[editorInfo.etnum][editorInfo.etpos];
			speed *= 2;
			if (speed > 65535) speed = 65535;

			ltable[editorInfo.etnum][editorInfo.etpos] = speed >> 8;
			rtable[editorInfo.etnum][editorInfo.etpos] = speed & 0xff;
		}
		if ((shiftOrCtrlPressed) && ((editorInfo.etnum == PTBL) || (editorInfo.etnum == FTBL)) && (ltable[editorInfo.etnum][editorInfo.etpos] < 0x80))
		{
			int speed = (signed char)(rtable[editorInfo.etnum][editorInfo.etpos]);
			speed *= 2;

			if (speed > 127) speed = 127;
			if (speed < -128) speed = -128;
			rtable[editorInfo.etnum][editorInfo.etpos] = speed;
		}
		break;

	case KEY_S:
		if (!ctrlpressed)
		{
			if ((shiftOrCtrlPressed) && (editorInfo.etnum == STBL))
			{
				int speed = (ltable[editorInfo.etnum][editorInfo.etpos] << 8) | rtable[editorInfo.etnum][editorInfo.etpos];
				speed /= 2;

				ltable[editorInfo.etnum][editorInfo.etpos] = speed >> 8;
				rtable[editorInfo.etnum][editorInfo.etpos] = speed & 0xff;
			}
			if ((shiftOrCtrlPressed) && ((editorInfo.etnum == PTBL) || (editorInfo.etnum == FTBL)) && (ltable[editorInfo.etnum][editorInfo.etpos] < 0x80))
			{
				int speed = (signed char)(rtable[editorInfo.etnum][editorInfo.etpos]);
				speed /= 2;

				rtable[editorInfo.etnum][editorInfo.etpos] = speed;
			}
		}
		break;

	case KEY_SPACE:
		if (!shiftOrCtrlPressed)
			playtestnote(FIRSTNOTE + editorInfo.epoctave * 12, editorInfo.einum, editorInfo.epchn, gt);
		else
			releasenote(editorInfo.epchn, gt);
		break;

	case KEY_RIGHT:
		if (!ctrlpressed)
		{
			editorInfo.etcolumn++;
			int maxC = 3;
			if (editorInfo.editTableMode == EDIT_TABLE_PULSE && editorInfo.editmode == EDIT_TABLES)
			{
				maxC = 4;
			}

			if (editorInfo.etcolumn > maxC)
			{
				disableEnterToReturnToLastPos = 1;

				editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
				editorInfo.etcolumn = 0;

				if (editorInfo.editTableMode == EDIT_TABLE_PULSE && editorInfo.editmode == EDIT_TABLES)
				{
					if (ltable[PTBL][editorInfo.etpos] < 0x80 || ltable[PTBL][editorInfo.etpos] >= 0xfe)
						editorInfo.etcolumn++;
				}

				editorInfo.etnum++;



				if (editorInfo.etnum >= MAX_TABLES)
					editorInfo.etnum = 0;

				editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
			}

			if (editorInfo.editTableMode == EDIT_TABLE_WAVE)
			{
				if (editorInfo.etnum != EDIT_TABLE_SPEED - 1 && editorInfo.etnum > 0)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = EDIT_TABLE_SPEED - 1;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}
			else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
			{
				if (editorInfo.etnum != FTBL)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = FTBL;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}
			else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
			{
				if (editorInfo.etnum != PTBL)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = PTBL;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}

			if (shiftpressed) editorInfo.etmarknum = -1;
		}
		break;

	case KEY_LEFT:
		if (!ctrlpressed)
		{
			editorInfo.etcolumn--;

			int maxC = 3;
			if (editorInfo.editTableMode == EDIT_TABLE_PULSE && editorInfo.editmode == EDIT_TABLES)
			{
				maxC = 4;
				if (ltable[PTBL][editorInfo.etpos] < 0x80 || ltable[PTBL][editorInfo.etpos] >= 0xfe)
				{
					if (editorInfo.etcolumn == 0)
						editorInfo.etcolumn--;
				}
			}

			if (editorInfo.etcolumn < 0)
			{
				disableEnterToReturnToLastPos = 1;
				editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
				editorInfo.etcolumn = maxC;
				editorInfo.etnum--;
				if (editorInfo.etnum < 0) editorInfo.etnum = MAX_TABLES - 1;
				editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
			}

			if (editorInfo.editTableMode == EDIT_TABLE_WAVE)
			{
				if (editorInfo.etnum != EDIT_TABLE_SPEED - 1 && editorInfo.etnum > 0)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = WTBL;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}
			else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
			{
				if (editorInfo.etnum != FTBL)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = FTBL;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}
			else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
			{
				if (editorInfo.etnum != PTBL)
				{
					editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
					editorInfo.etnum = PTBL;
					editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
				}
			}


			if (shiftpressed) editorInfo.etmarknum = -1;
		}
		break;

	case KEY_HOME:
		while (editorInfo.etpos != 0) tableup();
		break;

	case KEY_END:
		while (editorInfo.etpos != MAX_TABLELEN - 1) tabledown();
		break;

	case KEY_PGUP:
		for (c = 0; c < PGUPDNREPEAT; c++) tableup();
		break;

	case KEY_PGDN:
		for (c = 0; c < PGUPDNREPEAT; c++) tabledown();
		break;

	case KEY_UP:
		tableup();
		break;

	case KEY_DOWN:
		tabledown();
		break;

	case KEY_X:
	case KEY_C:
		if (shiftOrCtrlPressed)
		{
			if (editorInfo.etmarknum == -1)	// no table selected. copy single row under cursor
			{
				editorInfo.etmarknum = editorInfo.etnum;
				editorInfo.etmarkstart = editorInfo.etpos;
				editorInfo.etmarkend = editorInfo.etmarkstart;
			}

			if (editorInfo.etmarknum != -1)
			{
				int d = 0;
				if (editorInfo.etmarkstart <= editorInfo.etmarkend)
				{
					for (c = editorInfo.etmarkstart; c <= editorInfo.etmarkend; c++)
					{
						ltablecopybuffer[d] = ltable[editorInfo.etmarknum][c];
						rtablecopybuffer[d] = rtable[editorInfo.etmarknum][c];
						if (rawkey == KEY_X)
						{
							ltable[editorInfo.etmarknum][c] = 0;
							rtable[editorInfo.etmarknum][c] = 0;
						}
						d++;
					}
				}
				else
				{
					for (c = editorInfo.etmarkend; c <= editorInfo.etmarkstart; c++)
					{
						ltablecopybuffer[d] = ltable[editorInfo.etmarknum][c];
						rtablecopybuffer[d] = rtable[editorInfo.etmarknum][c];
						if (rawkey == KEY_X)
						{
							ltable[editorInfo.etmarknum][c] = 0;
							rtable[editorInfo.etmarknum][c] = 0;
						}
						d++;
					}
				}
				tablecopyrows = d;
			}
			editorInfo.etmarknum = -1;
		}
		break;

	case KEY_V:
		if (shiftOrCtrlPressed)
		{
			if (tablecopyrows)
			{
				for (c = 0; c < tablecopyrows; c++)
				{
					ltable[editorInfo.etnum][editorInfo.etpos] = ltablecopybuffer[c];
					rtable[editorInfo.etnum][editorInfo.etpos] = rtablecopybuffer[c];
					editorInfo.etpos++;
					if (editorInfo.etpos >= MAX_TABLELEN) editorInfo.etpos = MAX_TABLELEN - 1;
				}
			}
		}
		break;

	case KEY_O:
		if (shiftOrCtrlPressed) optimizetable(editorInfo.etnum);
		break;

	case KEY_U:
		if (shiftOrCtrlPressed)
		{
			editorInfo.etlock ^= 1;
			validatetableview();
			if (editorInfo.etlock)
				sprintf(infoTextBuffer, "Table Lock: Enabled");
			else
				sprintf(infoTextBuffer, "Table Lock: Disabled");
			forceInfoLine = 1;
		}
		break;

	case KEY_R:
		if (editorInfo.etnum == WTBL)
		{
			if (ltable[editorInfo.etnum][editorInfo.etpos] != 0xff)
			{
				// Convert absolute pitch to relative pitch or vice versa
				int basenote = editorInfo.epoctave * 12;
				int note = rtable[editorInfo.etnum][editorInfo.etpos];

				if (note >= 0x80)
				{
					note -= basenote;
					note &= 0x7f;
				}
				else
				{
					note += basenote;
					note |= 0x80;
				}

				rtable[editorInfo.etnum][editorInfo.etpos] = note;
			}
		}

	case KEY_L:
		if (editorInfo.etnum == PTBL)
		{
			int c;
			int currentpulse = -1;
			int targetpulse = ltable[editorInfo.etnum][editorInfo.etpos] << 4;
			int speed = rtable[editorInfo.etnum][editorInfo.etpos];
			int time;
			int steps;

			if (!speed) break;

			// Follow the chain of pulse commands backwards to the nearest set command so we know what current pulse is
			for (c = editorInfo.etpos - 1; c >= 0; c--)
			{
				if (ltable[editorInfo.etnum][c] == 0xff) break;
				if (ltable[editorInfo.etnum][c] >= 0x80)
				{
					currentpulse = (ltable[editorInfo.etnum][c] << 8) | rtable[editorInfo.etnum][c];
					currentpulse &= 0xfff;
					break;
				}
			}
			if (currentpulse == -1) break;

			// Then follow the chain of modulation steps
			for (; c < editorInfo.etpos; c++)
			{
				if (ltable[editorInfo.etnum][c] < 0x80)
				{
					currentpulse += ltable[editorInfo.etnum][c] * (rtable[editorInfo.etnum][c] & 0xff);
					if (rtable[editorInfo.etnum][c] >= 0x80) currentpulse -= 256 * ltable[editorInfo.etnum][c];
					currentpulse &= 0xfff;
				}
			}

			time = abs(targetpulse - currentpulse) / speed;
			if (speed < 128)
				steps = (time + 126) / 127;
			else
				steps = time;

			if (!steps) break;
			if (editorInfo.etpos + steps > MAX_TABLELEN) break;
			if (targetpulse < currentpulse) speed = -speed;

			// Make room in the table
			for (c = steps; c > 1; c--) inserttable(editorInfo.etnum, editorInfo.etpos, 1);

			while (time)
			{
				if (abs(speed) < 128)
				{
					if (time < 127) ltable[editorInfo.etnum][editorInfo.etpos] = time;
					else ltable[editorInfo.etnum][editorInfo.etpos] = 127;
					rtable[editorInfo.etnum][editorInfo.etpos] = speed;
					time -= ltable[editorInfo.etnum][editorInfo.etpos];
					editorInfo.etpos++;
				}
				else
				{
					currentpulse += speed;
					ltable[editorInfo.etnum][editorInfo.etpos] = 0x80 | ((currentpulse >> 8) & 0xf);
					rtable[editorInfo.etnum][editorInfo.etpos] = currentpulse & 0xff;
					time--;
					editorInfo.etpos++;
				}
			}
		}
		if (editorInfo.etnum == FTBL)
		{
			int c;
			int currentfilter = -1;
			int targetfilter = ltable[editorInfo.etnum][editorInfo.etpos];
			int speed = rtable[editorInfo.etnum][editorInfo.etpos] & 0x7f;
			int time;
			int steps;

			if (!speed) break;

			// Follow the chain of filter commands backwards to the nearest set command so we know what current pulse is
			for (c = editorInfo.etpos - 1; c >= 0; c--)
			{
				if (ltable[editorInfo.etnum][c] == 0xff) break;
				if (ltable[editorInfo.etnum][c] == 0x00)
				{
					currentfilter = rtable[editorInfo.etnum][c];
					break;
				}
			}
			if (currentfilter == -1) break;

			// Then follow the chain of modulation steps
			for (; c < editorInfo.etpos; c++)
			{
				if (ltable[editorInfo.etnum][c] < 0x80)
				{
					currentfilter += ltable[editorInfo.etnum][c] * rtable[editorInfo.etnum][c];
					currentfilter &= 0xff;
				}
			}

			time = abs(targetfilter - currentfilter) / speed;
			steps = (time + 126) / 127;
			if (!steps) break;
			if (editorInfo.etpos + steps > MAX_TABLELEN) break;
			if (targetfilter < currentfilter) speed = -speed;

			// Make room in the table
			for (c = steps; c > 1; c--) inserttable(editorInfo.etnum, editorInfo.etpos, 1);

			while (time)
			{
				if (time < 127) ltable[editorInfo.etnum][editorInfo.etpos] = time;
				else ltable[editorInfo.etnum][editorInfo.etpos] = 127;
				rtable[editorInfo.etnum][editorInfo.etpos] = speed;
				time -= ltable[editorInfo.etnum][editorInfo.etpos];
				editorInfo.etpos++;
			}
		}
		break;

	case KEY_N:
		if (shiftOrCtrlPressed)
		{
			switch (editorInfo.etnum)
			{
				// Negate pulse or filter speed
			case FTBL:
				if (!ltable[editorInfo.etnum][editorInfo.etpos]) break;
			case PTBL:
				if (ltable[editorInfo.etnum][editorInfo.etpos] < 0x80)
					rtable[editorInfo.etnum][editorInfo.etpos] = (rtable[editorInfo.etnum][editorInfo.etpos] ^ 0xff) + 1;
				break;

				// Negate relative note
			case WTBL:
				if ((ltable[editorInfo.etnum][editorInfo.etpos] != 0xff) && (rtable[editorInfo.etnum][editorInfo.etpos] < 0x80))
					rtable[editorInfo.etnum][editorInfo.etpos] = (0x80 - rtable[editorInfo.etnum][editorInfo.etpos]) & 0x7f;
				break;
			}
		}
		break;

	case KEY_DEL:
		deletetable(editorInfo.etnum, editorInfo.etpos);
		break;

	case KEY_INS:
		inserttable(editorInfo.etnum, editorInfo.etpos, shiftOrCtrlPressed);
		break;

	case KEY_ENTER:


		if (editorInfo.etnum == WTBL)
		{
			int table = -1;
			int mstmode = MST_PORTAMENTO;

			switch (ltable[editorInfo.etnum][editorInfo.etpos])
			{
			case WAVECMD + CMD_PORTAUP:
			case WAVECMD + CMD_PORTADOWN:
			case WAVECMD + CMD_TONEPORTA:
				table = STBL;
				break;

			case WAVECMD + CMD_VIBRATO:
				table = STBL;
				mstmode = editorInfo.finevibrato;
				break;

			case WAVECMD + CMD_FUNKTEMPO:
				table = STBL;
				mstmode = MST_FUNKTEMPO;
				break;

			case WAVECMD + CMD_SETPULSEPTR:
				table = PTBL;
				break;

			case WAVECMD + CMD_SETFILTERPTR:
				table = FTBL;
				break;
			}
			switch (table)
			{
			default:

				editorInfo.editmode = EDIT_INSTRUMENT;	// move back to instrument view if there's no other table to jump to
				editorInfo.eipos = editorInfo.etnum + 2;
				return;

			case STBL:
				if (rtable[editorInfo.etnum][editorInfo.etpos])
				{
					if (!shiftOrCtrlPressed)
					{

						allowEnterToReturnToPosition();
						gototable(STBL, rtable[editorInfo.etnum][editorInfo.etpos] - 1);
						return;
					}
					else
					{
						int oldeditpos = editorInfo.etpos;
						int oldeditcolumn = editorInfo.etcolumn;
						int pos = makespeedtable(rtable[editorInfo.etnum][editorInfo.etpos], mstmode, 1);
						allowEnterToReturnToPosition();
						gototable(WTBL, oldeditpos);
						editorInfo.etcolumn = oldeditcolumn;

						rtable[editorInfo.etnum][editorInfo.etpos] = pos + 1;
						return;
					}
				}
				else
				{
					int pos = findfreespeedtable();
					if (pos >= 0)
					{
						rtable[editorInfo.etnum][editorInfo.etpos] = pos + 1;
						allowEnterToReturnToPosition();
						gototable(STBL, pos);
						return;
					}
				}
				break;

			case PTBL:
			case FTBL:
				if (rtable[editorInfo.etnum][editorInfo.etpos])
				{
					allowEnterToReturnToPosition();
					gototable(table, rtable[editorInfo.etnum][editorInfo.etpos] - 1);
					return;
				}
				else
				{
					if (shiftOrCtrlPressed)
					{
						int pos = gettablelen(table);
						if (pos >= MAX_TABLELEN - 1) pos = MAX_TABLELEN - 1;
						rtable[editorInfo.etnum][editorInfo.etpos] = pos + 1;
						allowEnterToReturnToPosition();
						gototable(table, pos);
						return;
					}
				}
			}
		}
		else
		{
			if (!disableEnterToReturnToLastPos)
				memcpy((char*)&editorInfo, (char*)&editorInfoBackup, sizeof(EDITOR_INFO));
			return;
		}
		break;

	case KEY_APOST2:
		if (shiftOrCtrlPressed)
		{
			editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
			editorInfo.etnum--;
			if (editorInfo.etnum < 0) editorInfo.etnum = MAX_TABLES - 1;
			editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
		}
		else
		{
			editorInfo.etpos -= editorInfo.etview[editorInfo.etnum];
			editorInfo.etnum++;
			if (editorInfo.etnum >= MAX_TABLES)
				editorInfo.etnum = 0;

			editorInfo.etpos += editorInfo.etview[editorInfo.etnum];
		}
	}

	if (hexnybble >= 0)
	{
		if (editorInfo.editTableMode == EDIT_TABLE_WAVE && editorInfo.etnum == 0)
		{
			modifyWaveTableDetailed(hexnybble);

			editorInfo.etcolumn++;
			if (editorInfo.etcolumn > 3)
			{
				editorInfo.etcolumn = 2;
			}
			else if (editorInfo.etcolumn == 2)
				editorInfo.etcolumn = 0;
		}
		else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
		{
			modifyFilterTableDetailed(hexnybble);

			editorInfo.etcolumn++;
			if (editorInfo.etcolumn > 3)
			{
				editorInfo.etcolumn = 2;
			}
			else if (editorInfo.etcolumn == 2)
				editorInfo.etcolumn = 0;
		}
		else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
		{
			modifyPulseTableDetailed(hexnybble);

			editorInfo.etcolumn++;
			if (editorInfo.etcolumn > 4)
			{
				editorInfo.etcolumn = 3;
			}
			else if (editorInfo.etcolumn == 3)
				editorInfo.etcolumn = 0;
		}
		else
		{
			switch (editorInfo.etcolumn)
			{
			case 0:
				ltable[editorInfo.etnum][editorInfo.etpos] &= 0x0f;
				ltable[editorInfo.etnum][editorInfo.etpos] |= hexnybble << 4;
				break;
			case 1:
				ltable[editorInfo.etnum][editorInfo.etpos] &= 0xf0;
				ltable[editorInfo.etnum][editorInfo.etpos] |= hexnybble;
				break;
			case 2:
				rtable[editorInfo.etnum][editorInfo.etpos] &= 0x0f;
				rtable[editorInfo.etnum][editorInfo.etpos] |= hexnybble << 4;
				break;
			case 3:
				rtable[editorInfo.etnum][editorInfo.etpos] &= 0xf0;
				rtable[editorInfo.etnum][editorInfo.etpos] |= hexnybble;
				break;
			}

			editorInfo.etcolumn++;
			if (editorInfo.etcolumn > 3)
			{
				editorInfo.etcolumn = 0;
				editorInfo.etpos++;
				if (editorInfo.etpos >= MAX_TABLELEN) editorInfo.etpos = MAX_TABLELEN - 1;
			}
		}

	}

	validatetableview();
	setTableBackgroundColours(editorInfo.einum);
}

void deletetable(int num, int pos)
{
	int c, d;

	// Shift tablepointers in instruments
	for (c = 1; c < MAX_INSTR; c++)
	{
		if ((instr[c].ptr[num] - 1) > pos) instr[c].ptr[num]--;
	}

	// Shift tablepointers in wavetable commands
	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if ((ltable[WTBL][c] >= WAVECMD) && (ltable[WTBL][c] <= WAVELASTCMD))
		{
			int cmd = ltable[WTBL][c] & 0xf;

			if (num < STBL)
			{
				if (cmd == CMD_SETWAVEPTR + num)
				{
					if ((rtable[WTBL][c] - 1) > pos) rtable[WTBL][c]--;
				}
			}
			else
			{
				if ((cmd == CMD_FUNKTEMPO) || ((cmd >= CMD_PORTAUP) && (cmd <= CMD_VIBRATO)))
				{
					if ((rtable[WTBL][c] - 1) > pos) rtable[WTBL][c]--;
				}
			}
		}
	}

	// Shift tablepointers in patterns
	for (c = 0; c < MAX_PATT; c++)
	{
		for (d = 0; d <= MAX_PATTROWS; d++)
		{
			if (num < STBL)
			{
				if (pattern[c][d * 4 + 2] == CMD_SETWAVEPTR + num)
				{
					if ((pattern[c][d * 4 + 3] - 1) > pos) pattern[c][d * 4 + 3]--;
				}
			}
			else
			{
				if ((pattern[c][d * 4 + 2] == CMD_FUNKTEMPO) ||
					((pattern[c][d * 4 + 2] >= CMD_PORTAUP) && (pattern[c][d * 4 + 2] <= CMD_VIBRATO)))
				{
					if ((pattern[c][d * 4 + 3] - 1) > pos) pattern[c][d * 4 + 3]--;
				}
			}
		}
	}

	// Shift jumppointers in the table itself
	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if (num != STBL)
		{
			if (ltable[num][c] == 0xff)
				if ((rtable[num][c] - 1) > pos) rtable[num][c]--;
		}
	}

	for (c = pos; c < MAX_TABLELEN; c++)
	{
		if (c + 1 < MAX_TABLELEN)
		{
			ltable[num][c] = ltable[num][c + 1];
			rtable[num][c] = rtable[num][c + 1];
		}
		else
		{
			ltable[num][c] = 0;
			rtable[num][c] = 0;
		}
	}
}

void inserttable(int num, int pos, int mode)
{
	int c, d;

	// Shift tablepointers in instruments
	for (c = 1; c < MAX_INSTR; c++)
	{
		if (!mode)
		{
			if ((instr[c].ptr[num] - 1) >= pos) instr[c].ptr[num]++;
		}
		else
		{
			if ((instr[c].ptr[num] - 1) > pos) instr[c].ptr[num]++;
		}
	}

	// Shift tablepointers in wavetable commands
	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if ((ltable[WTBL][c] >= WAVECMD) && (ltable[WTBL][c] <= WAVELASTCMD))
		{
			int cmd = ltable[WTBL][c] & 0xf;

			if (num < STBL)
			{
				if (cmd == CMD_SETWAVEPTR + num)
				{
					if (!mode)
					{
						if ((rtable[WTBL][c] - 1) >= pos) rtable[WTBL][c]++;
					}
					else
					{
						if ((rtable[WTBL][c] - 1) > pos) rtable[WTBL][c]++;
					}
				}
			}
			else
			{
				if ((cmd == CMD_FUNKTEMPO) || ((cmd >= CMD_PORTAUP) && (cmd <= CMD_VIBRATO)))
				{
					if (!mode)
					{
						if ((rtable[WTBL][c] - 1) >= pos) rtable[WTBL][c]++;
					}
					else
					{
						if ((rtable[WTBL][c] - 1) > pos) rtable[WTBL][c]++;
					}
				}
			}
		}
	}


	// Shift tablepointers in patterns
	for (c = 0; c < MAX_PATT; c++)
	{
		for (d = 0; d <= MAX_PATTROWS; d++)
		{
			if (num < STBL)
			{
				if (pattern[c][d * 4 + 2] == CMD_SETWAVEPTR + num)
				{
					if (!mode)
					{
						if ((pattern[c][d * 4 + 3] - 1) >= pos) pattern[c][d * 4 + 3]++;
					}
					else
					{
						if ((pattern[c][d * 4 + 3] - 1) > pos) pattern[c][d * 4 + 3]++;
					}
				}
			}
			else
			{
				if ((pattern[c][d * 4 + 2] == CMD_FUNKTEMPO) ||
					((pattern[c][d * 4 + 2] >= CMD_PORTAUP) && (pattern[c][d * 4 + 2] <= CMD_VIBRATO)))
				{
					if (!mode)
					{
						if ((pattern[c][d * 4 + 3] - 1) >= pos) pattern[c][d * 4 + 3]++;
					}
					else
					{
						if ((pattern[c][d * 4 + 3] - 1) > pos) pattern[c][d * 4 + 3]++;
					}
				}
			}
		}
	}

	// Shift jumppointers in the table itself
	if (num != STBL)
	{
		for (c = 0; c < MAX_TABLELEN; c++)
		{
			if (ltable[num][c] == 0xff)
			{
				if (!mode)
				{
					if ((rtable[num][c] - 1) >= pos) rtable[num][c]++;
				}
				else
				{
					if ((rtable[num][c] - 1) > pos) rtable[num][c]++;
				}
			}
		}
	}

	for (c = MAX_TABLELEN - 1; c >= pos; c--)
	{
		if (c > pos)
		{
			ltable[num][c] = ltable[num][c - 1];
			rtable[num][c] = rtable[num][c - 1];
		}
		else
		{
			if ((num == WTBL) && (mode == 1))
			{
				ltable[num][c] = 0xe9;
				rtable[num][c] = 0;
			}
			else
			{
				ltable[num][c] = 0;
				rtable[num][c] = 0;
			}
		}
	}
}

int gettablelen(int num)
{
	int c;

	for (c = MAX_TABLELEN - 1; c >= 0; c--)
	{
		if (ltable[num][c] | rtable[num][c]) break;
	}
	return c + 1;
}

int gettablepartlen(int num, int pos)
{
	int c;

	if (pos < 0) return 0;
	if (num == STBL) return 1;

	for (c = pos; c < MAX_TABLELEN; c++)
	{
		if (ltable[num][c] == 0xff)
		{
			c++;
			break;
		}
	}
	return c - pos;
}

void optimizetable(int num)
{
	int c, d;

	memset(tableused, 0, sizeof tableused);

	for (c = 0; c < MAX_PATT; c++)
	{
		for (d = 0; d < pattlen[c]; d++)
		{
			if ((pattern[c][d * 4 + 2] >= CMD_SETWAVEPTR) && (pattern[c][d * 4 + 2] <= CMD_SETFILTERPTR))
				exectable(pattern[c][d * 4 + 2] - CMD_SETWAVEPTR, pattern[c][d * 4 + 3]);
			if ((pattern[c][d * 4 + 2] >= CMD_PORTAUP) && (pattern[c][d * 4 + 2] <= CMD_VIBRATO))
				exectable(STBL, pattern[c][d * 4 + 3]);
			if (pattern[c][d * 4 + 2] == CMD_FUNKTEMPO)
				exectable(STBL, pattern[c][d * 4 + 3]);
		}
	}

	for (c = 0; c < MAX_INSTR; c++)
	{
		for (d = 0; d < MAX_TABLES; d++)
		{
			exectable(d, instr[c].ptr[d]);
		}
	}

	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if (tableused[WTBL][c + 1])
		{
			if ((ltable[WTBL][c] >= WAVECMD) && (ltable[WTBL][c] <= WAVELASTCMD))
			{
				d = -1;

				switch (ltable[WTBL][c] - WAVECMD)
				{
				case CMD_PORTAUP:
				case CMD_PORTADOWN:
				case CMD_TONEPORTA:
				case CMD_VIBRATO:
					d = STBL;
					break;

				case CMD_SETPULSEPTR:
					d = PTBL;
					break;

				case CMD_SETFILTERPTR:
					d = FTBL;
					break;
				}

				if (d != -1) exectable(d, rtable[WTBL][c]);
			}
		}
	}

	for (c = MAX_TABLELEN - 1; c >= 0; c--)
	{
		if ((ltable[num][c]) || (rtable[num][c])) break;
	}
	for (; c >= 0; c--)
	{
		if (!tableused[num][c + 1]) deletetable(num, c);
	}
}

int makespeedtable(unsigned data, int mode, int makenew)
{
	int c;
	unsigned char l = 0, r = 0;

	if (!data) return -1;

	switch (mode)
	{
	case MST_NOFINEVIB:
		l = (data & 0xf0) >> 4;
		r = (data & 0x0f) << 4;
		break;

	case MST_FINEVIB:
		l = (data & 0x70) >> 4;
		r = ((data & 0x0f) << 4) | ((data & 0x80) >> 4);
		break;

	case MST_FUNKTEMPO:
		l = (data & 0xf0) >> 4;
		r = data & 0x0f;
		break;

	case MST_PORTAMENTO:
		l = (data << 2) >> 8;
		r = (data << 2) & 0xff;
		break;

	case MST_RAW:
		r = data & 0xff;
		l = data >> 8;
		break;
	}

	if (makenew == 0)
	{
		for (c = 0; c < MAX_TABLELEN; c++)
		{
			if ((ltable[STBL][c] == l) && (rtable[STBL][c] == r))
				return c;
		}
	}

	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if ((!ltable[STBL][c]) && (!rtable[STBL][c]))
		{
			ltable[STBL][c] = l;
			rtable[STBL][c] = r;

			settableview(STBL, c);
			return c;
		}
	}
	return -1;
}

void deleteinstrtable(int i)
{
	int c, d;
	int eraseok = 1;

	for (c = 0; c < MAX_TABLES; c++)
	{
		if (instr[i].ptr[c])
		{
			int pos = instr[i].ptr[c] - 1;
			int len = gettablepartlen(c, pos);

			// Check that this table area isn't used by another instrument
			for (d = 1; d < MAX_INSTR; d++)
			{
				if ((d != i) && (instr[d].ptr[c]))
				{
					int cmppos = instr[d].ptr[c] - 1;
					if ((cmppos >= pos) && (cmppos < pos + len)) eraseok = 0;
				}
			}
			if (eraseok)
				while (len--) deletetable(c, pos);
		}
	}
}

void gototable(int num, int pos)
{
	if (editorInfo.editmode == EDIT_PATTERN)
	{
		if (num == STBL)
		{
			if (editorInfo.editTableMode != EDIT_TABLE_NONE)
				editorInfo.editTableMode = EDIT_TABLE_WAVE;
		}
	}

	editorInfo.editmode = EDIT_TABLES;
	settableview(num, pos);
}

void settableview(int num, int pos)
{
	// If we're focusing on a table, continuefocus on the correct table
	if (editorInfo.editTableMode != EDIT_TABLE_NONE)
	{
		if (num != STBL)
			editorInfo.editTableMode = num + 1;
	}

	editorInfo.etnum = num;
	editorInfo.etcolumn = 0;
	editorInfo.etpos = pos;

	validatetableview();


}

void settableviewfirst(int num, int pos)
{
	editorInfo.etview[num] = pos;
	settableview(num, pos);
}
void validatetableview(void)
{
	if (editorInfo.etpos - editorInfo.etview[editorInfo.etnum] < 0)
		editorInfo.etview[editorInfo.etnum] = editorInfo.etpos;
	if (editorInfo.etpos - editorInfo.etview[editorInfo.etnum] >= VISIBLETABLEROWS)
		editorInfo.etview[editorInfo.etnum] = editorInfo.etpos - VISIBLETABLEROWS + 1;

	// Table view lock?
	if (editorInfo.etlock)
	{
		int c;

		for (c = 0; c < MAX_TABLES; c++) editorInfo.etview[c] = editorInfo.etview[editorInfo.etnum];
	}
}

void tableup(void)
{
	if (shiftOrCtrlPressed)
	{
		if ((editorInfo.etmarknum != editorInfo.etnum) || (editorInfo.etpos != editorInfo.etmarkend))
		{
			editorInfo.etmarknum = editorInfo.etnum;
			editorInfo.etmarkstart = editorInfo.etmarkend = editorInfo.etpos;
		}
	}
	editorInfo.etpos--;
	if (editorInfo.etpos < 0) editorInfo.etpos = 0;
	if (shiftOrCtrlPressed)
	{
		editorInfo.etmarkend = editorInfo.etpos;
		if (editorInfo.etmarkend == editorInfo.etmarkstart)
			editorInfo.etmarknum = -1;

	}
}

void tabledown(void)
{
	if (shiftOrCtrlPressed)
	{
		if ((editorInfo.etmarknum != editorInfo.etnum) || (editorInfo.etpos != editorInfo.etmarkend))
		{
			editorInfo.etmarknum = editorInfo.etnum;
			editorInfo.etmarkstart = editorInfo.etmarkend = editorInfo.etpos;
		}
	}
	editorInfo.etpos++;
	if (editorInfo.etpos >= MAX_TABLELEN) editorInfo.etpos = MAX_TABLELEN - 1;
	if (shiftOrCtrlPressed)
	{
		editorInfo.etmarkend = editorInfo.etpos;
		if (editorInfo.etmarkend == editorInfo.etmarkstart)
			editorInfo.etmarknum = -1;
	}
}

void exectable(int num, int ptr)
{
	// Jump error check
	if ((num != STBL) && (ptr) && (ptr <= MAX_TABLELEN))
	{
		if (ltable[num][ptr - 1] == 0xff)
		{
			tableerror = TYPE_JUMP;
			return;
		}
	}

	for (;;)
	{
		// Exit when table stopped
		if (!ptr) break;
		// Overflow check
		if ((num != STBL) && (ptr > MAX_TABLELEN))
		{
			tableerror = TYPE_OVERFLOW;
			break;
		}
		// If were already here, exit
		if (tableused[num][ptr]) break;
		// Mark current position used
		tableused[num][ptr] = 1;
		// Go to next ptr.
		if (num != STBL)
		{
			if (ltable[num][ptr - 1] == 0xff)
			{
				ptr = rtable[num][ptr - 1];
			}
			else ptr++;
		}
		else break;
	}
}

int findfreespeedtable(void)
{
	int c;
	for (c = 0; c < MAX_TABLELEN; c++)
	{
		if ((!ltable[STBL][c]) && (!rtable[STBL][c]))
		{
			return c;
		}
	}
	return -1;
}

/*
Handle user changing values in detailed wave table
*/
void modifyWaveTableDetailed(int hexnybble)
{
	if (editorInfo.etcolumn >= 2)
		modifyWaveTableDetailedRight(hexnybble);
	else
		modifyWaveTableDetailedLeft(hexnybble);
}

void modifyWaveTableDetailedLeft(int hexnybble)
{

	if (detailedTableBaseLValue[editorInfo.etpos] == -1)
		return;

	unsigned char v = detailedTableLValue[editorInfo.etpos];
	unsigned char o = v;

	switch (editorInfo.etcolumn)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}

	if ((unsigned char)v > detailedTableMaxLValue[editorInfo.etpos])
		v = detailedTableMaxLValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinLValue[editorInfo.etpos])
		v = detailedTableMinLValue[editorInfo.etpos];

	detailedTableLValue[editorInfo.etpos] = v;

	// Convert detailed value back to original table value
	v += detailedTableBaseLValue[editorInfo.etpos];

	if (ltable[0][editorInfo.etpos] == 0xff)
		rtable[0][editorInfo.etpos] = v;
	else
	{
		if (ltable[0][editorInfo.etpos] >= 0x10 && ltable[0][editorInfo.etpos] <= 0xef)	// waveform (not delay or command)
		{
			if (v < 0x10)
				v += 0xe0;		// remap 0-0xf to 0xe0-0xef			
		}

		ltable[0][editorInfo.etpos] = v;
	}
}

void modifyWaveTableDetailedRight(int hexnybble)
{
	unsigned char v = detailedTableRValue[editorInfo.etpos];
	unsigned char o = v;


	if (detailedTableBaseRValue[editorInfo.etpos] == -1)
		return;

	switch (editorInfo.etcolumn - 2)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}

	if ((unsigned char)v > detailedTableMaxRValue[editorInfo.etpos])
		v = detailedTableMaxRValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinRValue[editorInfo.etpos])
		v = detailedTableMinRValue[editorInfo.etpos];

	detailedTableRValue[editorInfo.etpos] = v;

	// Convert detailed R value back to original table value


	if (rtable[0][editorInfo.etpos] >= 0x60 && rtable[0][editorInfo.etpos] <= 0x7f)	// negative relative notes
	{
		int v2 = -v;
		v2 &= 0xff;

		int v3 = v2;

		v2 += 0x20;


		v = v2;
	}

	v += detailedTableBaseRValue[editorInfo.etpos];
	rtable[0][editorInfo.etpos] = v;

}

void modifyPulseTableDetailed(int hexnybble)
{
	if (ltable[PTBL][editorInfo.etpos] >= 0x80 && ltable[PTBL][editorInfo.etpos] <= 0xfe)
	{
		int v = detailedTableLValue[editorInfo.etpos];

		switch (editorInfo.etcolumn)
		{
		case 0:
			v &= 0x0ff;
			v |= hexnybble << 8;
			break;
		case 1:
			v &= 0xf0f;
			v |= hexnybble << 4;
			break;
		case 2:
			v &= 0xff0;
			v |= hexnybble;
			break;
		}

		ltable[PTBL][editorInfo.etpos] = ((v >> 8) & 0xf) | 0x80;
		rtable[PTBL][editorInfo.etpos] = v & 0xff;
	}
	else if (editorInfo.etcolumn >= 3)
		modifyPulseTableDetailedRight(hexnybble);
	else
		modifyPulseTableDetailedLeft(hexnybble);

}

void modifyPulseTableDetailedRight(int hexnybble)
{
	unsigned char v = detailedTableRValue[editorInfo.etpos];
	unsigned char o = v;

	if (detailedTableBaseRValue[editorInfo.etpos] == -1)
		return;

	int lv = ltable[PTBL][editorInfo.etpos];


	switch (editorInfo.etcolumn - 3)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}

	if ((unsigned char)v > detailedTableMaxRValue[editorInfo.etpos])
		v = detailedTableMaxRValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinRValue[editorInfo.etpos])
		v = detailedTableMinRValue[editorInfo.etpos];

	detailedTableRValue[editorInfo.etpos] = v;


	// Convert detailed R value back to original table value

	if (lv >= 1 && lv <= 0x7f)	// Modify pulse
	{
		if (rtable[PTBL][editorInfo.etpos] >= 0x80)	// currently holds negative value
		{
			v = 0x100 - v;
		}
	}

	v += detailedTableBaseRValue[editorInfo.etpos];
	rtable[PTBL][editorInfo.etpos] = v;

}

void modifyPulseTableDetailedLeft(int hexnybble)
{
	if (detailedTableBaseLValue[editorInfo.etpos] == -1)
		return;

	unsigned char v = detailedTableLValue[editorInfo.etpos];
	unsigned char o = v;

	switch (editorInfo.etcolumn - 1)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}


	if ((unsigned char)v > detailedTableMaxLValue[editorInfo.etpos])
		v = detailedTableMaxLValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinLValue[editorInfo.etpos])
		v = detailedTableMinLValue[editorInfo.etpos];

	detailedTableLValue[editorInfo.etpos] = v;

	// Convert detailed value back to original table value
	v += detailedTableBaseLValue[editorInfo.etpos];


	int lv = ltable[PTBL][editorInfo.etpos];
	if (lv == 0xff)
	{
		rtable[PTBL][editorInfo.etpos] = v;
	}
	else if (lv >= 1 && lv <= 0x7f)
	{
		ltable[PTBL][editorInfo.etpos] = v;
	}
}



void modifyFilterTableDetailed(int hexnybble)
{
	if (editorInfo.etcolumn >= 2)
		modifyFilterTableDetailedRight(hexnybble);
	else
		modifyFilterTableDetailedLeft(hexnybble);
}

int jt = 0;
void modifyFilterTableDetailedLeft(int hexnybble)
{
	if (detailedTableBaseLValue[editorInfo.etpos] == -1)
		return;


	unsigned char v = detailedTableLValue[editorInfo.etpos];
	unsigned char o = v;

	switch (editorInfo.etcolumn)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}


	if ((unsigned char)v > detailedTableMaxLValue[editorInfo.etpos])
		v = detailedTableMaxLValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinLValue[editorInfo.etpos])
		v = detailedTableMinLValue[editorInfo.etpos];

	detailedTableLValue[editorInfo.etpos] = v;

	// Convert detailed value back to original table value
	v += detailedTableBaseLValue[editorInfo.etpos];


	int lv = ltable[FTBL][editorInfo.etpos];
	if (lv == 0 || lv == 0xff)
	{
		rtable[FTBL][editorInfo.etpos] = v;
	}
	else if (lv >= 1 && lv <= 0x7f)	// modify filter cutoff
	{
		ltable[FTBL][editorInfo.etpos] = v;
	}
	else if (lv >= 0x80 && lv <= 0x0f0)	// modify filter resonance?
	{
		int rv = rtable[FTBL][editorInfo.etpos];
		rv &= 0xf;
		rv |= v << 4;
		rtable[FTBL][editorInfo.etpos] = rv;
	}
}

void modifyFilterTableDetailedRight(int hexnybble)
{
	unsigned char v = detailedTableRValue[editorInfo.etpos];
	unsigned char o = v;

	if (detailedTableBaseRValue[editorInfo.etpos] == -1)
		return;

	int lv = ltable[FTBL][editorInfo.etpos];


	switch (editorInfo.etcolumn - 2)
	{
	case 0:
		v &= 0x0f;
		v |= hexnybble << 4;
		break;
	case 1:
		v &= 0xf0;
		v |= hexnybble;
		break;
	}

	if ((unsigned char)v > detailedTableMaxRValue[editorInfo.etpos])
		v = detailedTableMaxRValue[editorInfo.etpos];

	if ((unsigned char)v < detailedTableMinRValue[editorInfo.etpos])
		v = detailedTableMinRValue[editorInfo.etpos];

	detailedTableRValue[editorInfo.etpos] = v;


	// Convert detailed R value back to original table value

	if (lv >= 1 && lv <= 0x7f)	// modify filter cutoff
	{
		if (rtable[FTBL][editorInfo.etpos] >= 0x80)	// currently holds negative value
		{
			v = 0x100 - v;
		}
	}

	v += detailedTableBaseRValue[editorInfo.etpos];
	rtable[FTBL][editorInfo.etpos] = v;

}

void allowEnterToReturnToPosition()
{
	disableEnterToReturnToLastPos = 0;
	memcpy((char*)&editorInfoBackup, (char*)&editorInfo, sizeof(EDITOR_INFO));
}


