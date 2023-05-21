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
//void paletteEditCommands();
//void paletteLeft();
//void paletteRight();



void orderlistcommands(GTOBJECT *gt)
{
	int c, scrrep;
	int ret = 0;

	//	if (editPaletteMode)
	//	{
	//		paletteEditCommands();
	//		return;
	//	}

	int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12


	int playingSong = getActualSongNumber(editorInfo.esnum, c2);	// JP added this. Only highlight playing row if showing the right song
	int maxCh = 6;
	if ((editorInfo.maxSIDChannels == 3) || (editorInfo.maxSIDChannels == 9 && (editorInfo.esnum & 1)))
		maxCh = 3;

	if (hexnybble >= 0)
	{
		if (editorInfo.expandOrderListView == 0)
			orderListHandleHexInputOriginalView(gt);
		else
			orderListHandleHexInputExpandedView(gt);
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
		if (editorInfo.expandOrderListView == 0)
		{
			if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
			{
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = REPEAT + 0x01;
				editorInfo.escolumn = 1;
			}
		}
		break;

	case '+':

		if (editorInfo.expandOrderListView == 0)
		{
			if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
			{
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = TRANSUP;
				editorInfo.escolumn = 1;
			}
		}
		else
		{
			if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xff)
			{
				if (editorInfo.escolumn == 3)
				{
					songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x7f;
					if ((songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] & 0x7f) == 0xf)
						songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos]--;

					updateTransposeToPlayingSong(gt);
				}
			}
		}
		break;

	case '-':
		if (editorInfo.expandOrderListView == 0)
		{
			if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
			{
				songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = TRANSDOWN + 0x0F;
				editorInfo.escolumn = 1;
			}
		}
		else
		{

			if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xff)
			{
				if (editorInfo.escolumn == 3)
				{
					songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= 0x80;
					updateTransposeToPlayingSong(gt);
				}
			}
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
		if (shiftOrCtrlPressed)
		{
			int schn = editorInfo.eschn;
			int tchn = 0;

			editorInfo.esmarkchn = -1;
			editorInfo.esmarkchnend = -1;
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


				// Do the same for expanded view..
				lentemp = songOrderLength[editorInfo.esnum][schn];
				songOrderLength[editorInfo.esnum][schn] = songOrderLength[editorInfo.esnum][tchn];
				songOrderLength[editorInfo.esnum][tchn] = lentemp;

				for (c = 0;c < MAX_SONGLEN_EXPANDED;c++)
				{
					unsigned char temp = songOrderPatterns[editorInfo.esnum][schn][c];
					songOrderPatterns[editorInfo.esnum][schn][c] = songOrderPatterns[editorInfo.esnum][tchn][c];
					songOrderPatterns[editorInfo.esnum][tchn][c] = temp;

					short stemp = songOrderTranspose[editorInfo.esnum][schn][c];
					songOrderTranspose[editorInfo.esnum][schn][c] = songOrderTranspose[editorInfo.esnum][tchn][c];
					songOrderTranspose[editorInfo.esnum][tchn][c] = stemp;
				}

			}
		}
		break;

	case KEY_X:
		if (shiftOrCtrlPressed)
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
				editorInfo.esmarkchnend = -1;
			}
		}
		break;

	case KEY_C:
		if (shiftOrCtrlPressed)
		{
			if (editorInfo.expandOrderListView == 0)
				orderListCopyMarkedArea();
			else
				orderListCopyMarkedArea_Expanded();
		}
		break;

	case KEY_V:
		if (shiftOrCtrlPressed)
		{
			if (editorInfo.expandOrderListView == 0)
				orderListPasteToCursor(gt);
			else
			{
				int transposeOnly = 0;
				if (editorInfo.escolumn > 2)
					transposeOnly++;
				orderListPasteToCursor_External(gt, 0, transposeOnly);
			}
		}
		break;

	case KEY_I:
		if (shiftOrCtrlPressed)
		{
			if (editorInfo.expandOrderListView == 1)
				orderListPasteToCursor_External(gt, 1, 0);
		}
		break;

	case KEY_L:
		if (shiftOrCtrlPressed)
		{
			if (editorInfo.esmarkchn == -1)
			{
				if (editorInfo.expandOrderListView == 0)
				{
					editorInfo.esmarkend = songlen[editorInfo.esnum][editorInfo.eschn] - 1;
				}
				else
				{
					editorInfo.esmarkend = songOrderLength[editorInfo.esnum][editorInfo.eschn] - 1;
				}
				editorInfo.esmarkchn = editorInfo.eschn;
				editorInfo.esmarkchnend = editorInfo.esmarkchn;
				editorInfo.esmarkstart = 0;
			}
			else
			{
				editorInfo.esmarkchn = -1;
				editorInfo.esmarkchnend = -1;
			}
		}
		break;


	case KEY_SPACE:
		if (!shiftOrCtrlPressed)
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if (editorInfo.expandOrderListView == 0)
			{
				if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
					gt->editorUndoInfo.editorInfo[c2].espos = editorInfo.eseditpos;
			}
			else
			{
				if (editorInfo.eseditpos < songOrderLength[editorInfo.esnum][editorInfo.eschn])
					gt->editorUndoInfo.editorInfo[c2].espos = editorInfo.eseditpos;
			}
			if (gt->editorUndoInfo.editorInfo[c2].esend < gt->editorUndoInfo.editorInfo[c2].espos)
				gt->editorUndoInfo.editorInfo[c2].esend = 0;
		}
		else
		{
			for (c = 0; c < editorInfo.maxSIDChannels; c++)
			{
				int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
				int songNum = getActualSongNumber(editorInfo.esnum, c2);
				int c3 = c2 % 6;

				if (editorInfo.expandOrderListView == 0)
				{
					if (editorInfo.eseditpos < songlen[songNum][c3])
						gt->editorUndoInfo.editorInfo[c2].espos = editorInfo.eseditpos;
				}
				else
				{
					if (editorInfo.eseditpos < songOrderLength[songNum][c3])
						gt->editorUndoInfo.editorInfo[c2].espos = editorInfo.eseditpos;
				}
				if (gt->editorUndoInfo.editorInfo[c2].esend < gt->editorUndoInfo.editorInfo[c2].espos)
					gt->editorUndoInfo.editorInfo[c2].esend = 0;
			}
		}
		break;

	case KEY_BACKSPACE:
		if (!shiftOrCtrlPressed)
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if ((gt->editorUndoInfo.editorInfo[c2].esend != editorInfo.eseditpos) && (editorInfo.eseditpos > gt->editorUndoInfo.editorInfo[c2].espos))
			{
				if (editorInfo.expandOrderListView == 0)
				{
					if (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn])
						gt->editorUndoInfo.editorInfo[c2].esend = editorInfo.eseditpos;
				}
				else
				{
					if (editorInfo.eseditpos < songOrderLength[editorInfo.esnum][editorInfo.eschn])
						gt->editorUndoInfo.editorInfo[c2].esend = editorInfo.eseditpos;
				}
			}
			else
				gt->editorUndoInfo.editorInfo[c2].esend = 0;
		}
		else
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if ((gt->editorUndoInfo.editorInfo[c2].esend != editorInfo.eseditpos) && (editorInfo.eseditpos > gt->editorUndoInfo.editorInfo[c2].espos))
			{
				for (c = 0; c < editorInfo.maxSIDChannels; c++)
				{
					int c3 = c % 6;
					int playingSong = getActualSongNumber(editorInfo.esnum, c);	// JP added this.
					c2 = getActualChannel(editorInfo.esnum, c);	// 0-12

					if (editorInfo.expandOrderListView == 0)
					{
						if (editorInfo.eseditpos < songlen[playingSong][c3])
							gt->editorUndoInfo.editorInfo[c2].esend = editorInfo.eseditpos;
					}
					else
					{
						if (editorInfo.eseditpos < songOrderLength[playingSong][c3])
							gt->editorUndoInfo.editorInfo[c2].esend = editorInfo.eseditpos;
					}
				}
			}
			else
			{
				for (c = 0; c < editorInfo.maxSIDChannels; c++)
					gt->editorUndoInfo.editorInfo[c].esend = 0;
			}
		}
		break;

	case KEY_ENTER:

		if (editorInfo.expandOrderListView == 0)
			ret = handleEnterInCompressedView(gt);
		else
			ret = handleEnterInExpandedView(gt);

		if (ret)
		{
			editorInfo.epmarkchn = -1;
			editorInfo.epchn = editorInfo.eschn;
			editorInfo.epcolumn = 0;
			editorInfo.eppos = 0;
			editorInfo.epview = -VISIBLEPATTROWS / 2;
			editorInfo.editmode = EDIT_PATTERN;
			if (editorInfo.epchn == editorInfo.epmarkchn)
				editorInfo.epmarkchn = -1;
		}
		break;

	case KEY_DEL:
		if (editorInfo.expandOrderListView == 0)
		{
			editorInfo.esmarkchn = -1;
			editorInfo.esmarkchnend = -1;
			deleteorder(gt);
		}
		else
			orderListDelete_External();
		break;

	case KEY_INS:
		if (editorInfo.expandOrderListView == 0)
		{
			editorInfo.esmarkchn = -1;
			editorInfo.esmarkchnend = -1;
			insertorder(0, gt);
		}
		else
			orderListInsert_External(gt);
		break;

	case KEY_HOME:
		if (editorInfo.expandOrderListView == 0)
		{
			if (songlen[editorInfo.esnum][editorInfo.eschn])
			{
				while ((editorInfo.eseditpos != 0) || (editorInfo.escolumn != 0)) orderleft();
			}
		}
		else
		{
			editorInfo.eseditpos = 0;
		}
		break;

	case KEY_END:
		if (editorInfo.expandOrderListView == 0)
		{
			while (editorInfo.eseditpos != songlen[editorInfo.esnum][editorInfo.eschn] + 1) orderright();
		}
		else
		{
			editorInfo.eseditpos = songOrderLength[editorInfo.esnum][editorInfo.eschn];		// 1.3.4
		}
		break;

	case KEY_PGUP:
		if (editorInfo.expandOrderListView == 0)
		{
			for (scrrep = PGUPDNREPEAT * 2; scrrep; scrrep--)
				orderleft();
		}
		else
		{
			editorInfo.eseditpos -= EXTENDEDVISIBLEORDERLIST;
			if (editorInfo.eseditpos < 0)
				editorInfo.eseditpos = 0;
		}
		break;

	case KEY_PGDN:
		if (editorInfo.expandOrderListView == 0)
		{
			for (scrrep = PGUPDNREPEAT * 2; scrrep; scrrep--)
				orderright();
		}
		else
		{
			editorInfo.eseditpos += EXTENDEDVISIBLEORDERLIST;
			if (editorInfo.eseditpos > 0x7ff)
				editorInfo.eseditpos = 0x7ff;
		}
		break;

	case KEY_LEFT:
		if (editorInfo.expandOrderListView == 0)
		{
			if (!ctrlpressed)
				orderleft();
		}
		else
		{
			if (editorInfo.escolumn > 0)
			{
				editorInfo.escolumn--;
				if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xff)
				{
					if (editorInfo.escolumn == 2)
						editorInfo.escolumn--;
				}
			}
			else
			{
				editorInfo.escolumn = 4;

				editorInfo.eschn--;
				if (editorInfo.eschn < 0)
					editorInfo.eschn = maxCh - 1;

				setMasterLoopChannel(gt, "debug_1");

			}
			if (shiftOrCtrlPressed)
			{
				if (editorInfo.esmarkchn == -1)
				{
					editorInfo.esmarkchn = editorInfo.esmarkchnend = editorInfo.eschn;
					editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
				}
				else
					editorInfo.esmarkchnend = editorInfo.eschn;
			}
		}

		break;

	case KEY_RIGHT:
		if (editorInfo.expandOrderListView == 0)
		{
			if (!ctrlpressed)
				orderright();
		}
		else
		{
			editorInfo.escolumn++;
			if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xff)
			{
				if (editorInfo.escolumn == 2)
					editorInfo.escolumn++;
			}
			editorInfo.escolumn %= 5;
			if (!editorInfo.escolumn)
			{
				editorInfo.eschn++;
				if (editorInfo.eschn >= maxCh)
					editorInfo.eschn = 0;

				setMasterLoopChannel(gt, "debug_2");
			}

			if (shiftOrCtrlPressed)
			{
				if (editorInfo.esmarkchn == -1)
				{
					editorInfo.esmarkchn = editorInfo.esmarkchnend = editorInfo.eschn;
					editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
				}
				else
					editorInfo.esmarkchnend = editorInfo.eschn;
			}


		}
		break;

	case KEY_UP:
		if (editorInfo.expandOrderListView == 0)
		{
			editorInfo.eschn--;
			if (editorInfo.eschn < 0) editorInfo.eschn = maxCh - 1;
			if ((editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) || (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn] + 1))
			{
				editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
				editorInfo.escolumn = 0;
			}
			setMasterLoopChannel(gt, "debug_3");
			if (shiftOrCtrlPressed)
			{
				editorInfo.esmarkchn = -1;
				editorInfo.esmarkchnend = -1;
			}
		}
		else
		{
			if (shiftOrCtrlPressed)
			{
				if (editorInfo.esmarkchn == -1)
				{
					editorInfo.esmarkchn = editorInfo.esmarkchnend = editorInfo.eschn;
					editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
				}
			}

			if (editorInfo.eseditpos > 0)
			{

				editorInfo.eseditpos--;
				if (shiftOrCtrlPressed)
					editorInfo.esmarkend = editorInfo.eseditpos;
			}
		}
		break;

	case KEY_DOWN:

		if (editorInfo.expandOrderListView == 0)
		{
			editorInfo.eschn++;
			if (editorInfo.eschn >= maxCh) editorInfo.eschn = 0;
			if ((editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn]) || (editorInfo.eseditpos > songlen[editorInfo.esnum][editorInfo.eschn] + 1))
			{
				editorInfo.eseditpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
				editorInfo.escolumn = 0;
			}
			setMasterLoopChannel(gt, "debug_4");

			if (shiftOrCtrlPressed)
			{
				editorInfo.esmarkchn = -1;
				editorInfo.esmarkchnend = -1;
			}
		}
		else
		{
			if (shiftOrCtrlPressed)
			{
				if (editorInfo.esmarkchn == -1)
				{
					editorInfo.esmarkchn = editorInfo.esmarkchnend = editorInfo.eschn;
					editorInfo.esmarkstart = editorInfo.esmarkend = editorInfo.eseditpos;
				}
			}

			if (editorInfo.eseditpos < 0x7ff)
			{
				editorInfo.eseditpos++;
				if (shiftOrCtrlPressed)
					editorInfo.esmarkend = editorInfo.eseditpos;
			}
		}
		break;
	}

	if (editorInfo.eseditpos - editorInfo.esview < 0)
	{
		editorInfo.esview = editorInfo.eseditpos;
	}
	if (editorInfo.expandOrderListView == 0)
	{
		if (editorInfo.eseditpos - editorInfo.esview >= VISIBLEORDERLIST)
		{
			editorInfo.esview = editorInfo.eseditpos - VISIBLEORDERLIST + 1;
		}
	}
	else
	{
		if (editorInfo.eseditpos - editorInfo.esview >= EXTENDEDVISIBLEORDERLIST)
		{
			editorInfo.esview = editorInfo.eseditpos - EXTENDEDVISIBLEORDERLIST + 1;
		}
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

// Insert single byte into orderlist
void insertorder(unsigned char byte, GTOBJECT *gt)
{
	if ((songlen[editorInfo.esnum][editorInfo.eschn] - editorInfo.eseditpos) - 1 >= 0)
	{
		int len;
		if (songlen[editorInfo.esnum][editorInfo.eschn] < MAX_SONGLEN)
		{
			len = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			songorder[editorInfo.esnum][editorInfo.eschn][len + 1] = songorder[editorInfo.esnum][editorInfo.eschn][len];
			songorder[editorInfo.esnum][editorInfo.eschn][len] = LOOPSONG;
			if (len)
				songorder[editorInfo.esnum][editorInfo.eschn][len - 1] = byte;
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
	if ((shiftOrCtrlPressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
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
			if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn])
				editorInfo.eseditpos--;
			editorInfo.escolumn = 1;
			if (editorInfo.eseditpos < 0)
			{
				editorInfo.eseditpos = 1;
				editorInfo.escolumn = 0;
			}
		}
		else editorInfo.escolumn = 0;
	}
	if ((shiftOrCtrlPressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		editorInfo.esmarkend = editorInfo.eseditpos;
		if (editorInfo.esmarkend == editorInfo.esmarkstart)
		{
			editorInfo.esmarkchn = -1;
			editorInfo.esmarkchnend = -1;
		}
	}
}

void orderright(void)
{
	if ((shiftOrCtrlPressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
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
	if ((shiftOrCtrlPressed) && (editorInfo.eseditpos < songlen[editorInfo.esnum][editorInfo.eschn]))
	{
		editorInfo.esmarkend = editorInfo.eseditpos;
		if (editorInfo.esmarkend == editorInfo.esmarkstart)
		{
			editorInfo.esmarkchn = -1;
			editorInfo.esmarkchnend = -1;
		}
	}
}

void nextsong(GTOBJECT *gt)
{
	if (editorInfo.expandOrderListView)
		compressSong(editorInfo.esnum);
	editorInfo.esnum++;
	if (editorInfo.esnum >= MAX_SONGS) editorInfo.esnum = MAX_SONGS - 1;
	songchange(gt, 1);

	//	if (gt->masterLoopSubSong==-1)
	//		setMasterLoopChannel(gt,"nextsong");
}

void prevsong(GTOBJECT *gt)
{
	if (editorInfo.expandOrderListView)
		compressSong(editorInfo.esnum);

	editorInfo.esnum--;
	if (editorInfo.esnum < 0) editorInfo.esnum = 0;
	songchange(gt, 1);
	//	if (gt->masterLoopSubSong == -1)
	//		setMasterLoopChannel(gt, "prevsong");
}

int backupPatPos[MAX_PLAY_CH];
int jr = 0;

int lastSong = -1;
void songchange(GTOBJECT *gt, int resetEditingPositions)
{
	int c;
	int s = editorInfo.esnum / 2;	// JP 9 or 12 channel song only

	int jc2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12 for currently selected channel in orderlist
	int jsongNum = getActualSongNumber(editorInfo.esnum, jc2);


	editorInfo.highlightLoopChannel = 999;			// remove from display
//	gt->interPatternLoopEnabledFlag = 0;		// disable in player
	editorInfo.highlightLoopPatternNumber = -1;
	editorInfo.highlightLoopStart = editorInfo.highlightLoopEnd = 0;

	if (editorInfo.maxSIDChannels <= 6)
		lastSong = s + 1;
	if (s != lastSong)
	{
		lastSong = s;

		if (resetEditingPositions)
		{
			resetSongInfo(gt, jc2);
		}

		if (gt->songinit != PLAY_STOPPED)
		{
			stopsong(gt);
		}

		lastInfoPatternCh = -1;	// force text
		displayPatternInfo(gt);
	}

	if ((editorInfo.maxSIDChannels == 3) || (editorInfo.maxSIDChannels == 9 && (editorInfo.esnum & 1)))
	{
		if (editorInfo.eschn >= 3)
			editorInfo.eschn = 2;
	}


	for (c = 0; c < editorInfo.maxSIDChannels; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int songNum = getActualSongNumber(editorInfo.esnum, c2);

		if (gt->editorUndoInfo.editorInfo[c2].espos >= songlen[songNum][c2 % 6] + 1)
		{
			gt->editorUndoInfo.editorInfo[c2].espos = songlen[songNum][c2 % 6] - 1;	//0;	//
			if (gt->editorUndoInfo.editorInfo[c2].espos < 0)
				gt->editorUndoInfo.editorInfo[c2].espos = 0;
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
	editorInfo.epview = -VISIBLEPATTROWS / 2;

	if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn])
		editorInfo.eseditpos++;
	//editorInfo.epmarkchn = -1;	// JP - Removed on 27thAug2022 - Show any existing marked channel area
	editorInfo.esmarkchn = -1;
	editorInfo.esmarkchnend = -1;

	updateviewtopos(gt);
}


/*
A cut down version of songchange
initialises editor parameters (espos and epnum..)
*/
void initEditorSongInfo(GTOBJECT *gt)
{
	editorInfo.highlightLoopChannel = 999;			// remove from display
	editorInfo.highlightLoopPatternNumber = -1;
	editorInfo.highlightLoopStart = editorInfo.highlightLoopEnd = 0;

	// Set up the editor pattern length info (gt->editorUndoInfo.editorInfo[c2].epnum ) and song pos (.espos) for each channel	
	orderSelectPatternsFromSelected(gt);

	editorInfo.eseditpos = 0;
	editorInfo.esmarkchn = -1;
	editorInfo.esmarkchnend = -1;
	editorInfo.eppos = 0;	// pattern pos
	editorInfo.epview = -VISIBLEPATTROWS / 2;

	if (editorInfo.eseditpos == songlen[editorInfo.esnum][editorInfo.eschn])
		editorInfo.eseditpos++;
	editorInfo.esmarkchn = -1;

	updateviewtopos(gt);
}

void resetSongInfo(GTOBJECT *gt, int jc2)
{
	for (int c = 0; c < editorInfo.maxSIDChannels; c++)
	{
		gt->editorUndoInfo.editorInfo[c].espos = 0;		// highlighted (green) position
		gt->editorUndoInfo.editorInfo[c].esend = 0;		// end position (length of song)
		gt->editorUndoInfo.editorInfo[c].epnum = c;
	}

	gt->masterLoopSubSong = editorInfo.esnum;
	gt->masterLoopChannel = jc2;

	editorInfo.eseditpos = 0;		// Reset cursor position in order list
	editorInfo.esview = 0;			// reset scroll position in order list
}


void updateviewtopos(GTOBJECT *gt)
{
	int c, d;

	//	if (editPaletteMode)
	//		return;

	for (c = 0; c < editorInfo.maxSIDChannels; c++)
	{
		int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
		int songNum = getActualSongNumber(editorInfo.esnum, c2);
		int c3 = c % 6;

		if (editorInfo.expandOrderListView == 0)
		{
			for (d = gt->editorUndoInfo.editorInfo[c2].espos; d < songlen[songNum][c3]; d++)
			{
				if (songorder[songNum][c3][d] < MAX_PATT)
				{
					gt->editorUndoInfo.editorInfo[c2].epnum = songorder[songNum][c3][d];
					break;
				}
				else
				{
					if (gt->editorUndoInfo.editorInfo[c2].epnum >= MAX_PATT)	// JP added this. Handles 12 channel songs where there's no data in odd song number
						gt->editorUndoInfo.editorInfo[c2].epnum = 0;
				}
			}
			if (songlen[songNum][c3] == 0)
			{
				gt->editorUndoInfo.editorInfo[c2].epnum = 0;
			}
		}
		else
		{
			for (d = gt->editorUndoInfo.editorInfo[c2].espos; d < songOrderLength[songNum][c3]; d++)
			{
				if (songOrderPatterns[songNum][c3][d] < MAX_PATT)
				{
					gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[songNum][c3][d];
					break;
				}
				else
				{
					if (gt->editorUndoInfo.editorInfo[c2].epnum >= MAX_PATT)	// JP added this. Handles 12 channel songs where there's no data in odd song number
						gt->editorUndoInfo.editorInfo[c2].epnum = 0;
				}
			}
			if (songOrderLength[songNum][c3] == 0)
			{
				gt->editorUndoInfo.editorInfo[c2].epnum = 0;
			}
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
		editorInfo.highlightLoopPatternNumber == gtPlayer->editorUndoInfo.editorInfo[c3].epnum)
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
		editorInfo.highlightLoopPatternNumber = gtPlayer->editorUndoInfo.editorInfo[c3].epnum;	// highlight looping area	
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

	if (c2 >= editorInfo.maxSIDChannels)
		return -1;

	gtloop->loopEnabledFlag = 0;
	initsong(sng, PLAY_BEGINNING, gtloop);

	gtloop->disableLoopSearch = 1;

	do {
		playroutine(gtloop);

		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

	} while (gtloop->chn[c3].songptr <= startSongPos);

	int tempMin = gtloop->timemin;
	int tempSec = gtloop->timesec;
	int tempFrame = gtloop->timeframe;

	// Now get Select Start, Select End and Play Start ..Then get pattern end

	int findPatternLoopStart = 0;
	int findPatternLoopEnd = 0;
	int loopPatternNum = 0;

	// Now sync to end of pattern (info used for looping)
	int sptr = gtloop->chn[c3].songptr;

	do {
		playroutine(gtloop);
		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

		int lastpattptr = gtloop->chn[c3].pattptr;
		int found = 0;
		if (findPatternLoopStart == 0 && gtloop->chn[c3].pattptr == markStart * 4)
			found = 1;
		else if (gtloop->chn[c3].songptr != startSongPos + 1 || gtloop->chn[c3].pattptr < lastpattptr)
			found = 1;
		if (found)
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
		lastpattptr = gtloop->chn[c3].pattptr;

	} while (gtloop->chn[c3].songptr == sptr);

	return 0;
}


int calculateLoopInfo2(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop)
{
	GTOBJECT *gtPlayer = &gtObject;

	int c3 = getActualChannel(songNum, channelNum);
	int sng = songNum;

	if (c3 >= editorInfo.maxSIDChannels)
		return -1;

	gtloop->loopEnabledFlag = 0;
	initsong(sng, PLAY_BEGINNING, gtloop);
	gtloop->disableLoopSearch = 1;
	gtloop->noSIDWrites = 1;

	do {

		playroutine(gtloop);

		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

	} while (gtloop->chn[c3].songptr <= startSongPos);

	memcpy((char *)&gtPlayer->loopStartChn[0], (char*)&gtloop->chn[0], sizeof(CHN)*MAX_PLAY_CH);

	// Do this manually, in case compiler changes order of data
	//	memcpy((char *)&gtPlayer->looptimemin, (char*)&gtloop->timemin, sizeof(int) * 3);
	gtPlayer->looptimemin = gtloop->looptimemin;
	gtPlayer->looptimesec = gtloop->looptimesec;
	gtPlayer->looptimeframe = gtloop->looptimeframe;
	
	int tempMin = gtloop->timemin;
	int tempSec = gtloop->timesec;
	int tempFrame = gtloop->timeframe;

	// Now get Select Start, Select End and Play Start ..Then get pattern end

	int findPatternLoopStart = 0;
	int findPatternLoopEnd = 0;
	int loopPatternNum = 0;

	// Now sync to end of pattern (info used for looping)
	int sptr = gtloop->chn[c3].songptr;

	int quitloop = 0;
	do {
		playroutine(gtloop);
		if (gtloop->songinit == PLAY_STOPPED)	// Error in song data
			return -1;

		if (gtloop->chn[c3].loopCount)		// reached end of song and looped?
			quitloop++;
		else if (gtloop->chn[c3].songptr != sptr)
			quitloop++;

	} while (!quitloop);	//gtloop->chn[c2].songptr == sptr);

	memcpy((char*)&gtPlayer->loopEndChn[0], (char*)&gtloop->chn[0], sizeof(CHN)*MAX_PLAY_CH);

	return 0;
}


/*
void playFromCurrentPosition(GTOBJECT *gt, int currentPos)
{
	if (editPaletteMode)
		return;

	int t1 = followplay;
	int t2 = gt->interPatternLoopEnabledFlag;
	gt->loopEnabledFlag = 0;
	gt->interPatternLoopEnabledFlag = 0;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	handleShiftSpace(gt, c2, currentPos * 4, 0, 1);

	gt->loopEnabledFlag = transportLoopPattern;
	gt->interPatternLoopEnabledFlag = t2;
	followplay = t1;
}
*/

void orderPlayFromPosition(GTOBJECT *gt, int startPatternPos, int startSongPos, int focusChannel, int enableSIDWrites)
{

	//	sprintf(textbuffer, "spp %d ssp %d, fc %d mlc %d", startPatternPos, startSongPos, focusChannel, gt->masterLoopChannel);
	//	printtext(PANEL_NAMES_X, PANEL_NAMES_Y + 3, 0xe, textbuffer);

		//		int t1 = followplay;
	int t2 = gt->interPatternLoopEnabledFlag;

	if (editorInfo.expandOrderListView == 0)
	{
		if (startSongPos >= songlen[editorInfo.esnum][focusChannel % 6])
			return;
	}
	else
	{
		if (startSongPos >= songOrderLength[editorInfo.esnum][focusChannel % 6] - 1)	// 1.3.3
			return;
	}


	int c2 = getActualChannel(editorInfo.esnum, focusChannel);
	int sng = getActualSongNumber(editorInfo.esnum, c2);

	if (c2 >= editorInfo.maxSIDChannels)
		return;

	//printf("play1\n");
	if (gt->songinit != PLAY_STOPPED)
	{
		stopsong(gt);
	}

	//printf("play2\n");
	bypassPlayRoutine = 1;	// Stop interrupt from updating play routine. We're going to do it manually
	SDL_Delay(50);

	int loopMode = transportLoopPattern;	// gt->loopEnabledFlag;

	int ep = startSongPos;
	int ep2;

	if (editorInfo.expandOrderListView == 0)
	{
		/*
					do
					{
						ep2 = ep;
						if ((songorder[sng][c2 % 6][ep] >= REPEAT) && (songorder[sng][c2 % 6][ep] < TRANSDOWN))
							ep++;
						if ((songorder[sng][c2 % 6][ep] >= TRANSDOWN) && (songorder[sng][c2 % 6][ep] < LOOPSONG))
							ep++;
					} while (ep != ep2);
		*/
	}

	initsong(editorInfo.esnum, PLAY_BEGINNING, gt);
	gt->loopEnabledFlag = 0;
	gt->disableLoopSearch = 1;

	do {
		playroutine(gt);
		if (gt->songinit == PLAY_STOPPED)	// Error in song data
			return;

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
	gt->interPatternLoopEnabledFlag = t2;

}

int jcnt = 0;

/*
Set up the editor pattern length info (gt->editorUndoInfo.editorInfo[c2].epnum ) and song pos (.espos) for each channel
*/
void orderSelectPatternsFromSelected(GTOBJECT *gt)
{

	if (editorInfo.expandOrderListView == 0)
	{
		if (editorInfo.eseditpos >= songlen[editorInfo.esnum][editorInfo.eschn])
			return;
	}
	else
	{
		if (editorInfo.eseditpos >= songOrderLength[editorInfo.esnum][editorInfo.eschn] - 1)	// 1.3.3
			return;
	}
	// V1.2.2. fix - rather than using eschn or epchn, use masterLoopChannel instead. works if you're editing pattern or song.
//	int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);
	int c2 = gt->masterLoopChannel;
	int sng = getActualSongNumber(editorInfo.esnum, c2);

	if (c2 >= editorInfo.maxSIDChannels)
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
			{
				return;
			}

		} while (gte->chn[c2].songptr - 1 < ep);

		for (int c = 0; c < editorInfo.maxSIDChannels; c++)
		{
			c2 = c;
			int sng = getActualSongNumber(editorInfo.esnum, c2);

			ep = gte->chn[c2].songptr - 1;	// -1;

			if (editorInfo.expandOrderListView == 0)
			{
				do
				{
					ep2 = ep;
					if ((songorder[sng][c2 % 6][ep] >= REPEAT) && (songorder[sng][c2 % 6][ep] < TRANSDOWN))
						ep++;
					if ((songorder[sng][c2 % 6][ep] >= TRANSDOWN) && (songorder[sng][c2 % 6][ep] < LOOPSONG))
						ep++;
				} while (ep != ep2);
				gt->editorUndoInfo.editorInfo[c2].epnum = songorder[sng][c2 % 6][ep];
			}
			else
				gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[sng][c2 % 6][ep];
			gt->editorUndoInfo.editorInfo[c2].espos = ep;
		}
	}
	else
	{
		for (int c = 0; c < editorInfo.maxSIDChannels; c++)
		{
			c2 = c;
			int sng = getActualSongNumber(editorInfo.esnum, c2);
			if (editorInfo.expandOrderListView == 0)
				gt->editorUndoInfo.editorInfo[c2].epnum = songorder[sng][c2 % 6][0];
			else
				gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[sng][c2 % 6][0];
			gt->editorUndoInfo.editorInfo[c2].espos = 0;
		}
	}

	editorInfo.epview = -VISIBLEPATTROWS / 2;
	editorInfo.eppos = 0;

	int c3 = getActualChannel(gt->psnum, editorInfo.epmarkchn);

	int plen = (pattlen[gt->editorUndoInfo.editorInfo[c3].epnum] - 1);	// *4;

//		sprintf(textbuffer, "%x markchan ch %x len %x end %x  ", jdebug[15]++, editorInfo.epmarkchn,plen, editorInfo.epmarkend);
	//	printtext(70, 12, 0xe, textbuffer);

	if (editorInfo.epmarkchn >= 0)
	{

		if (editorInfo.epmarkend > plen || editorInfo.epmarkstart > plen)
		{
			editorInfo.epmarkstart = editorInfo.epmarkend = 0;
			editorInfo.epmarkchn = -1;
		}
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

	if (editorInfo.expandOrderListView == 0)
	{
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
	}
	else
	{
		for (int s = 0;s < MAX_SONGS;s++)
		{

			for (int c = 0;c < MAX_CHN;c++)
			{
				//	printf("song %x orderlength %x\n", s, songOrderLength[s][c]);

				for (int l = 0;l < songOrderLength[s][c] - 1;l++)
				{
					int pat = songOrderPatterns[s][c][l];

					//	printf("song %x orderlength %x chan %x index %x pat %x\n", s, songOrderLength[s][c],c,l,pat);

					if (!patternChecked[pat])
					{

						countInstrumentsInPattern(pat);
						patternChecked[pat]++;
					}
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

	if (pat >= 208)
	{
		printf("ERROR!  pattern %x \n", pat);
		return;
	}
	for (int i = 0;i < MAX_INSTR;i++)
	{
		pattInstrumentCount[pat][i] = 0;
	}

	for (int p = 0;p < pattlen[pat];p++)
	{
		int instr = pattern[pat][(p * 4) + 1];
		if (instr != 0)
		{
			if (instr >= 64 || pat >= 208)
			{
				printf("ERROR! Instrument %x in pattern %x position %x\n", instr, pat, p);
			}
			else
				pattInstrumentCount[pat][instr]++;
		}
	}

}


void setMasterLoopChannel(GTOBJECT *gt, char *debugText)
{

	//	sprintf(textbuffer, "%x master %s", jdebug[15]++, debugText);
	//	printtext(70, 12, 0xe, textbuffer);

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
			gt->masterLoopSubSong = editorInfo.esnum;
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

void orderListHandleHexInputOriginalView(GTOBJECT *gt)
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

		int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

		if (editorInfo.eseditpos == gt->editorUndoInfo.editorInfo[c2].espos)
		{
			if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < MAX_PATT)	// remember pattern number for undo
				gt->editorUndoInfo.editorInfo[c2].epnum = songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
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

void updateTransposeToPlayingSong(GTOBJECT *gt)
{
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);
	if (editorInfo.eseditpos == gt->chn[c2].songptr - 1)	// cursor editing row as whats currently playing in this channel?
	{
		int t = songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];	// Yes. So modify the transpose directly too
		if (t < 0x080)
			gt->chn[c2].trans = t;
		else
			gt->chn[c2].trans = -(t & 0x7f);
	}
}

// TO WORK OUT:
// Loop position can be 12 bit in expanded view. 
// We can't enter a value longer than 2 digits
// Shift-click on entry to set loop position?
void orderListHandleHexInputExpandedView(GTOBJECT *gt)
{
	// songOrderPatterns[editorInfo.esnum][c][p];



	if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xff)	// editing pattern number or transpose value
	{
		if (editorInfo.escolumn == 3)
			return;	// cursor currently on +/- so we don't want to handle hex input there

		if (editorInfo.escolumn == 4)		// transpose value
		{
			if (songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0x80)
			{
				if (hexnybble < 0xf)
				{
					songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = hexnybble;
				}
			}
			else
			{
				songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = 0x80 + hexnybble;
			}

			updateTransposeToPlayingSong(gt);

			songCompressedSize[editorInfo.esnum][editorInfo.eschn] = generateCompressedSongChannel(editorInfo.esnum, editorInfo.eschn, 1);
			return;
		}
	}
	else if (editorInfo.escolumn >= 2)		// editing loop position
	{
		switch (editorInfo.escolumn)
		{
		case 2:
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x0ff;
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble << 8;
			editorInfo.escolumn = 3;
			break;
		case 3:
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x0f0f;
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble << 4;
			editorInfo.escolumn = 4;
			break;
		case 4:
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0xff0;
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble;
			editorInfo.escolumn = 2;
			break;
		}
		songCompressedSize[editorInfo.esnum][editorInfo.eschn] = generateCompressedSongChannel(editorInfo.esnum, editorInfo.eschn, 1);
		return;
	}

	if (editorInfo.escolumn <= 2)	// pattern editing
	{
		int temp = songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
		int tempColumn = editorInfo.escolumn;

		switch (editorInfo.escolumn)
		{
		case 0:

			songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0x0f;
			songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble << 4;
			editorInfo.escolumn = 1;
			break;
		case 1:
			songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] &= 0xf0;
			songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] |= hexnybble;
			editorInfo.escolumn = 0;
			break;

		}

		if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] == 0xff)
		{
			if (temp == 0xff)
				editorInfo.escolumn = tempColumn;
			else
				songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = 0;
		}
		else if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] > 0xcf)
		{
			if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < 0xf0)
			{
				songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = temp;
				editorInfo.escolumn = tempColumn;
			}
			else
			{
				editorInfo.escolumn = tempColumn;
				songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = 0xff;
				songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = 0;
			}
		}
		else if (temp == 0xff)
		{
			songOrderTranspose[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] = 0;
		}
		else
		{
			int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

			if (editorInfo.eseditpos == gt->editorUndoInfo.editorInfo[c2].espos)
			{
				if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < MAX_PATT)	// jpjpjp
					gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
			}
		}
	}

	songCompressedSize[editorInfo.esnum][editorInfo.eschn] = generateCompressedSongChannel(editorInfo.esnum, editorInfo.eschn, 1);

	int index = findFirstEndMarkerIndex(editorInfo.esnum, editorInfo.eschn);
	songOrderLength[editorInfo.esnum][editorInfo.eschn] = index + 1;

	//sprintf(textbuffer, "j %x, chn %x, songorderLen %x\n", editorInfo.esnum, editorInfo.eschn, (songOrderLength[editorInfo.esnum][editorInfo.eschn] - 1));
	//printtext(70, 1, 0xe, textbuffer);

	//	songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos]++;
	return;
}

