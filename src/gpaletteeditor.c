//
// GOATTRACKER ULTRA MIDI Port Select
//

#define GPALETTEEDITOR_C

#include "goattrk2.h"
#include "gpaletteeditor.h"
#include <stdio.h>
#include <dirent.h>


char *RGBText[3] = { "R","G","B" };
int paletteScrollOffset = 0;

/*
TO DO:
1. DEFAULT all 16 palettes to a default palette stored in the .exe
2. Remove gtskin loading
3. Add older palettes (original GT, SIDTracker..)
*/

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
	"Table: Current Instr. Foreground",
	"Table: Current Instr. Background",
	"General: Background",
	"General: Title",
	"General: Info",
	"General: Highlight",
	"TransportBar: Button Background",
	"TransportBar: Background",
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
	"TransportBar: Background2",
	"Top Bar: Background",
	"TransportBar: Between Buttons",
	"Exclamation",
	"Order/Inst: Background",
	"Order/Inst: Foreground",
	"Order/Inst/Table: Edit Foreground",
	"Orderlist: Transpose/Repeat",
	"Top Bar: Foreground",
	"Top Bar: Foreground OFF",
	"TransportBar: Button Foreground"
};


int currentRGB[3];
int copyRGB[3];
char copiedPalette[3][MAX_PALETTE_ENTRIES];
int copyFlag = 0;
struct dirent *paletteFolderEntry;

char *paletteNames[16];

int allowPaletteQuickSave = 0;

void copyRGBInfo()
{

	for (int i = 0;i < 3;i++)
	{
		copyRGB[i] = currentRGB[i];
		copyFlag = 1;
	}
}

int getPaletteTextArraySize()
{
	return sizeof(paletteText) / sizeof(paletteText[0]);
}


void handlePaste(int *cx, GTOBJECT *gt)
{
	if (copyFlag == 0)
		return;

//	int jx = *cx;
	int x = *cx / 2;
	x += paletteScrollOffset;

	if (copyFlag == 1)
	{

		for (int y = 0;y < 3;y++)
		{
			paletteRGB[currentPalettePreset][y][x] = copyRGB[y];
		}
	}
	else
	{
		for (int i = 0;i < MAX_PALETTE_ENTRIES;i++)
		{
			for (int j = 0;j < 3;j++)
			{
				paletteRGB[currentPalettePreset][j][i] = copiedPalette[j][i];
			}
		}
	}

	setGFXPaletteRGBFromPaletteRGB(currentPalettePreset, x);

	paletteChanged = 1;
	displayupdate(gt);


}


void rememberCurrentRGB(int *cx)
{
//	int jx = *cx;
	int x = *cx / 2;
	//jx &= 1;
	x += paletteScrollOffset;

	for (int i = 0;i < 3;i++)
	{
		//	switch (jx)
		//	{
		//	case 0:
		currentRGB[i] = (paletteRGB[currentPalettePreset][i][x]);	// >> 4) & 0xf;
//		break;
//	case 1:
//		currentRGB[i] = paletteRGB[currentPalettePreset][i][x]);	// &0xf;
//		break;
//		}
	}
}

int paletteEdit(int *cx, int *cy, GTOBJECT *gt)
{
	int jx = *cx & 1;
	int x = *cx / 2;
//	jx &= 1;
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

		rememberCurrentRGB(cx);


		setGFXPaletteRGBFromPaletteRGB(currentPalettePreset, x);


		(*cx)++;
		paletteChanged = 1;
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
		allowPaletteQuickSave = 0;	// changed preset. Stop CTRL-S from quick saving. Otherwise you can easily overwrite the wrong palette
	}
}

int quickSavePalette()
{
	FILE *configfile;

	if (allowPaletteQuickSave)
	{
		savePaletteText();
		return 1;
	}
	return 0;		
}




