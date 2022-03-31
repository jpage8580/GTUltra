//
// GTUltra orderlist & songname editor
//

#define GORDER_C

#include "goattrk2.h"

unsigned char trackcopybuffer[MAX_SONGLEN + 2];
int trackcopyrows = 0;
int trackcopywhole;
int trackcopyrpos;

void orderlistcommands(GTOBJECT *gt);
void namecommands(GTOBJECT *gt);
void paletteEditCommands();
void paletteLeft();
void paletteRight();



void orderlistcommands(GTOBJECT *gt)
{
	int c, scrrep;

	if (editPaletteMode)
	{
		paletteEditCommands();
		return;
	}

	int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12


	int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song
	int maxCh = 6;
	if ((maxSIDChannels == 3) || (maxSIDChannels == 9 && (editorInfo.esnum & 1)))
		maxCh = 3;

	if (hexnybble >= 0)
	{
		if (editorInfo.eseditpos != songlen[editorInfo.esnum][editorInfo.eschn])
		{
			switch (editorInfo.escolumn)
			{
			case 0:
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x0f;
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble << 4;
				if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
				{
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] >= MAX_PATT)
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = MAX_PATT - 1;
				}
				else
				{
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] >= MAX_SONGLEN)
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = MAX_SONGLEN - 1;
				}
				break;

			case 1:
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0xf0;
				if ((songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] & 0xf0) == 0xd0)
				{
					hexnybble--;
					if (hexnybble < 0) hexnybble = 0xf;
				}
				if ((songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] & 0xf0) == 0xe0)
				{
					hexnybble = 16 - hexnybble;
					hexnybble &= 0xf;
				}
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble;

				if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
				{
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] == LOOPSONG)
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = LOOPSONG - 1;
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] == TRANSDOWN)
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = TRANSDOWN + 0x0f;
				}
				else
				{
					if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] >= MAX_SONGLEN)
						songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = MAX_SONGLEN - 1;
				}
				break;
			}

			editorInfo.escolumn++;
			if (editorInfo.escolumn > 1)
			{
				editorInfo.escolumn = 0;
				if (editorInfo.eseditpos < (songlen[editorInfo.esnum][editorInfo.eschn] + 1))
				{
					editorInfo.eseditpos++;
					if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) editorInfo.eseditpos++;
				}
			}
		}
	}


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

	switch (key)
	{
	case 'R':
		if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
		{
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = REPEAT + 0x01;
			editorInfo.escolumn = 1;
		}
		break;

	case '+':
		if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
		{
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = TRANSUP;
			editorInfo.escolumn = 1;
		}
		break;

	case '-':
		if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
		{
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = TRANSDOWN + 0x0F;
			editorInfo.escolumn = 1;
		}
		break;

	case '>':
	case ')':
	case ']':
		nextsong(gt);
		break;

	case '<':
	case '(':
	case '[':
		prevsong(gt);
		break;
	}
	switch (rawkey)
	{
	case KEY_1:
	case KEY_2:
	case KEY_3:
	case KEY_4:
	case KEY_5:
	case KEY_6:
		if (shiftpressed)
		{
			int schn = editorInfo.eschn;
			int tchn = 0;

			editorInfo.esmarkchn = -1;
			if (rawkey == KEY_1) tchn = 0;
			if (rawkey == KEY_2) tchn = 1;
			if (rawkey == KEY_3) tchn = 2;
			if (rawkey == KEY_4) tchn = 3;
			if (rawkey == KEY_5) tchn = 4;
			if (rawkey == KEY_6) tchn = 5;
			if (schn != tchn)
			{
				int lentemp = songlen[editorInfo.esnum][schn];
				songlen[editorInfo.esnum][schn] = songlen[editorInfo.esnum][tchn];
				songlen[editorInfo.esnum][tchn] = lentemp;

				for (c = 0; c < MAX_SONGLEN + 2; c++)
				{
					unsigned char temp = songorder[editorInfo.esnum][schn][c];
					songorder[editorInfo.esnum][schn][c] = songorder[editorInfo.esnum][tchn][c];
					songorder[editorInfo.esnum][tchn][c] = temp;
				}
			}
		}
		break;

	case KEY_X:
		if (shiftpressed)
		{
			if (editorInfo.esmarkchn == -1)	// no table selected. copy single row under cursor
			{
				editorInfo.esmarkchn = editorInfo.eschn;
				editorInfo.esmarkstart = editorInfo.eseditpos;
				editorInfo.esmarkend = editorInfo.esmarkstart;
			}

			if (editorInfo.esmarkchn != -1)
			{
				int d = 0;

				editorInfo.eschn = editorInfo.esmarkchn;
				if (editorInfo.esmarkstart <= editorInfo.esmarkend)
				{
					editorInfo.eseditpos = editorInfo.esmarkstart;
					for (c = editorInfo.esmarkstart; c <= editorInfo.esmarkend; c++)
						trackcopybuffer[d++] = songorder[editorInfo.esnum][editorInfo.eschn][c];
					trackcopyrows = d;
				}
				else
				{
					editorInfo.eseditpos = editorInfo.esmarkend;
					for (c = editorInfo.esmarkend; c <= editorInfo.esmarkstart; c++)
						trackcopybuffer[d++] = songorder[editorInfo.esnum][editorInfo.eschn][c];
					trackcopyrows = d;
				}
				if (trackcopyrows == songlen[editorInfo.esnum][editorInfo.eschn])
				{
					trackcopywhole = 1;
					trackcopyrpos = songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1];
				}
				else trackcopywhole = 0;
				for (c = 0; c < trackcopyrows; c++) deleteorder(gt);
				editorInfo.esmarkchn = -1;
			}
		}
		break;

	case KEY_C:
		if (shiftpressed)
		{
			if (editorInfo.esmarkchn == -1)	// no table selected. copy single row under cursor
			{
				editorInfo.esmarkchn = editorInfo.eschn;
				editorInfo.esmarkstart = editorInfo.eseditpos;
				editorInfo.esmarkend = editorInfo.esmarkstart;
			}

			if (editorInfo.esmarkchn != -1)
			{
				int d = 0;
				if (editorInfo.esmarkstart <= editorInfo.esmarkend)
				{
					for (c = editorInfo.esmarkstart; c <= editorInfo.esmarkend; c++)
						trackcopybuffer[d++] = songorder[editorInfo.esnum][editorInfo.eschn][c];
					trackcopyrows = d;
				}
				else
				{
					for (c = editorInfo.esmarkend; c <= editorInfo.esmarkstart; c++)
						trackcopybuffer[d++] = songorder[editorInfo.esnum][editorInfo.eschn][c];
					trackcopyrows = d;
				}
				if (trackcopyrows == songlen[editorInfo.esnum][editorInfo.eschn])
				{
					trackcopywhole = 1;
					trackcopyrpos = songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1];
				}
				else trackcopywhole = 0;
				editorInfo.esmarkchn = -1;
			}
		}
		break;

	case KEY_V:
		if (shiftpressed)
		{
			int oldlen = songlen[editorInfo.esnum][editorInfo.eschn];

			if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
			{
				for (c = trackcopyrows - 1; c >= 0; c--)
					insertorder(trackcopybuffer[c], gt);
			}
			else
			{
				for (c = 0; c < trackcopyrows; c++)
					insertorder(trackcopybuffer[c], gt);
			}
			if ((trackcopywhole) && (!oldlen))
				songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1] = trackcopyrpos;
		}
		break;

	case KEY_L:
		if (shiftpressed)
		{
			if (editorInfo.esmarkchn == -1)
			{
				editorInfo.esmarkchn = editorInfo.eschn;
				editorInfo.esmarkstart = 0;
				editorInfo.esmarkend = songlen[editorInfo.esnum][editorInfo.eschn] - 1;
			}
			else editorInfo.esmarkchn = -1;
		}
		break;


	case KEY_SPACE:
		if (!shiftpressed)
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
				gt->editorInfo[c2].espos = editorInfo.eseditpos;
			if (gt->editorInfo[c2].esend < gt->editorInfo[c2].espos)
				gt->editorInfo[c2].esend = 0;

		}
		else
		{
			for (c = 0; c < maxSIDChannels; c++)
			{
				int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
				int songNum = getActualSongNumber(editorInfo.esnum, c2);
				int c3 = c2 % 6;

				if (editorInfo.eseditpos < songlen[songNum][c3])
					gt->editorInfo[c2].espos = editorInfo.eseditpos;
				if (gt->editorInfo[c2].esend < gt->editorInfo[c2].espos)
					gt->editorInfo[c2].esend = 0;

			}
		}
		break;

	case KEY_BACKSPACE:
		if (!shiftpressed)
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if ((gt->editorInfo[c2].esend != editorInfo.eseditpos) && (editorInfo.eseditpos > gt->editorInfo[c2].espos))
			{
				if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
					gt->editorInfo[c2].esend = editorInfo.eseditpos;
			}
			else
				gt->editorInfo[c2].esend = 0;
		}
		else
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if ((gt->editorInfo[c2].esend != editorInfo.eseditpos) && (editorInfo.eseditpos > gt->editorInfo[c2].espos))
			{
				for (c = 0; c < maxSIDChannels; c++)
				{
					int c3 = c % 6;
					int playingSong = getActualSongNumber(editorInfo.esnum, c);	// JP added this.
					c2 = getActualChannel(editorInfo.esnum, c);	// 0-12

					if (editorInfo.eseditpos < songlen[playingSong][c3])
						gt->editorInfo[c2].esend = editorInfo.eseditpos;
				}
			}
			else
			{
				for (c = 0; c < maxSIDChannels; c++)
					gt->editorInfo[c].esend = 0;
			}
		}
		break;

	case KEY_ENTER:
		if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
		{

			if (!shiftpressed)
			{
				int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

				if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < MAX_PATT)
					gt->editorInfo[c2].epnum = songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
			}
			else
			{
				int c, d;

				for (c = 0; c < maxSIDChannels; c++)
				{
					int start;

					int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
					int songNum = getActualSongNumber(editorInfo.esnum, c2);
					int c3 = c % 6;

					if (editorInfo.eseditpos != gt->editorInfo[c2].espos)
						start = editorInfo.eseditpos;
					else
						start = gt->editorInfo[c2].espos;

					for (d = start; d < songlen[songNum][c3]; d++)
					{
						if (songorder[songNum][c3][d] < MAX_PATT)
						{
							gt->editorInfo[c2].epnum = songorder[songNum][c3][d];
							break;
						}
					}
				}
			}
			editorInfo.epmarkchn = -1;
		}
		editorInfo.epchn = editorInfo.eschn;
		editorInfo.epcolumn = 0;
		editorInfo.eppos = 0;
		editorInfo.epview = -VISIBLEPATTROWS / 2;
		editorInfo.editmode = EDIT_PATTERN;
		if (editorInfo.epchn == editorInfo.epmarkchn) editorInfo.epmarkchn = -1;
		break;

	case KEY_DEL:
		editorInfo.esmarkchn = -1;
		deleteorder(gt);
		break;

	case KEY_INS:
		editorInfo.esmarkchn = -1;
		insertorder(0, gt);
		break;

	case KEY_HOME:
		if (songlen[editorInfo.esnum][editorInfo.eschn])
		{
			while ((editorInfo.eseditpos != 0) || (editorInfo.escolumn != 0)) orderleft();
		}
		break;

	case KEY_END:
		while (editorInfo.eseditpos != songlen[editorInfo.esnum][editorInfo.eschn] + 1) orderright();
		break;

	case KEY_PGUP:
		for (scrrep = PGUPDNREPEAT * 2; scrrep; scrrep--)
			orderleft();
		break;

	case KEY_PGDN:
		for (scrrep = PGUPDNREPEAT * 2; scrrep; scrrep--)
			orderright();
		break;

	case KEY_LEFT:
		orderleft();
		break;

	case KEY_RIGHT:
		orderright();
		break;

	case KEY_UP:
		editorInfo.eschn--;
		if (editorInfo.eschn < 0) editorInfo.eschn = maxCh - 1;
		if ((editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) || (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn] + 1))
		{
			editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			editorInfo.escolumn = 0;
		}
		setMasterLoopChannel(gt);
		if (shiftpressed) editorInfo.esmarkchn = -1;
		break;

	case KEY_DOWN:


		editorInfo.eschn++;
		if (editorInfo.eschn >= maxCh) editorInfo.eschn = 0;
		if ((editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) || (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn] + 1))
		{
			editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			editorInfo.escolumn = 0;
		}
		setMasterLoopChannel(gt);

		if (shiftpressed) editorInfo.esmarkchn = -1;
		break;
	}
	if (editorInfo.eseditpos - editorInfo.esview < 0)
	{
		editorInfo.esview = editorInfo.eseditpos;
	}
	if (editorInfo.eseditpos - editorInfo.esview >= VISIBLEORDERLIST)
	{
		editorInfo.esview = editorInfo.eseditpos - VISIBLEORDERLIST + 1;
	}
}