int findFirstEndMarkerIndex(int sng, int chn)
{
	for (int i = 0;i < MAX_SONGLEN_EXPANDED;i++)
	{
		if (songOrderPatterns[sng][chn][i] == 0xff)
			return i;
	}
	return MAX_SONGLEN_EXPANDED - 1;
}



int handleEnterInCompressedView(GTOBJECT *gt)
{
	if (editorInfo.eseditpos >= songlen[editorInfo.esnum][editorInfo.eschn])
		return 0;

	if (!shiftOrCtrlPressed)
	{
		int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

		if (songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < MAX_PATT)
			gt->editorUndoInfo.editorInfo[c2].epnum = songorder[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
	}
	else
	{
		backupPatternDisplayInfo(gt);	//V1.2.2 - Preserve pattern edit position
		orderSelectPatternsFromSelected(gt);
		restorePatternDisplayInfo(gt);	//V1.2.2
		return 0;
		/*
				int c, d;

				for (c = 0; c < editorInfo.maxSIDChannels; c++)
				{
					int start;

					int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
					int songNum = getActualSongNumber(editorInfo.esnum, c2);
					int c3 = c % 6;

					if (editorInfo.eseditpos != gt->editorUndoInfo.editorInfo[c2].espos)
						start = editorInfo.eseditpos;
					else
						start = gt->editorUndoInfo.editorInfo[c2].espos;

					for (d = start; d < songlen[songNum][c3]; d++)
					{
						if (songorder[songNum][c3][d] < MAX_PATT)
						{
							gt->editorUndoInfo.editorInfo[c2].epnum = songorder[songNum][c3][d];
							break;
						}
					}
				}
		*/
	}
	return 1;
}


int handleEnterInExpandedView(GTOBJECT *gt)
{

	//	sprintf(textbuffer, "snd %x, chn %x, songorderLen %x\n", editorInfo.esnum, editorInfo.eschn,(songOrderLength[editorInfo.esnum][editorInfo.eschn] - 1));
	//	printtext(70,1, 0xe, textbuffer);

	if (editorInfo.eseditpos >= songOrderLength[editorInfo.esnum][editorInfo.eschn] - 1)	// 1.3.3
		return 0;

	if (!shiftOrCtrlPressed)
	{
		int c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);	// 0-12

		if (songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos] < MAX_PATT)
			gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[editorInfo.esnum][editorInfo.eschn][editorInfo.eseditpos];
	}
	else
	{
		backupPatternDisplayInfo(gt);	//V1.2.2 - Preserve pattern edit position
		orderSelectPatternsFromSelected(gt);
		restorePatternDisplayInfo(gt);	//V1.2.2
		return 0;
		/*
				int c, d;

				for (c = 0; c < editorInfo.maxSIDChannels; c++)
				{
					int start;

					int c2 = getActualChannel(editorInfo.esnum, c);	// 0-12
					int songNum = getActualSongNumber(editorInfo.esnum, c2);
					int c3 = c % 6;

					if (editorInfo.eseditpos != gt->editorUndoInfo.editorInfo[c2].espos)
						start = editorInfo.eseditpos;
					else
						start = gt->editorUndoInfo.editorInfo[c2].espos;

					for (d = start; d < songOrderLength[songNum][c3]; d++)
					{
						if (songOrderPatterns[songNum][c3][d] < MAX_PATT)
						{
							gt->editorUndoInfo.editorInfo[c2].epnum = songOrderPatterns[songNum][c3][d];
							break;
						}
					}
				}
		*/
	}
	return 1;
}