void displayPaletteEditorWindow(GTOBJECT *gt)
{

	int lastclick = 0;
	int exitPaletteEditor = 1;
	int maxPaletteEntries = getPaletteTextArraySize();


	allowPaletteQuickSave = 0;

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

		// Display Info
		sprintf(textbuffer, "%d %s", FIRST_UI_COLOR + (cx / 2) + (paletteScrollOffset * 1), paletteText[(cx / 2) + (paletteScrollOffset * 1)]);

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
		int boxHeight = 9;
		int boxX = 60;
		int boxY = 1;


		if ((mouseb) && (!prevmouseb))
		{
			lastclick = DOUBLECLICKDELAY;

		}
		if (mouseb)
		{
			if ((mousey == boxY + 2) && (mousex < (boxX + 2 + 11)) && (mousex >= (boxX + 2 + 8)))
			{
				if (mouseb & MOUSEB_LEFT && (!prevmouseb))
					changePalettePreset(1, gt);
				else if (mouseb & MOUSEB_RIGHT && (!prevmouseb))
					changePalettePreset(-1, gt);
				rememberCurrentRGB(&cx);
			}
			else if ((mousey == boxY + 2) && (mousex <= (boxX + 18)) && (mousex >= (boxX + 15)))
			{
				if (!prevmouseb)
				{
					savePalette(gt);
					rawkey = 0;	// Stops ENTER / ESC key from quitting palette view too
//					quickSavePalette();
				}
			}
			else if ((mousey == boxY + 3) && (mousex >= (boxX + 18)) && (mousex <= (boxX + 22)))
			{
				handlePaste(&cx, gt);
				rememberCurrentRGB(&cx);
			}
			else if ((mousey == boxY + 2) && (mousex <= (boxX + 37)) && (mousex >= (boxX + 29)))
			{
				copyRGBInfo();
			}
			else if ((mousey == boxY + 2) && (mousex <= (boxX + 27)) && (mousex >= (boxX + 20)))
			{
				for (int i = 0;i < MAX_PALETTE_ENTRIES;i++)
				{
					for (int j = 0;j < 3;j++)
					{
						copiedPalette[j][i] = paletteRGB[currentPalettePreset][j][i];
					}
				}
				copyFlag = 2;
			}


			else if ((mousey < boxY) || (mousey >= boxY + boxHeight) || (mousex < boxX) || (mousex >= boxX + boxWidth))
			{
				if ((!prevmouseb) && (lastclick))
					exitPaletteEditor = 0;
			}
		}

	//	int jx = cx;
		int x = cx / 2;
		//		jx &= 1;
		x += paletteScrollOffset;

		//		jdebug[1] = x;

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
				if (!quickSavePalette())
					savePalette(gt);
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


			if (x < maxPaletteEntries - 1)
			{
				cx++;
				if (cx > (11 * 2) - 1)
				{
					paletteScrollOffset++;
					cx -= 1;
				}
				rememberCurrentRGB(&cx);
			}
			break;

		case KEY_LEFT:
			if (cx + (paletteScrollOffset * 1))
			{
				cx--;
				if (cx < 0)
				{
					cx++;
					paletteScrollOffset--;
				}
				rememberCurrentRGB(&cx);
			}
			break;

		case KEY_C:
			if (ctrlpressed)
				copyRGBInfo();
			break;

		case KEY_V:
			if (ctrlpressed)
				handlePaste(&cx, gt);
			break;

		}


		for (int c = 0;c < boxHeight;c++)
		{
			printblank(boxX, boxY + c, boxWidth);
		}

		drawbox(boxX, boxY, boxColor, boxWidth, boxHeight);

		sprintf(textbuffer, "Palette:%d/%d", currentPalettePreset, (MAX_PALETTE_PRESETS - 1));
		printtext(boxX + 2, boxY + 2, getColor(boxColor, 0), textbuffer);

		if (paletteNames[currentPalettePreset] != NULL)
		{
			sprintf(textbuffer, "%s", paletteNames[currentPalettePreset]);
		}
		else
			sprintf(textbuffer, "<Undefined>");

		printtext(boxX + 2, boxY + 1, getColor(boxColor, 0), textbuffer);

		int colorOffset = (paletteScrollOffset * 1);
		for (int y = 0;y < 3;y++)
		{
			sprintf(textbuffer, "%s", RGBText[y]);
			printtext(boxX + 2, boxY + 4 + y, getColor(boxColor, 0), textbuffer);

			for (int x = 0;x < 11;x++)
			{
				int p = x + paletteScrollOffset;



				sprintf(textbuffer, "%02X ", paletteRGB[currentPalettePreset][y][p]);
				printtext(boxX + 4 + (x * 3), boxY + 4 + y, getColor(boxColor, 0), textbuffer);
				if (y == 0)
				{
					printtext(boxX + 4 + (x * 3), boxY + 4 + 3, getColor(0, colorOffset + FIRST_UI_COLOR), "  ");
					//					printtext(boxX + 5 + (x * 3), boxY + 3 + 3, getColor(0, colorOffset + 1 + FIRST_UI_COLOR), " ");
					colorOffset += 1;
				}

			}
		}
		int cursorX = (cx / 2) * 3 + (cx & 1);
		printbg(boxX + 4 + cursorX, boxY + 4 + cy, cc, 1);

		printtext(boxX + 15, boxY + 2, getColor(0, boxColor), "SAVE");
		printtext(boxX + 20, boxY + 2, getColor(0, boxColor), "COPY ALL");
		printtext(boxX + 29, boxY + 2, getColor(0, boxColor), "COPY RGB");

		printtext(boxX + 18, boxY + 3, getColor(0, boxColor), "PASTE");

		fliptoscreen();

	}
	printmainscreen(gt);
}