void namecommands(GTOBJECT *gt)
{
	switch (rawkey)
	{
	case KEY_DOWN:
	case KEY_ENTER:
		editorInfo.enpos++;
		if (editorInfo.enpos > 2) editorInfo.enpos = 0;
		break;

	case KEY_UP:
		editorInfo.enpos--;
		if (editorInfo.enpos < 0) editorInfo.enpos = 2;
		break;
	}

	editorInfo.enpos = editorInfo.nameIndex;

	switch (editorInfo.enpos)
	{
	case 0:
		editstring(songname, MAX_STR);
		break;

	case 1:
		editstring(authorname, MAX_STR);
		break;

	case 2:
		editstring(copyrightname, MAX_STR);
		break;
	}
}

void insertorder(unsigned char byte, GTOBJECT *gt)
{
	if ((songlen[editorInfo.esnum][editorInfo.eschn] - editorInfo.eseditpos) - 1 >= 0)
	{
		int len;
		if (songlen[editorInfo.esnum][editorInfo.eschn] < MAX_SONGLEN)
		{
			len = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			songorder[editorInfo.esnum][editorInfo.eschn][len + 1] =
				songorder[editorInfo.esnum][editorInfo.eschn][len];
			songorder[editorInfo.esnum][editorInfo.eschn][len] = LOOPSONG;
			if (len) songorder[editorInfo.esnum][editorInfo.eschn][len - 1] = byte;
			countthispattern(gt);
		}
		memmove(&songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos + 1],
			&songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos],
			(songlen[editorInfo.esnum][editorInfo.eschn] - editorInfo.eseditpos) - 1);
		songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = byte;
		len = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
		if ((songorder[editorInfo.esnum][editorInfo.eschn][len] > editorInfo.eseditpos) &&
			(songorder[editorInfo.esnum][editorInfo.eschn][len] < (len - 2)))
			songorder[editorInfo.esnum][editorInfo.eschn][len]++;
	}
	else
	{
		if (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn])
		{
			if (songlen[editorInfo.esnum][editorInfo.eschn] < MAX_SONGLEN)
			{
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos + 1] =
					songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = LOOPSONG;
				if (editorInfo.eseditpos) songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos - 1] = byte;
				countthispattern(gt);
				editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			}
		}
	}
}