void orderListCopyMarkedArea()
{
	int c;
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
		editorInfo.esmarkchnend = -1;
	}
}

void orderListCopyMarkedArea_Expanded()
{
	int c;
	if (editorInfo.esmarkchn == -1)	// no table selected. copy single row under cursor
	{
		editorInfo.esmarkchn = editorInfo.eschn;
		editorInfo.esmarkchnend = editorInfo.esmarkchn;
		editorInfo.esmarkstart = editorInfo.eseditpos;
		editorInfo.esmarkend = editorInfo.esmarkstart;
	}

	if (editorInfo.esmarkchn != -1)
	{
		int x, y, w, h;
		getExpandedSelectedArea(&x, &y, &w, &h);

		int wy = 0;
		for (int i = y;i < (y + h);i++)
		{
			int wx = 0;
			for (int j = x;j < (x + w);j++)
			{
				songOrderPatternsCopyPaste[wx][wy] = songOrderPatterns[editorInfo.esnum][j][i];
				songOrderTransposeCopyPaste[wx++][wy] = songOrderTranspose[editorInfo.esnum][j][i];
			}
			wy++;
		}
		copyPasteW = w;
		copyPasteH = h;
		copyExpandedSongValidFlag = 1;

		editorInfo.esmarkchn = -1;
		editorInfo.esmarkchnend = -1;
	}
}

