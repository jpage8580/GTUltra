//
// GOATTRACKER ULTRA MIDI Port Select
//

#define GPALETTEEDITOR_C

#include "goattrk2.h"
#include "gpaletteeditor.h"


char *RGBText[3] = { "R","G","B" };
int paletteScrollOffset = 0;


char* paletteText[] = {
	"Pattern: Background 1",
	"Pattern: Foreground 1",
	"Pattern: Background 2",
	"Pattern: Foreground 2",
	"Pattern: Highlight Background",
	"Pattern: Highlight Foreground",
	"Pattern: Note Foreground",
	"Pattern: Command Foreground",
	"Pattern: Vertical Line",
	"Pattern: Playing Line Background",
	"Pattern: Playing Line Foreground",
	"Pattern: Unused Background",
	"Pattern: Unused Foreground",
	"Select: Background",
	"Select: Foreground",
	"Table: Background 1",
	"Table: Foreground 1",
	"Table: Background 2",
	"Table: Foreground 2",
	"Table: Unused Background",
	"Table: Unused Foreground",
	"Table: Current Instr. Background",
	"Table: Current Instr. Foreground",
	"General: Background",
	"General: Title",
	"General: Info",
	"General: Highlight",
	"TransportBar: Background",
	"TransportBar: Foreground",
	"TransportBar: Enabled",
	"RED (record / Filter Status)",
	"Pattern Loop Marker Background",
	"Pattern Loop Marker Foreground",
	"Pattern: Instrument Foreground",
	"Pattern: First Background 1",
	"Pattern: First Foreground 1",
	"Pattern: First Background 2",
	"Pattern: First Foreground 2",
	"Pattern: Index Highlight",
	"hello"
};

int getPaletteTextArraySize()
{
	return sizeof(paletteText) / sizeof(paletteText[0]);
}



int paletteEdit(int *cx, int *cy, GTOBJECT *gt)
{
	int jx = *cx;
	int x = jx / 2;
	jx &= 1;
	//	x += (*cx & 1);
	x += paletteScrollOffset;
	int y = *cy;


	if (hexnybble >= 0)
	{
		int lastEditPos = editorInfo.eseditpos;
		switch (jx)
		{
		case 0:

			paletteRGB[currentPalettePreset][y][x] &= 0x0f;
			paletteRGB[currentPalettePreset][y][x] |= hexnybble << 4;
			break;

		case 1:
			paletteRGB[currentPalettePreset][y][x] &= 0xf0;
			paletteRGB[currentPalettePreset][y][x] |= hexnybble;
			break;
		}

		setGFXPaletteRGBFromPaletteRGB(currentPalettePreset, x);


		(*cx)++;
		paletteChanged = 1;
		//		gfx_redraw = 1;
//		printmainscreen(gt);
		displayupdate(gt);

		return 1;
	}
	return 0;
}

void changePalettePreset(int change, GTOBJECT *gt)
{
	int ch = 0;
	if (change > 0)
	{
		if (currentPalettePreset < (MAX_PALETTE_PRESETS - 1))
		{
			currentPalettePreset++;
			ch++;
		}
	}
	else if (currentPalettePreset)
	{
		currentPalettePreset--;
		ch++;
	}

	if (ch)
	{
		setSkin(currentPalettePreset);
		displayupdate(gt);
	}
}

void savePalette()
{
	FILE *configfile;

	configfile = fopen(skinFilename, "wb");		// wb write binary. wt = write text
	if (configfile)
	{
		fwrite(&paletteRGB, MAX_PALETTE_PRESETS * 3 * MAX_PALETTE_ENTRIES, 1, configfile);
		fclose(configfile);
	}
}