void deleteorder(GTOBJECT *gt)
{
	if ((songlen[editorInfo.esnum][editorInfo.eschn] - editorInfo.eseditpos) - 1 >= 0)
	{
		int len;
		memmove(&songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos],
			&songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos + 1],
			(songlen[editorInfo.esnum][editorInfo.eschn] - editorInfo.eseditpos) - 1);
		songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] - 1] = 0x00;
		if (songlen[editorInfo.esnum][editorInfo.eschn] > 0)
		{
			songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] - 1] =
				songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn]];
			songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn]] =
				songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1];
			countthispattern(gt);
		}
		if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) editorInfo.eseditpos++;
		len = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
		if ((songorder[editorInfo.esnum][editorInfo.eschn][len] > editorInfo.eseditpos) &&
			(songorder[editorInfo.esnum][editorInfo.eschn][len] > 0))
			songorder[editorInfo.esnum][editorInfo.eschn][len]--;
	}
	else
	{
		if (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn])
		{
			if (songlen[editorInfo.esnum][editorInfo.eschn] > 0)
			{
				songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] - 1] =
					songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn]];
				songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn]] =
					songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1];
				countthispattern(gt);
				editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			}
		}
	}
}

void orderleft(void)
{
	if ((shiftpressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		if ((editorInfo.esmarkchn != editorInfo.eschn) || (editorInfo.eseditpos != editorInfo.esmarkend))
		{
			editorInfo.esmarkchn = editorInfo.eschn;
			editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
		}
	}
	editorInfo.escolumn--;
	if (editorInfo.escolumn < 0)
	{
		if (editorInfo.eseditpos > 0)
		{
			editorInfo.eseditpos--;
			if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) editorInfo.eseditpos--;
			editorInfo.escolumn = 1;
			if (editorInfo.eseditpos < 0)
			{
				editorInfo.eseditpos = 1;
				editorInfo.escolumn = 0;
			}
		}
		else editorInfo.escolumn = 0;
	}
	if ((shiftpressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		editorInfo.esmarkend = editorInfo.eseditpos;
		if (editorInfo.esmarkend == editorInfo.esmarkstart)
			editorInfo.esmarkchn = -1;
	}
}

void orderright(void)
{
	if ((shiftpressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		if ((editorInfo.esmarkchn != editorInfo.eschn) || (editorInfo.eseditpos != editorInfo.esmarkend))
		{
			editorInfo.esmarkchn = editorInfo.eschn;
			editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
		}
	}
	editorInfo.escolumn++;
	if (editorInfo.escolumn > 1)
	{
		editorInfo.escolumn = 0;
		if (editorInfo.eseditpos < (songlen[editorInfo.esnum][editorInfo.eschn] + 1))
		{

			editorInfo.eseditpos++;
			if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn])
				editorInfo.eseditpos++;
		}
		else editorInfo.escolumn = 1;
	}
	if ((shiftpressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		editorInfo.esmarkend = editorInfo.eseditpos;
		if (editorInfo.esmarkend == editorInfo.esmarkstart)
			editorInfo.esmarkchn = -1;
	}
}

void nextsong(GTOBJECT *gt)
{
	if (!editPaletteMode)
	{
		editorInfo.esnum++;
		if (editorInfo.esnum >= MAX_SONGS) editorInfo.esnum = MAX_SONGS - 1;
		songchange(gt, 0);

		setMasterLoopChannel(gt);
	}
	else
	{
		currentPalettePreset++;
		if (currentPalettePreset >= MAX_PALETTE_PRESETS)
			currentPalettePreset = MAX_PALETTE_PRESETS - 1;

		copyPaletteToOrderList(currentPalettePreset);
		setSkin(currentPalettePreset);
	}

}

void prevsong(GTOBJECT *gt)
{
	if (!editPaletteMode)
	{
		editorInfo.esnum--;
		if (editorInfo.esnum < 0) editorInfo.esnum = 0;
		songchange(gt, 0);
		setMasterLoopChannel(gt);
	}
	else
	{
		currentPalettePreset--;
		if (currentPalettePreset < 0)
			currentPalettePreset = 0;

		copyPaletteToOrderList(currentPalettePreset);
		setSkin(currentPalettePreset);
	}
}

int lastSong = -1;
void songchange(GTOBJECT *gt, int resetEditingPositions)
{
	int c;
	int s = editorInfo.esnum / 2;	// JP 9 or 12 channel song only



	editorInfo.highlightLoopChannel = 999;			// remove from display
	gt->interPatternLoopEnabledFlag = 0;		// disable in player
	editorInfo.highlightLoopPatternNumber = -1;
	editorInfo.highlightLoopStart = editorInfo.highlightLoopEnd = 0;

	if (maxSIDChannels <= 6)
		lastSong = s + 1;
	if (s != lastSong)
	{
		lastSong = s;

		if (resetEditingPositions)
		{
			for (c = 0; c < maxSIDChannels; c++)
			{
				gt->editorInfo[c].espos = 0;		// highlighted (green) position
				gt->editorInfo[c].esend = 0;		// end position (length of song)
				if (!editPaletteMode)
					gt->editorInfo[c].epnum = c;
			}

			editorInfo.eseditpos = 0;		// Reset cursor position in order list
			editorInfo.esview = 0;			// reset scroll position in order list
		}
		stopsong(gt);

		lastInfoPatternCh = -1;	// force text
		displayPatternInfo(gt);
	}

	int jc2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12 for currently selected channel in orderlist
	int jsongNum = getActualSongNumber(editorInfo.esnum, jc2);

	if ((maxSIDChannels == 3) || (maxSIDChannels == 9 && (editorInfo.esnum & 1)))
	{
		if (editorInfo.eschn >= 3)
			editorInfo.eschn = 2;
	}


	for (c = 0; c < maxSIDChannels; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int songNum = getActualSongNumber(editorInfo.esnum, c2);

		if (gt->editorInfo[c2].espos >= songlen[songNum][c2 % 6] + 1)
		{
			gt->editorInfo[c2].espos = songlen[songNum][c2 % 6] - 1;	//0;	//
			if (gt->editorInfo[c2].espos < 0)
				gt->editorInfo[c2].espos = 0;
		}

		if (c2 == jc2 && editorInfo.eseditpos > songlen[songNum][c2 % 6] + 1)	// +1 as we have the RPT text 
		{
			editorInfo.eseditpos = songlen[songNum][c2 % 6] - 1;	//0;
			if (editorInfo.eseditpos < 0)
				editorInfo.eseditpos = 0;
		}
	}

	orderSelectPatternsFromSelected(gt);

	editorInfo.eppos = 0;	// pattern pos
	if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn])
		editorInfo.eseditpos++;
	editorInfo.epmarkchn = -1;
	editorInfo.esmarkchn = -1;

	updateviewtopos(gt);

}