void getExpandedSelectedArea(int *x, int *y, int *w, int *h)
{
	int tx, ty, tw, th;

	if (editorInfo.esmarkchn < 0 || editorInfo.esmarkchnend < 0 || editorInfo.esmarkstart < 0 || editorInfo.esmarkend < 0)
	{
		*x = 0;
		*y = 0;
		*w = 0;
		*h = 0;
		return;
	}

	tx = editorInfo.esmarkchn;
	tw = editorInfo.esmarkchnend - editorInfo.esmarkchn;
	if (tw < 0)
	{
		tx = editorInfo.esmarkchnend;
		tw = editorInfo.esmarkchn - editorInfo.esmarkchnend;
	}
	tw++;

	ty = editorInfo.esmarkstart;
	th = editorInfo.esmarkend - editorInfo.esmarkstart;
	if (th < 0)
	{
		ty = editorInfo.esmarkend;
		th = editorInfo.esmarkstart - editorInfo.esmarkend;
	}
	th++;

	*x = tx;
	*y = ty;
	*w = tw;
	*h = th;
}


void orderListPasteToCursor(GTOBJECT *gt)
{
	int c;
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
		songorder[editorInfo.esnum][editorInfo.eschn][songlen[editorInfo.esnum][editorInfo.eschn] + 1] = trackcopyrpos; // copying whole channel song list? then copy over loop position too
}


