//
// GTUltra v2 instrument editor
//

#define GINSTR_C

#include "goattrk2.h"

INSTR instrcopybuffer;
int cutinstr = -1;


void instrumentcommands(GTOBJECT *gt)
{
	switch (rawkey)
	{
	case KEY_UP:
	case KEY_DOWN:
	case KEY_LEFT:
	case KEY_RIGHT:
		win_enableKeyRepeat();
		break;
	default:
		if (enablekeyrepeat)
			win_disableKeyRepeat();
	}

	switch (rawkey)
	{
	case 0x8:
	case KEY_DEL:
		if ((editorInfo.einum) && (shiftpressed) && (editorInfo.eipos < 9))
		{
			deleteinstrtable(editorInfo.einum);
			clearinstr(editorInfo.einum);
		}
		break;

	case KEY_X:
		if ((editorInfo.einum) && (shiftpressed) && (editorInfo.eipos < 9))
		{
			cutinstr = editorInfo.einum;
			memcpy(&instrcopybuffer, &instr[editorInfo.einum], sizeof(INSTR));
			clearinstr(editorInfo.einum);
		}
		break;

	case KEY_C:
		if ((editorInfo.einum) && (shiftpressed) && (editorInfo.eipos < 9))
		{
			cutinstr = -1;
			memcpy(&instrcopybuffer, &instr[editorInfo.einum], sizeof(INSTR));
		}
		break;

	case KEY_S:
		if ((editorInfo.einum) && (shiftpressed) && (editorInfo.eipos < 9))
		{
			memcpy(&instr[editorInfo.einum], &instrcopybuffer, sizeof(INSTR));
			if (cutinstr != -1)
			{
				int c, d;
				for (c = 0; c < MAX_PATT; c++)
				{
					for (d = 0; d < pattlen[c]; d++)
						if (pattern[c][d * 4 + 1] == cutinstr) pattern[c][d * 4 + 1] = editorInfo.einum;
				}
			}
		}
		break;

	case KEY_V:
		if ((editorInfo.einum) && (shiftpressed) && (editorInfo.eipos < 9))
		{
			memcpy(&instr[editorInfo.einum], &instrcopybuffer, sizeof(INSTR));
		}
		break;

	case KEY_RIGHT:
		if (editorInfo.eipos < 9)
		{
			editorInfo.eicolumn++;
			if (editorInfo.eicolumn > 1)
			{
				editorInfo.eicolumn = 0;
				editorInfo.eipos += 5;
				if (editorInfo.eipos >= 9) editorInfo.eipos -= 10;
				if (editorInfo.eipos < 0) editorInfo.eipos = 8;
			}
		}
		break;

	case KEY_LEFT:
		if (editorInfo.eipos < 9)
		{
			editorInfo.eicolumn--;
			if (editorInfo.eicolumn < 0)
			{
				editorInfo.eicolumn = 1;
				editorInfo.eipos -= 5;
				if (editorInfo.eipos < 0) editorInfo.eipos += 10;
				if (editorInfo.eipos >= 9) editorInfo.eipos = 8;
			}
		}
		break;

	case KEY_DOWN:
		if (editorInfo.eipos < 9)
		{
			editorInfo.eipos++;
			if (editorInfo.eipos > 8) editorInfo.eipos = 0;
		}
		break;

	case KEY_UP:
		if (editorInfo.eipos < 9)
		{
			editorInfo.eipos--;
			if (editorInfo.eipos < 0) editorInfo.eipos = 8;
		}
		break;

	case KEY_N:
		if ((editorInfo.eipos != 9) && (shiftpressed))
		{
			editorInfo.eipos = 9;
			return;
		}
		break;

	case KEY_U:
		if (shiftpressed)
		{
			editorInfo.etlock ^= 1;
			validatetableview();
		}
		break;

	case KEY_SPACE:
		if (editorInfo.eipos != 9)
		{
			if (!shiftpressed)
				playtestnote(FIRSTNOTE + editorInfo.epoctave * 12, editorInfo.einum, editorInfo.epchn, gt);
			else
				releasenote(editorInfo.epchn, gt);
		}
		break;

	case KEY_ENTER:
		if (!editorInfo.einum) break;
		switch (editorInfo.eipos)
		{
		case 2:
		case 3:
		case 4:
		case 5:
		{
			int pos;

			if (instr[editorInfo.einum].ptr[editorInfo.eipos - 2])
			{
				if ((editorInfo.eipos == 5) && (shiftpressed))
				{
					instr[editorInfo.einum].ptr[STBL] = makespeedtable(instr[editorInfo.einum].ptr[STBL], finevibrato, 1) + 1;
					break;
				}
				pos = instr[editorInfo.einum].ptr[editorInfo.eipos - 2] - 1;

			}
			else
			{
				pos = gettablelen(editorInfo.eipos - 2);

				if (pos >= MAX_TABLELEN - 1) pos = MAX_TABLELEN - 1;
				if (shiftpressed) instr[editorInfo.einum].ptr[editorInfo.eipos - 2] = pos + 1;
			}
			allowEnterToReturnToPosition();
			gototable(editorInfo.eipos - 2, pos);
			int e = editorInfo.etpos;
			for (int i = 0;i < (VISIBLETABLEROWS - (VISIBLETABLEROWS / 4));i++)
			{
				tabledown();
				validatetableview();

			}
			editorInfo.etpos = e;


		}
		return;

		case 9:
			editorInfo.eipos = 0;
			break;
		}
		break;
	}
	if ((editorInfo.eipos == 9) && (editorInfo.einum)) editstring(instr[editorInfo.einum].name, MAX_INSTRNAMELEN);
	if ((hexnybble >= 0) && (editorInfo.eipos < 9) && (editorInfo.einum))
	{
		unsigned char *ptr = &instr[editorInfo.einum].ad;
		ptr += editorInfo.eipos;

		switch (editorInfo.eicolumn)
		{
		case 0:
			*ptr &= 0x0f;
			*ptr |= hexnybble << 4;
			editorInfo.eicolumn++;
			break;

		case 1:
			*ptr &= 0xf0;
			*ptr |= hexnybble;
			editorInfo.eicolumn++;
			if (editorInfo.eicolumn > 1)
			{
				editorInfo.eicolumn = 0;
				editorInfo.eipos++;
				if (editorInfo.eipos >= 9) editorInfo.eipos = 0;
			}
			break;
		}
		lastEditWindow = -1;	// force redraw of Info bar with updated info
		setTableBackgroundColours(editorInfo.einum);
	}
	// Validate instrument parameters
	if (editorInfo.einum)
	{
		if (!(instr[editorInfo.einum].gatetimer & 0x3f)) instr[editorInfo.einum].gatetimer |= 1;
	}
}