void updateviewtopos(GTOBJECT *gt)
{
	int c, d;

	if (editPaletteMode)
		return;

	for (c = 0; c < maxSIDChannels; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int songNum = getActualSongNumber(editorInfo.esnum, c2);
		int c3 = c % 6;

		for (d = gt->editorInfo[c2].espos; d < songlen[songNum][c3]; d++)
		{
			if (songorder[songNum][c3][d] < MAX_PATT)
			{
				gt->editorInfo[c2].epnum = songorder[songNum][c3][d];
				break;
			}
			else
			{
				if (gt->editorInfo[c2].epnum >= MAX_PATT)	// JP added this. Handles 12 channel songs where there's no data in odd song number
					gt->editorInfo[c2].epnum = 0;
			}
		}

		if (songlen[songNum][c3] == 0)
		{
			gt->editorInfo[c2].epnum = 0;
		}

	}
}

int tempPatternMin = 0;
int tempPatternSec = 0;
int tempPatterFrame = 0;


int addOrRemoveInterPatternLoop()
{
	GTOBJECT *gtPlayer = &gtObject;
	int removeLoop = 0;

	int c3 = getActualChannel(editorInfo.esnum, editorInfo.epmarkchn);

	int markStart = editorInfo.epmarkstart;
	int markEnd = editorInfo.epmarkend;
	if (markEnd < markStart)
	{
		markStart = markEnd;
		markEnd = editorInfo.epmarkstart;
	}
	if (editorInfo.epmarkstart == editorInfo.epmarkend)
		removeLoop = 1;

	if (editorInfo.highlightLoopStart == markStart && editorInfo.highlightLoopEnd == markEnd &&
		editorInfo.highlightLoopChannel == c3 &&
		editorInfo.highlightLoopPatternNumber == gtPlayer->editorInfo[c3].epnum)
		removeLoop = 1;

	if (removeLoop)
	{
		editorInfo.highlightLoopChannel = 999;			// remove from display
		gtPlayer->interPatternLoopEnabledFlag = 0;		// disable in player
		editorInfo.highlightLoopPatternNumber = -1;
		return -1;
	}
	else
	{
		editorInfo.highlightLoopPatternNumber == gtPlayer->editorInfo[c3].epnum;	// highlight looping area	
	}
	return 0;

}


