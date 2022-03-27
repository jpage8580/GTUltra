//
// GOATTRACKER ULTRA Char Editor
//

#define GCHAREDITOR_C

#include "goattrk2.h"
#include "gchareditor.h"


int editPixelX = 0;
int editPixelY = 0;
int editPaletteX = 0;
int editPaletteY = 0;
int drawEnable = 0;
int selectedCharIndex = 0;

int copyCharIndex = -1;

FILE *fileHandle;

int sketchArray[100];

void displayCharWindow()
{
	int lastclick = 0;
	int exitCharEditor = 1;

	while (exitCharEditor)
	{
		int cc = cursorcolortable[cursorflash];
		if (cursorflashdelay >= 6)
		{
			cursorflashdelay %= 6;
			cursorflash++;
			cursorflash &= 3;
		}

		fliptoscreen();
		getkey();
		if (lastclick)
			lastclick--;

		if (win_quitted)
			return;

		int boxColor = 0xe;
		int boxWidth = 92;
		int boxHeight = 24;
		int boxX = 3;
		int boxY = 3;
		int charEditX = boxX + 40;
		int charEditY = boxY + 2;

		int charSketchX = charEditX + 20;
		int charSketchY = charEditY;

		int charPaletteX = boxX + 2;
		int charPaletteY = boxY + 5;

		if ((mouseb) && (!prevmouseb))
		{
			lastclick = DOUBLECLICKDELAY;

		}
		if (mouseb)
		{
			

			if ((mousey < boxY) || (mousey >= boxY + boxHeight) || (mousex < boxX) || (mousex >= boxX + boxWidth))
			{
				if ((!prevmouseb) && (lastclick))
					exitCharEditor = 0;
			}
			else if (mousex >= charEditX && mousex < charEditX + (8 * 2) && mousey >= charEditY && mousey < charEditY + 14)
			{

				editPixelY = mousey - charEditY;
				editPixelX = (mousex - charEditX) / 2;
				if (!prevmouseb)
				{
					drawEnable = getPixel(selectedCharIndex, editPixelX, editPixelY);
					if (drawEnable)
						drawEnable = 0;
					else
						drawEnable = 1;
				}
				setPixel(selectedCharIndex, editPixelX, editPixelY, drawEnable);
				forceRedraw();
				gfx_redraw = 1;
			}
			else if (mousex >= charPaletteX && mousex < charPaletteX + 32 && mousey >= charPaletteY && mousey < charPaletteY + 8)
			{
				editPaletteX = mousex - charPaletteX;
				editPaletteY = mousey - charPaletteY;
			}
			else if (mousex >= charSketchX && mousex < charSketchX + 10 && mousey >= charSketchY && mousey < charSketchY + 10)
			{
				sketchArray[((mousey- charSketchY) * 10) + (mousex- charSketchX)] = selectedCharIndex;
			}
		}

		for (int c = 0;c < boxHeight;c++)
		{
			printblank(boxX, boxY + c, boxWidth);
		}

		for (int y = 0;y < 10;y++)
		{
			for (int x = 0;x < 10;x++)
			{
				printbyte(x + charSketchX, y + charSketchY, boxColor, sketchArray[(y * 10) + x]);
			}
		}

		int chr = 0;
		for (int y = 0;y < 8;y++)
		{
			for (int x = 0;x < 32;x++)
			{
				printbyte(charPaletteX + x, charPaletteY + y, boxColor, chr);
				chr++;
			}
		}

		switch (rawkey)
		{
		case KEY_ESC:
			exitCharEditor = 0;
			break;

		case KEY_C:
			if (ctrlpressed)
			{
				copyCharIndex = selectedCharIndex;
			}
			break;

		case KEY_F:
			if (ctrlpressed)
			{
				flipCharX(selectedCharIndex);
			}
			break;

		case KEY_I:
			if (ctrlpressed)
			{
				inverseChar(selectedCharIndex);
			}
			break;

		case KEY_V:
			if (ctrlpressed)
			{
				if (copyCharIndex >= 0)
				{
					copyChar(copyCharIndex, selectedCharIndex);
				}
			}
			break;

			// CTRL-S to save charset
		case KEY_S:
			if (ctrlpressed)
			{
				fileHandle = fopen("charset.bin", "wb");		// wb write binary. wt = write text
				if (fileHandle)
				{
					fwrite(chardata, 4096, 1, fileHandle);
					fclose(fileHandle);
				}
			}
			break;
		}

		selectedCharIndex = (editPaletteY * 32) + editPaletteX;
		displayZoomedChar(selectedCharIndex, charEditX, charEditY, boxColor);
		// Not quite square - but close enough 2x2 for each pixel

		drawbox(3, 3, boxColor, boxWidth, boxHeight);

		drawbox(charEditX - 1, charEditY - 1, boxColor, (8 * 2) + 2, 14 + 2);
		drawbox(boxX + 2 - 1, boxY + 5 - 1, boxColor, 32 + 2, 8 + 2);

		drawbox(charSketchX - 1, charSketchY - 1, boxColor, 12, 12);

		printtext(50 - (MAX_FILENAME + 10) / 2 + 1, 4, getColor(0, boxColor), "Char Editor");

		int cx = selectedCharIndex % 32;
		int cy = selectedCharIndex / 32;
		printbg(charPaletteX + cx, charPaletteY + cy, cc, 1);

		// display cursors
		cx = editPixelX * 2;
		printbg(charEditX + cx, charEditY + editPixelY, cc, 2);


		sprintf(textbuffer, "Char: $%02X (%d)", selectedCharIndex, selectedCharIndex);
		printtext(boxX + 2, boxY + 3, boxColor, textbuffer);

		fliptoscreen();
	}
}