void displayPaletteEditorWindow(GTOBJECT *gt)
{

	int lastclick = 0;
	int exitPaletteEditor = 1;
	int maxPaletteEntries = getPaletteTextArraySize();


	int cx = 0;
	int cy = 0;

	win_enableKeyRepeat();
	//	currentPalettePreset

	while (exitPaletteEditor)
	{
		cursorflashdelay++;
		int cc = getColor(0, cursorcolortable[cursorflash]);
		if (cursorflashdelay >= 6)
		{
			cursorflashdelay %= 6;
			cursorflash++;
			cursorflash &= 3;
		}

		sprintf(textbuffer, "%d %s", FIRST_UI_COLOR + cx + (paletteScrollOffset * 2), paletteText[cx + (paletteScrollOffset * 2)]);

		printbyterow(6, TRANSPORT_BAR_Y + 2, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), 32, 40);
		printtext(6, TRANSPORT_BAR_Y + 2, getColor(CINFO_FOREGROUND, CTRANSPORT_FOREGROUND), textbuffer);

		fliptoscreen();
		getkey();

		converthex();
		paletteEdit(&cx, &cy, gt);


		if (lastclick)
			lastclick--;

		if (win_quitted)
			return;

		int boxColor = 0xe;
		int boxWidth = 39;
		int boxHeight = 8;
		int boxX = 60;
		int boxY = 2;


		if ((mouseb) && (!prevmouseb))
		{
			lastclick = DOUBLECLICKDELAY;

		}
		if (mouseb)
		{
			if ((mousey == boxY + 1) && (mousex < (boxX + 2 + 11)) && (mousex >= (boxX + 2 + 8)))
			{
				if (mouseb & MOUSEB_LEFT && (!prevmouseb))
					changePalettePreset(1, gt);
				else if (mouseb & MOUSEB_RIGHT && (!prevmouseb))
					changePalettePreset(-1, gt);
			}
			else if ((mousey == boxY + 1) && (mousex < (boxX + 24)) && (mousex >= (boxX + 20)))
			{
				if (!prevmouseb)
					savePalette();
			}

			else if ((mousey < boxY) || (mousey >= boxY + boxHeight) || (mousex < boxX) || (mousex >= boxX + boxWidth))
			{
				if ((!prevmouseb) && (lastclick))
					exitPaletteEditor = 0;
			}
		}

		switch (rawkey)
		{
		case KEY_ESC:
			exitPaletteEditor = 0;
			break;

		case KEY_ENTER:
			exitPaletteEditor = 0;
			break;

		case KEY_S:
			if (ctrlpressed)
			{
				savePalette();
			}
			break;

		case KEY_UP:
			if (cy)
				cy--;
			break;
		case KEY_DOWN:
			if (cy < 2)
				cy++;
			break;

		case KEY_RIGHT:
			if ((cx + (paletteScrollOffset * 2) < maxPaletteEntries - 1))
			{
				cx++;
				if (cx > (11 * 2) - 1)
				{
					paletteScrollOffset++;
					cx -= 1;
				}
			}
			break;

		case KEY_LEFT:
			if (cx + (paletteScrollOffset * 2))
			{
				cx--;
				if (cx < 0)
				{
					cx++;
					paletteScrollOffset--;
				}
			}
			break;

		}


		for (int c = 0;c < boxHeight;c++)
		{
			printblank(boxX, boxY + c, boxWidth);
		}

		drawbox(boxX, boxY, boxColor, boxWidth, boxHeight);

		sprintf(textbuffer, "Palette:%d/3", currentPalettePreset);
		printtext(boxX + 2, boxY + 1, getColor(boxColor, 0), textbuffer);

		int colorOffset = (paletteScrollOffset * 2);
		for (int y = 0;y < 3;y++)
		{
			sprintf(textbuffer, "%s", RGBText[y]);
			printtext(boxX + 2, boxY + 3 + y, getColor(boxColor, 0), textbuffer);

			for (int x = 0;x < 11;x++)
			{
				int p = x + paletteScrollOffset;



				sprintf(textbuffer, "%02X ", paletteRGB[currentPalettePreset][y][p]);
				printtext(boxX + 4 + (x * 3), boxY + 3 + y, getColor(boxColor, 0), textbuffer);
				if (y == 0)
				{
					printtext(boxX + 4 + (x * 3), boxY + 3 + 3, getColor(0, colorOffset + FIRST_UI_COLOR), " ");
					printtext(boxX + 5 + (x * 3), boxY + 3 + 3, getColor(0, colorOffset + 1 + FIRST_UI_COLOR), " ");
					colorOffset += 2;
				}

			}
		}
		int cursorX = (cx / 2) * 3 + (cx & 1);
		printbg(boxX + 4 + cursorX, boxY + 3 + cy, cc, 1);

		printtext(boxX + 20, boxY + 1, getColor(0, boxColor), "SAVE");

		fliptoscreen();

	}
	printmainscreen(gt);
}