void orderListPasteToCursor_External(GTOBJECT *gt, int insert, int transposeOnly)
{
	if (copyExpandedSongValidFlag == 0)
		return;

	int xd = editorInfo.eschn;
	for (int x = 0;x < copyPasteW;x++)
	{
		if (insert)
		{
			if (!transposeOnly)
			{
				for (int y = 0;y < copyPasteH;y++)
				{
					orderListInsertRowAtCursor_External(gt, editorInfo.esnum, xd, editorInfo.eseditpos);
				}
			}
		}

		int yd = editorInfo.eseditpos;
		for (int y = 0;y < copyPasteH;y++)
		{
			if (!transposeOnly)
				songOrderPatterns[editorInfo.esnum][xd][yd] = songOrderPatternsCopyPaste[x][y];
			songOrderTranspose[editorInfo.esnum][xd][yd] = songOrderTransposeCopyPaste[x][y];
			yd++;

			if (transposeOnly)
			{
				if (yd >= songOrderLength[editorInfo.esnum][xd] - 1)
					break;
			}
			else
			{
				if (yd == MAX_SONGLEN_EXPANDED - 1)
					break;
			}
		}
		int index = findFirstEndMarkerIndex(editorInfo.esnum, xd);
		songOrderLength[editorInfo.esnum][xd] = index + 1;	// 1.3.8

		songCompressedSize[editorInfo.esnum][xd] = generateCompressedSongChannel(editorInfo.esnum, xd, 1);

		xd++;
		if (xd == MAX_CHN)
			break;
	}
}