void process32EntryPalette(int maxPresets, int maxPaletteEntries, char* tempPalette)
{
	for (int presetIndex = 0;presetIndex < maxPresets;presetIndex++)
	{
		for (int rgb = 0;rgb < 3;rgb++)
		{
			for (int paletteIndex = 0;paletteIndex < maxPaletteEntries;paletteIndex++)
			{
				for (int i = 0;i < (MAX_PALETTE_PRESETS / maxPresets);i++)	// duplicate first x skins y times, to fill all presets
				{
					paletteLoadRGB[presetIndex + (i * 4)][rgb][paletteIndex] = *tempPalette;
				}
				tempPalette++;
			}
		}
	}
}

void convert4BitPaletteTo8Bit()
{
	//	return;

	for (int presetIndex = 0;presetIndex < MAX_PALETTE_PRESETS;presetIndex++)
	{
		for (int rgb = 0;rgb < 3;rgb++)
		{
			int c = 0;
			for (int paletteIndex = 0;paletteIndex < MAX_PALETTE_LOAD_ENTRIES;paletteIndex++)
			{
				int colr = paletteLoadRGB[presetIndex][rgb][paletteIndex];	// colr = 2 nybbles of 4bit color.			

				paletteRGB[presetIndex][rgb][c] = colr & 0xf0;	// convert to 2* 8bit values
				c++;
				paletteRGB[presetIndex][rgb][c] = (colr & 0xf) << 4;
				c++;

			}
		}
	}
}



int savePalette(GTOBJECT *gt)
{
	int done;
	int c;
	char ident[] = { 'G', 'T', 'P', 'A' };

	if (fileselector(paletteFileName, palettepath, palettefilter, "SAVE PALETTE", 3, gt, 12))
	{
		//		done = saveinstrument();

		if (strlen(paletteFileName) < MAX_FILENAME - 6)
		{
			int extfound = 0;
			for (c = strlen(paletteFileName) - 1; c >= 0; c--)
			{
				if (paletteFileName[c] == '.')
					extfound = 1;
			}
			if (!extfound)
				strcat(paletteFileName, ".gtp");
		}

		if (savePaletteText())
		{
			allowPaletteQuickSave = 1;
			win_quitted = 0;
			return 1;
		}
		else
			allowPaletteQuickSave = 0;
	}
	win_quitted = 0;
	return 0;
}

int savePaletteText()
{
	FILE *handle;

	handle = fopen(paletteFileName, "wt");
	if (handle)
	{
		fprintf(handle, ";GTUltra Palette: %s\n\nPALETTEDATA:\n", paletteFileName);

		int size = getPaletteTextArraySize();
		for (int i = 0;i < size;i++)
		{
			int c = i + FIRST_UI_COLOR;
			fprintf(handle, "%02X:%02X,%02X,%02X\t%s\n", i, paletteR[c], paletteG[c], paletteB[c], paletteText[i]);
		}
		fclose(handle);
		return 1;
	}
	return 0;
}


char paletteFile[256];
char paletteStringBuffer[MAX_PATHNAME];
int currentLoadedPresetIndex = 0;


int loadPalette(char *paletteName)
{
	if (currentLoadedPresetIndex >= MAX_PALETTE_PRESETS)
		return -1;	// Already loaded max number of preset palettes

	FILE *handle = fopen(paletteName, "rb");	// Have to use RB instead of RT as the default file is within the binary wad..grrr
	if (handle == NULL)
	{
		jdebug[0] = 0xfe;
		return 0;
	}

	// Read palette text file
	fseek(handle, 0, SEEK_END);
	int size = ftell(handle);
	fseek(handle, 0, SEEK_SET);

	char *paletteMem = malloc(size+1);
	int s = fread(paletteMem, size, 1, handle);
	fclose(handle);
	paletteMem[size] = 0;	// end marker

	// Read palette data from text buffer
	int ret=readPaletteData(paletteMem);
	free(paletteMem);

	return ret;
}