int calcStartofInterPatternLoop(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop)
{
	GTOBJECT *gtPlayer = &gtObject;

	int c3 = getActualChannel(songNum, editorInfo.epmarkchn);

	int markStart = editorInfo.epmarkstart;
	int markEnd = editorInfo.epmarkend;
	if (markEnd < markStart)
	{
		markStart = markEnd;
		markEnd = editorInfo.epmarkstart;
	}


	int c2 = channelNum;	//getActualChannel(songNum, channelNum);
	int sng = songNum;	//getActualSongNumber(songNum, c2);

	if (c2 >= maxSIDChannels)
		return -1;

	gtloop->loopEnabledFlag = 0;
	initsong(sng, PLAY_BEGINNING, gtloop);

	do {
		playroutine(gtloop);

		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

	} while (gtloop->chn[c2].songptr <= startSongPos);

	int tempMin = gtloop->timemin;
	int tempSec = gtloop->timesec;
	int tempFrame = gtloop->timeframe;

	// Now get Select Start, Select End and Play Start ..Then get pattern end

	int findPatternLoopStart = 0;
	int findPatternLoopEnd = 0;
	int loopPatternNum = 0;

	// Now sync to end of pattern (info used for looping)
	int sptr = gtloop->chn[c2].songptr;

	do {
		playroutine(gtloop);
		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

		if (findPatternLoopStart == 0 && gtloop->chn[c3].pattptr == markStart * 4)
		{
			findPatternLoopStart = 1;
			memcpy((char*)&gtPlayer->patternLoopStartChn[0], (char*)&gtloop->chn[0], sizeof(CHN)*MAX_PLAY_CH);
			memcpy((char *)&gtPlayer->looptimemin, (char*)&gtloop->timemin, sizeof(int) * 3);
			tempPatternMin = gtloop->timemin;
			tempPatternSec = gtloop->timesec;
			tempPatterFrame = gtloop->timeframe;
			loopPatternNum = gtloop->chn[c3].pattnum;

			editorInfo.highlightLoopStart = markStart;
			editorInfo.highlightLoopEnd = markEnd;
			editorInfo.highlightLoopPatternNumber = loopPatternNum;
			editorInfo.highlightLoopChannel = c3;

			return 1;
		}

	} while (gtloop->chn[c2].songptr == sptr);

	return 0;
}