void orderListInsertRowAtCursor_External(GTOBJECT *gt, int sng, int chn, int row)
{
	for (int y = MAX_SONGLEN_EXPANDED - 2;y >= row;y--)	// 1.3.4
	{
		songOrderPatterns[sng][chn][y] = songOrderPatterns[sng][chn][y - 1];
		songOrderTranspose[sng][chn][y] = songOrderTranspose[sng][chn][y - 1];
	}
	songOrderPatterns[sng][chn][row] = 0;
	songOrderTranspose[sng][chn][row] = 0;

	//	int index = findFirstEndMarkerIndex(sng, chn);
	songOrderLength[sng][chn]++;

	//	sprintf(textbuffer, "sng %x, chn %x songorderLen %x\n", sng,chn, (songOrderLength[sng][chn] - 1));
	//	printtext(70, 1, 0xe, textbuffer);

	int c2 = getActualChannel(sng, chn);	// 0-11

	if (gt->editorUndoInfo.editorInfo[c2].espos >= row)	// 1.3.8 Was chn
		gt->editorUndoInfo.editorInfo[c2].espos++;

	songCompressedSize[sng][chn] = generateCompressedSongChannel(sng, chn, 1);
}

void orderListDeleteRowAtCursor_External(int sng, int chn, int row)
{
	for (int y = row;y < MAX_SONGLEN_EXPANDED - 2;y++)	// 1.3.4
	{
		songOrderPatterns[sng][chn][y] = songOrderPatterns[editorInfo.esnum][chn][y + 1];
		songOrderTranspose[sng][chn][y] = songOrderTranspose[editorInfo.esnum][chn][y + 1];
	}
	songOrderPatterns[sng][chn][MAX_SONGLEN_EXPANDED - 2] = 0;		// 1.3.4
	songOrderTranspose[sng][chn][MAX_SONGLEN_EXPANDED - 2] = 0;
	songOrderLength[sng][chn]--;

	songCompressedSize[sng][chn] = generateCompressedSongChannel(sng, chn, 1);
}