void displayZoomedChar(int charToDisplay, int xp, int yp, int color)
{

	unsigned char *chptr = &chardata[charToDisplay * 16];
	char outChar;
	for (int y = 0;y < 14;y++)
	{
		unsigned char c = *chptr++;

		for (int x = 0;x < 8;x++)
		{
			int a = c & (128 >> x);
			if (a)
				outChar = 0xf4;
			else
				outChar = '.';

			printbyte(xp + (x * 2), yp + y, color, outChar);
			printbyte(xp + 1 + (x * 2), yp + y, color, outChar);
		}
	}

}

void inverseChar(int charIndex)
{
	unsigned char *chptr = &chardata[(charIndex * 16)];
	for (int i = 0;i < 16;i++)
	{
		unsigned char c = *chptr;
		*chptr = 255 - c;
		chptr++;
	}
}


void flipCharX(int charIndex)
{
	unsigned char *chptr = &chardata[(charIndex * 16)];
	for (int i = 0;i < 16;i++)
	{
		unsigned char c = *chptr;
		int b = 0;
		for (int j = 0;j < 8;j++)
		{
			int a = c & (128 >> j);
			if (a)
				b |= (1 << j);
		}
		*chptr = b;
		chptr++;
	}
}


void copyChar(int sourceCharIndex, int destCharIndex)
{
	unsigned char *sourceChptr = &chardata[(sourceCharIndex * 16)];
	unsigned char *destChptr = &chardata[(destCharIndex * 16)];

	for (int i = 0;i < 16;i++)
	{
		*destChptr = *sourceChptr;
		sourceChptr++;
		destChptr++;
	}
}

void setPixel(int charToDisplay, int x, int y, int onOff)
{
	unsigned char *chptr = &chardata[(charToDisplay * 16) + y];
	unsigned char c = *chptr;

	if (onOff)	// set pixel to ON
		c |= (128 >> x);
	else
		c &= 255 - ((128 >> x));
	*chptr = c;
}



int getPixel(int charToDisplay, int x, int y)
{
	unsigned char *chptr = &chardata[(charToDisplay * 16) + y];
	unsigned char c = *chptr;
	return (c & (128 >> x));
}

void saveCharset()
{

}