int calculateLoopInfo2(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop)
{

	GTOBJECT *gtPlayer = &gtObject;

	int c3 = getActualChannel(songNum, editorInfo.epmarkchn);

	int markStart = editorInfo.epmarkstart;
	int markEnd = editorInfo.epmarkend;
	if (markEnd < markStart)
	{
		markStart = markEnd;
		markEnd = editorInfo.epmarkstart;
	}


	int c2 = channelNum;
	int sng = songNum;

	if (c2 >= maxSIDChannels)
		return -1;

	gtloop->loopEnabledFlag = 0;
	initsong(sng, PLAY_BEGINNING, gtloop);

	do {
		playroutine(gtloop);

		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

	} while (gtloop->chn[c2].songptr <= startSongPos);


	memcpy((char *)&gtPlayer->loopStartChn[0], (char*)&gtloop->chn[0], sizeof(CHN)*MAX_PLAY_CH);

	memcpy((char *)&gtPlayer->looptimemin, (char*)&gtloop->timemin, sizeof(int) * 3);


	int tempMin = gtloop->timemin;
	int tempSec = gtloop->timesec;
	int tempFrame = gtloop->timeframe;

	// Now get Select Start, Select End and Play Start ..Then get pattern end

	int findPatternLoopStart = 0;
	int findPatternLoopEnd = 0;
	int loopPatternNum = 0;

	// Now sync to end of pattern (info used for looping)
	int sptr = gtloop->chn[c2].songptr;

	int quitloop = 0;
	do {
		playroutine(gtloop);
		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;


		if (gtloop->chn[c2].loopCount)
			quitloop++;
		else if (gtloop->chn[c2].songptr != sptr)
			quitloop++;

	} while (!quitloop);	//gtloop->chn[c2].songptr == sptr);

	memcpy((char*)&gtPlayer->loopEndChn[0], (char*)&gtloop->chn[0], sizeof(CHN)*MAX_PLAY_CH);

	return 0;
}