int readPaletteData(char *paletteMem)
{

	int lines = 0;
	int foundPaletteInfo = 0;

	char **p = &paletteMem;

	for (;;)
	{
		// Get text line
		if (sgets(paletteStringBuffer, MAX_PATHNAME, p) == NULL)
		{
			if (foundPaletteInfo)
				currentLoadedPresetIndex++;	// palette loaded. Next time load to the next preset
			return 1;
		}

		if (foundPaletteInfo == 0)
		{
			if (strcmp(paletteStringBuffer, "PALETTEDATA:\n") == 0)
			{
				foundPaletteInfo++;	// we know that the next line will be palette entry info
			}
		}
		else
		{
			char *token;

			/* get the first token */
			token = strtok(paletteStringBuffer, ":");
			if (token != NULL)
			{
				int paletteIndex = convertStringToHex(token);

				token = strtok(NULL, ",");
				if (token == NULL)
					break;
				int red = convertStringToHex(token);
				token = strtok(NULL, ",");
				if (token == NULL)
					break;
				int green = convertStringToHex(token);
				token = strtok(NULL, "\t");
				if (token == NULL)
					break;
				int blue = convertStringToHex(token);

				paletteRGB[currentLoadedPresetIndex][0][paletteIndex] = red;
				paletteRGB[currentLoadedPresetIndex][1][paletteIndex] = green;
				paletteRGB[currentLoadedPresetIndex][2][paletteIndex] = blue;

				/*
				if (paletteIndex == 0x2f)
				{
					// temp
					jdebug[1] = 255;	//red;
					jdebug[2] = green;
					jdebug[3] = blue;
				}
				*/
			}
		}

	}

	if (foundPaletteInfo)
		currentLoadedPresetIndex++;	// palette loaded. Next time load to the next preset

	jdebug[1] = lines;

	return 0;

}


/*
Had to modify this to handle \n manually due to not being able to open "rt".
I add a 0 at the end of the loaded file buffer as a terminator.
I then check for <=0xd for a carriage return to cover 0xd,0xa combinations, rather than =='\n'
*/

// https://stackoverflow.com/questions/20299987/loop-through-char-array-line-by-line
char *sgets(char *s, int n,  char **strp) {
	if (**strp == 0)	// We expect a 0 as a terminator for the char buffer
		return NULL;
	int i;
	for (i = 0;i < n - 1;++i, ++(*strp)) {
		s[i] = **strp;
		if (**strp == 0)
			break;
		if (**strp == 0xd){	//'\n') {
			s[i] = '\n';
			s[i + 1] = '\0';

			while (**strp <= 0xd)	// skip past 0xd,0xa or whatever
			{
				if (**strp == 0)	// end of file?
					break;
				++(*strp);
			}
//			++(*strp);
			break;
		}
	}
	if (i == n - 1)
		s[i] = '\0';
	return s;
}


int convertStringToHex(char *str)
{
	int value = 0;
	for (;;)
	{
		char c = tolower(*str++);
		int h = -1;

		if ((c >= 'a') && (c <= 'f'))
			h = c - 'a' + 10;
		if ((c >= '0') && (c <= '9'))
			h = c - '0';

		if (h >= 0)
		{
			value *= 16;
			value += h;
		}
		else break;
	}
	return value;
}



int loadPalettes()
{
	DIR *folder;

	createFilename(appFileName, paletteFile, "gtpalettes");

	folder = opendir(paletteFile);
	if (folder == NULL)
	{
		jdebug[0] = 0xff;
		perror("Unable to read directory");
		return(1);
	}

	while ((paletteFolderEntry = readdir(folder)))
	{
		if (paletteFolderEntry->d_name[0] == '.')	// skip . and .. (first two folder entries)
			continue;

		createFilename(appFileName, paletteFile, "gtpalettes");
		strcat(paletteFile, "/");	//"\\");	// Changed to / to hopefully allow both Windows + Linux loading work
		strcat(paletteFile, paletteFolderEntry->d_name);	// create full path

		paletteNames[currentLoadedPresetIndex] = malloc(sizeof(paletteFolderEntry->d_name));
		strcpy(paletteNames[currentLoadedPresetIndex], paletteFolderEntry->d_name);	// copy filename. This is saved in the cfg file

		loadPalette(paletteFile);
	}

	closedir(folder);

	return(0);
}