void clearinstr(int num)
{
	memset(&instr[num], 0, sizeof(INSTR));
	if (num)
	{
		if (multiplier)
			instr[num].gatetimer = 2 * multiplier;
		else
			instr[num].gatetimer = 1;

		instr[num].firstwave = 0x9;
	}
}

void gotoinstr(int i)
{
	if (i < 0) return;
	if (i >= MAX_INSTR) return;

	editorInfo.einum = i;
	showinstrtable();

	editorInfo.editmode = EDIT_INSTRUMENT;
}

void nextinstr(void)
{
	editorInfo.einum++;

	sprintf(infoTextBuffer, "instr:%d", editorInfo.einum);

	if (editorInfo.einum >= MAX_INSTR) editorInfo.einum = MAX_INSTR - 1;
	showinstrtable();
}

void previnstr(void)
{
	editorInfo.einum--;
	if (editorInfo.einum < 0) editorInfo.einum = 0;
	showinstrtable();setTableBackgroundColours(editorInfo.einum);
}

void showinstrtable(void)
{
	setTableBackgroundColours(editorInfo.einum);

	if (!editorInfo.etlock)
	{
		int c;

		for (c = MAX_TABLES - 1; c >= 0; c--)
		{
			if (instr[editorInfo.einum].ptr[c])
				settableviewfirst(c, instr[editorInfo.einum].ptr[c] - 1);
		}
	}
}