void orderPlayFromPosition(GTOBJECT *gt, int startPatternPos, int startSongPos, int focusChannel)
{

	if (startSongPos < songlen[editorInfo.esnum][focusChannel])
	{
		int c2 = getActualChannel(editorInfo.esnum, focusChannel);
		int sng = getActualSongNumber(editorInfo.esnum, c2);

		if (c2 >= maxSIDChannels)
			return;

		stopsong(gt);
		bypassPlayRoutine = 1;	// Stop interrupt from updating play routine. We're going to do it manually
		SDL_Delay(50);

		int loopMode = gt->loopEnabledFlag;


		int ep = startSongPos;
		int ep2;
		do
		{
			ep2 = ep;
			if ((songorder[sng][c2 % 6][ep] >= REPEAT) && (songorder[sng][c2 % 6][ep] < TRANSDOWN))
				ep++;
			if ((songorder[sng][c2 % 6][ep] >= TRANSDOWN) && (songorder[sng][c2 % 6][ep] < LOOPSONG))
				ep++;
		} while (ep != ep2);

		initsong(editorInfo.esnum, PLAY_BEGINNING, gt);
		gt->loopEnabledFlag = 0;
		gt->disableLoopSearch = 1;


		do {
			playroutine(gt);
			if (gt->songinit == PLAY_STOPPED)	// Error in song data
				return;

			if (gt->chn[c2].songptr >= startSongPos - 1)
				JPSoundMixer(NULL, 1024);

		} while (gt->chn[c2].songptr <= ep);

		int tempMin = gt->timemin;
		int tempSec = gt->timesec;
		int tempFrame = gt->timeframe;

		// Now sync to pattern start position (where cursor was when F3 was pressed)
		if (startPatternPos > 0)
		{
			while (gt->chn[c2].pattptr < startPatternPos) {
				playroutine(gt);
				if (gt->songinit == PLAY_STOPPED)	// Error in song data
					return;

				if (gt->chn[c2].songptr >= startSongPos - 1)
					JPSoundMixer(NULL, 1024);
			};

			tempMin = gt->timemin;
			tempSec = gt->timesec;
			tempFrame = gt->timeframe;
		}

		if (!gtObject.interPatternLoopEnabledFlag)
		{
			gt->timemin = tempMin;
			gt->timesec = tempSec;
			gt->timeframe = tempFrame;
		}
		else
		{
			gt->timemin = tempPatternMin;
			gt->timesec = tempPatternSec;
			gt->timeframe = tempPatterFrame;
		}

		bypassPlayRoutine = 0;

		gt->loopEnabledFlag = loopMode;
		gt->disableLoopSearch = 0;
	}
}

int jcnt = 0;
void orderSelectPatternsFromSelected(GTOBJECT *gt)
{
	if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
	{
		int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);
		int sng = getActualSongNumber(editorInfo.esnum, c2);

		if (c2 >= maxSIDChannels)
			return;

		int ep = editorInfo.eseditpos;

		int ep2;

		if (ep >= 0)
		{
			GTOBJECT *gte = &gtEditorObject;
			initsong(sng, PLAY_BEGINNING, gte);	// JP FEB
			do {
				playroutine(gte);
				if (gte->songinit == PLAY_STOPPED)	// Error in song data
					return;

			} while (gte->chn[c2].songptr - 1 < ep);

			for (int c = 0; c < maxSIDChannels; c++)
			{
				c2 = c;
				int sng = getActualSongNumber(editorInfo.esnum, c2);

				ep = gte->chn[c2].songptr - 1;	// -1;

				do
				{
					ep2 = ep;
					if ((songorder[sng][c2 % 6][ep] >= REPEAT) && (songorder[sng][c2 % 6][ep] < TRANSDOWN))
						ep++;
					if ((songorder[sng][c2 % 6][ep] >= TRANSDOWN) && (songorder[sng][c2 % 6][ep] < LOOPSONG))
						ep++;
				} while (ep != ep2);

				gt->editorInfo[c2].epnum = songorder[sng][c2 % 6][ep];
				gt->editorInfo[c2].espos = ep;
			}
		}
		else
		{
			for (int c = 0; c < maxSIDChannels; c++)
			{
				c2 = c;
				int sng = getActualSongNumber(editorInfo.esnum, c2);
				gt->editorInfo[c2].epnum = songorder[sng][c2 % 6][0];
				gt->editorInfo[c2].espos = 0;
			}
		}

		editorInfo.epview = -VISIBLEPATTROWS / 2;
		editorInfo.eppos = 0;
	}
}


int pattInstrumentCount[MAX_PATT][MAX_INSTR];
int instrumentCount[MAX_INSTR];
int firstInstrumentPattern[MAX_INSTR];
int patternChecked[MAX_PATT];

void countInstruments()
{
	for (int p = 0;p < MAX_PATT;p++)
	{
		patternChecked[p] = 0;

		for (int i = 0;i < MAX_INSTR;i++)
		{
			pattInstrumentCount[p][i] = 0;
		}
	}

	for (int s = 0;s < MAX_SONGS;s++)
	{
		for (int c = 0;c < MAX_CHN;c++)
		{
			for (int l = 0;l < songlen[s][c];l++)
			{
				int pat = songorder[s][c][l];
				if (pat >= REPEAT && pat < TRANSDOWN)
					continue;
				if (pat >= TRANSDOWN && pat <= LOOPSONG)
					continue;
				if (!patternChecked[pat])
				{
					countInstrumentsInPattern(pat);
					patternChecked[pat]++;
				}
			}
		}
	}

	calculateTotalInstrumentsFromAllPatterns();
}