void orderListInsert_External(GTOBJECT *gt)
{
	int x, y, w, h;
	getExpandedSelectedArea(&x, &y, &w, &h);
	if (w == 0)	// Nothing selected
	{
		w = 1;
		h = 1;
		x = editorInfo.eschn;
		y = editorInfo.eseditpos;
	}

	for (int j = 0;j < h;j++)
	{
		for (int i = x;i < (x + w);i++)
		{
			orderListInsertRowAtCursor_External(gt, editorInfo.esnum, i, y);
		}
	}

	//	for (int i = x;i < (x + w);i++)
	//	{
		//	int index = findFirstEndMarkerIndex(editorInfo.esnum, i);
		//	songOrderLength[editorInfo.esnum][i] = index;
		//}
}

void orderListDelete_External()
{
	int x, y, w, h;
	getExpandedSelectedArea(&x, &y, &w, &h);
	if (w == 0)	// Nothing selected
	{
		w = 1;
		h = 1;
		x = editorInfo.eschn;
		y = editorInfo.eseditpos;
	}

	for (int j = 0;j < h;j++)
	{
		for (int i = x;i < (x + w);i++)
		{
			orderListDeleteRowAtCursor_External(editorInfo.esnum, i, y);
		}
	}

	for (int i = x;i < (x + w);i++)
	{
		int index = findFirstEndMarkerIndex(editorInfo.esnum, i);
		songOrderLength[editorInfo.esnum][i] = index + 1;
	}

	editorInfo.esmarkchn = -1;
	editorInfo.esmarkchnend = -1;
}