void calculateTotalInstrumentsFromAllPatterns()
{
	for (int i = 0;i < MAX_INSTR;i++)
	{
		instrumentCount[i] = 0;
		firstInstrumentPattern[i] = -1;
	}

	for (int p = 0;p < MAX_PATT;p++)
	{
		for (int i = 0;i < MAX_INSTR;i++)
		{
			instrumentCount[i] += pattInstrumentCount[p][i];
		}
	}
}

void countInstrumentsInPattern(int pat)
{
	for (int i = 0;i < MAX_INSTR;i++)
	{
		pattInstrumentCount[pat][i] = 0;
	}

	for (int p = 0;p < pattlen[pat];p++)
	{
		int instr = pattern[pat][(p * 4) + 1];
		if (instr != 0)
		{
			pattInstrumentCount[pat][instr]++;
		}
	}

}


void paletteLeft()
{

	editorInfo.escolumn--;
	if (editorInfo.escolumn < 0)
	{
		editorInfo.escolumn = 1;
		if (editorInfo.eseditpos > 0)
		{
			editorInfo.eseditpos--;
		}
		else editorInfo.escolumn = 0;
	}
}

void paletteRight()
{

	int maxPaletteEntries = getPaletteTextArraySize();
	editorInfo.escolumn++;
	if (editorInfo.escolumn > 1)
	{
		editorInfo.escolumn = 0;
		if (editorInfo.eseditpos < (maxPaletteEntries / 2) - 1)
		{
			editorInfo.eseditpos++;
		}
		else editorInfo.escolumn = 1;
	}
}


void paletteEditCommands()
{

	if (hexnybble >= 0)
	{
		int lastEditPos = editorInfo.eseditpos;
		switch (editorInfo.escolumn)
		{
		case 0:
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x0f;
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble << 4;
			break;

		case 1:
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0xf0;
			songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble;
			break;
		}

		editorInfo.escolumn++;
		if (editorInfo.escolumn > 1)
		{
			editorInfo.escolumn = 0;
			if (editorInfo.eseditpos < MAX_PALETTE_ENTRIES)
			{
				editorInfo.eseditpos++;

			}
		}

		setPaletteRGB(currentPalettePreset, lastEditPos, songorder[editorInfo.esnum][0][lastEditPos], songorder[editorInfo.esnum][1][lastEditPos], songorder[editorInfo.esnum][2][lastEditPos]);

		paletteChanged = 1;

	}


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
	case KEY_LEFT:
		paletteLeft();
		break;

	case KEY_RIGHT:
		paletteRight();
		break;

	case KEY_UP:
		if (editorInfo.eschn > 0)
			editorInfo.eschn--;
		break;
	case KEY_DOWN:
		if (editorInfo.eschn < 2)
			editorInfo.eschn++;
		break;
	}

	if (editorInfo.eseditpos - editorInfo.esview < 0)
	{
		editorInfo.esview = editorInfo.eseditpos;
	}
	if (editorInfo.eseditpos - editorInfo.esview >= VISIBLEORDERLIST)
	{
		editorInfo.esview = editorInfo.eseditpos - VISIBLEORDERLIST + 1;
	}
}

void setMasterLoopChannel(GTOBJECT *gt)
{
	int loopChannel = -1;
	if (editorInfo.editmode == EDIT_PATTERN)
		loopChannel = editorInfo.epchn;
	else if (editorInfo.editmode == EDIT_ORDERLIST)
		loopChannel = editorInfo.eschn;

	if (loopChannel >= 0)
	{
		int c2 = getActualChannel(editorInfo.esnum, loopChannel);
		if (gt->songinit == PLAY_STOPPED)
		{
			gt->masterLoopChannel = c2;
		}
	}
}


void resetOrderView(GTOBJECT *gt)
{
	if (gt->chn[gt->masterLoopChannel].songptr <= editorInfo.esview)
	{

		editorInfo.esview = gt->chn[gt->masterLoopChannel].songptr - 1;
		if (editorInfo.esview < 0)
			editorInfo.esview = 0;
		editorInfo.eseditpos = gt->chn[gt->masterLoopChannel].songptr;
	}
	else if (gt->chn[gt->masterLoopChannel].songptr > editorInfo.esview + VISIBLEORDERLIST)
	{
		editorInfo.esview = gt->chn[gt->masterLoopChannel].songptr - VISIBLEORDERLIST - 1;
		editorInfo.eseditpos = gt->chn[gt->masterLoopChannel].songptr - 1;
	}
	updateviewtopos(gt);
}


