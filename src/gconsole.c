/*
 * GOATTRACKER "console" output routines
 */

#define GCONSOLE_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "goattrk2.h"

FILE *charset;

int gfxinitted = 0;
unsigned *scrbuffer = NULL;
unsigned *colorbuffer = NULL;
unsigned *prevscrbuffer = NULL;
unsigned *prevcolorbuffer = NULL;

unsigned char *chardata = NULL;
int key = 0;
int rawkey = 0;
int shiftpressed = 0;
int shiftOrCtrlPressed = 0;
int ctrlpressed = 0;
int bothShiftAndCtrlPressed = 0;
int cursorflashdelay = 0;
int mouseb = 0;
int prevmouseb = 0;
unsigned mousex = 0;
unsigned mousey = 0;
unsigned mousepixelx = 0;
unsigned mousepixely = 0;
unsigned oldmousepixelx = 0xffffffff;
unsigned oldmousepixely = 0xffffffff;
int mouseheld = 0;
int region[MAX_ROWS];
int fontwidth = 8;
int fontheight = 14;
int mousesizex = 11;
int mousesizey = 20;
unsigned bigwindow = 1;

void loadexternalpalette(void);
void initicon(void);

inline void setcharcolor(unsigned *dptr, unsigned *cptr, short ch, short color)
{
	*dptr = (ch & 0xff) | UIUnderline;

	*cptr = color;
}

inline void setcolor(unsigned *dptr, unsigned *cptr, short color)
{

	*cptr = color;
}

int initscreen(void)
{
	int handle;

	if (bigwindow - 1)
	{
		fontwidth *= bigwindow;
		fontheight *= bigwindow;
		mousesizex *= 2;
		mousesizey *= 2;
	}

	unsigned xsize = MAX_COLUMNS * fontwidth;
	unsigned ysize = MAX_ROWS * fontheight;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
		return 0;
	win_openwindow(xsize, ysize, "GoatTracker Ultra - (Enhanced GoatTracker Stereo V2.76 - Jason Page / MSL)", NULL);
	win_setmousemode(MOUSE_ALWAYS_HIDDEN);
	initicon();

	if (!gfx_init(xsize, ysize, 60, 0))
	{
		win_fullscreen = 0;
		if (!gfx_init(xsize, ysize, 60, 0))
			return 0;
	}

	scrbuffer = (unsigned*)malloc(MAX_COLUMNS * MAX_ROWS * sizeof(unsigned));
	colorbuffer = (unsigned*)malloc(MAX_COLUMNS * MAX_ROWS * sizeof(unsigned));

	prevscrbuffer = (unsigned*)malloc(MAX_COLUMNS * MAX_ROWS * sizeof(unsigned));
	prevcolorbuffer = (unsigned*)malloc(MAX_COLUMNS * MAX_ROWS * sizeof(unsigned));

	if ((!scrbuffer) || (!prevscrbuffer)) return 0;

	memset(region, 0, sizeof region);

	chardata = (unsigned char*)malloc(4096);
	if (!chardata) return 0;

	charset = fopen("charset.bin", "rb");		//Have we a local copy of the charset? If so, use that. Otherwise, use the one in the wad file..
	if (!charset)
	{
		handle = io_open("chargen.bin");
		if (handle == -1) return 0;
		io_read(handle, &chardata[0], 4096);
		io_close(handle);
	}
	else
	{
		fread(chardata, 4096, 1, charset);
		fclose(charset);
	}



	gfx_loadpalette("palette.bin");
	loadexternalpalette();


	gfx_setPaletteRGB(1, 100, 40, 100);
	gfx_setPaletteRGB(9, 150, 0, 150);


	gfx_setpalette();

	if (bigwindow - 1) gfx_loadsprites(0, "bcursor.bin");
	else gfx_loadsprites(0, "cursor.bin");

	gfxinitted = 1;
	clearscreen(getColor(7, CGENERAL_BACKGROUND));
	atexit(closescreen);
	return 1;
}

void loadexternalpalette(void)
{
	FILE *ext_f;
	if ((ext_f = fopen("custom.pal", "rt")))
	{
		int p = 0;
		char ln[100];
		strcpy(ln, "");
		fgets(ln, sizeof(ln), ext_f);

		if (strncmp("JASC-PAL", ln, 8) == 0)
		{
			int colors;
			fgets(ln, sizeof(ln), ext_f);
			fgets(ln, sizeof(ln), ext_f);
			if (sscanf(ln, "%d", &colors) == 1 && colors == 256)
			{
				while (!feof(ext_f))
				{
					int r, g, b;
					if (!fgets(ln, sizeof(ln), ext_f)) break;
					if (sscanf(ln, "%d %d %d", &r, &g, &b) == 3)
					{
						// JASC palette is 8-bit and goat palette is 6-bit
						gfx_palette[p++] = r / 4;
						gfx_palette[p++] = g / 4;
						gfx_palette[p++] = b / 4;
					}

					if (p >= 768) break;
				}
				gfx_calcpalette(64, 0, 0, 0);
			}
		}

		fclose(ext_f);
	}
}

void initicon(void)
{
	int handle = io_open("goattrk2.bmp");
	if (handle != -1)
	{
		SDL_RWops *rw;
		SDL_Surface *icon;
		char *iconbuffer;
		int size;

		size = io_lseek(handle, 0, SEEK_END);
		io_lseek(handle, 0, SEEK_SET);
		iconbuffer = (char*)malloc(size);
		if (iconbuffer)
		{
			io_read(handle, iconbuffer, size);
			io_close(handle);
			rw = SDL_RWFromMem(iconbuffer, size);
			icon = SDL_LoadBMP_RW(rw, 0);
			//			SDL_WM_SetIcon(icon, 0);
			SDL_SetWindowIcon(win_window, icon);
			free(iconbuffer);
		}
	}
}
void closescreen(void)
{
	if (scrbuffer)
	{
		free(scrbuffer);
		scrbuffer = NULL;
	}
	if (prevscrbuffer)
	{
		free(prevscrbuffer);
		prevscrbuffer = NULL;
	}
	if (chardata)
	{
		free(chardata);
		chardata = NULL;
	}

	gfxinitted = 0;
}

void clearscreen(int backColor)
{
	int c;
	unsigned *dptr = scrbuffer;
	unsigned *cptr = colorbuffer;

	if (!gfxinitted) return;

	for (c = 0; c < MAX_ROWS * MAX_COLUMNS; c++)
	{
		setcharcolor(dptr, cptr, 0x20, backColor);	//   0x7);
		dptr++;
		cptr++;
	}
}

int getColor(int fcolor, int bcolor)
{
	int color = fcolor | (bcolor << 8);

	//jcounter = color;
	return color;
}

void printtext(int x, int y, int color, const char *text)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);

	if (!gfxinitted) return;
	if (y < 0) return;
	if (y >= MAX_ROWS) return;
	while (*text)
	{
		setcharcolor(dptr, cptr, *text, color);
		dptr++;
		cptr++;
		text++;
	}
}

void printbyterow(int x, int y, int color, char b, int length)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);
	for (int i = 0;i < length;i++)
	{
		setcharcolor(dptr, cptr, b, color);
		dptr++;
		cptr++;
	}
}

void printbytecol(int x, int y, int color, char b, int length)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);
	for (int i = 0;i < length;i++)
	{
		setcharcolor(dptr, cptr, b, color);
		dptr += MAX_COLUMNS;
		cptr += MAX_COLUMNS;
	}
}


void printbyte(int x, int y, int color, char b)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);
	setcharcolor(dptr, cptr, b, color);
}

void printtextc(int y, int color, const char *text)
{
	int x = (MAX_COLUMNS - strlen(text)) / 2;

	printtext(x, y, color, text);
}

void printtextcp(int cp, int y, int color, const char *text)
{
	int x = cp - (strlen(text) / 2);

	printtext(x, y, color, text);
}


void printblank(int x, int y, int length)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);

	if (!gfxinitted) return;
	if (y < 0) return;
	if (y >= MAX_ROWS) return;
	while (length--)
	{
		setcharcolor(dptr, cptr, 0x20, 0x7);
		dptr++;
		cptr++;
	}
}

void printblankc(int x, int y, int color, int length)
{
	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);

	if (!gfxinitted) return;
	if (y < 0) return;
	if (y >= MAX_ROWS) return;
	while (length--)
	{
		setcharcolor(dptr, cptr, 0x20, color);
		dptr++;
		cptr++;
	}
}

void drawbox(int x, int y, int color, int sx, int sy)
{
	unsigned *dptr;
	unsigned *dptr2;
	int counter;

	if (!gfxinitted) return;
	if (y < 0) return;
	if (y >= MAX_ROWS) return;
	if (y + sy > MAX_ROWS) return;
	if ((!sx) || (!sy)) return;

	dptr = scrbuffer + (x + y * MAX_COLUMNS);
	dptr2 = scrbuffer + ((x + sx - 1) + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr2 = colorbuffer + ((x + sx - 1) + y * MAX_COLUMNS);

	counter = sy;

	while (counter--)
	{

		setcharcolor(dptr, cptr, 0xfe, color);
		setcharcolor(dptr2, cptr2, 0xfd, color);
		dptr += MAX_COLUMNS;
		dptr2 += MAX_COLUMNS;
		cptr += MAX_COLUMNS;
		cptr2 += MAX_COLUMNS;
	}

	dptr = scrbuffer + (x + y * MAX_COLUMNS);
	dptr2 = scrbuffer + (x + (y + sy - 1) * MAX_COLUMNS);
	cptr = colorbuffer + (x + y * MAX_COLUMNS);
	cptr2 = colorbuffer + (x + (y + sy - 1) * MAX_COLUMNS);

	counter = sx;

	while (counter--)
	{
		setcharcolor(dptr, cptr, 0xfc, color);
		setcharcolor(dptr2, cptr2, 0xfb, color);
		dptr++;
		dptr2++;
		cptr++;
		cptr2++;
	}

	dptr = scrbuffer + (x + y * MAX_COLUMNS);
	cptr = colorbuffer + (x + y * MAX_COLUMNS);
	setcharcolor(dptr, cptr, 0xfa, color);

	dptr = scrbuffer + ((x + sx - 1) + y * MAX_COLUMNS);
	cptr = colorbuffer + ((x + sx - 1) + y * MAX_COLUMNS);
	setcharcolor(dptr, cptr, 0xf9, color);

	dptr = scrbuffer + (x + (y + sy - 1) * MAX_COLUMNS);
	cptr = colorbuffer + (x + (y + sy - 1) * MAX_COLUMNS);
	setcharcolor(dptr, cptr, 0xf8, color);

	dptr = scrbuffer + ((x + sx - 1) + (y + sy - 1) * MAX_COLUMNS);
	cptr = colorbuffer + ((x + sx - 1) + (y + sy - 1) * MAX_COLUMNS);
	setcharcolor(dptr, cptr, 0xf7, color);
}

void printbg(int x, int y, int color, int length)
{

	unsigned *dptr = scrbuffer + (x + y * MAX_COLUMNS);
	unsigned *cptr = colorbuffer + (x + y * MAX_COLUMNS);

	if (!gfxinitted) return;
	if (y < 0) return;
	if (y >= MAX_ROWS) return;
	while (length--)
	{
		setcolor(dptr, cptr, color);
		dptr++;
		cptr++;
	}
}

void fliptoscreen(void)
{
	unsigned *sptr = scrbuffer;
	unsigned *cmpptr = prevscrbuffer;

	unsigned *cptr = colorbuffer;
	unsigned *colorcmpptr = prevcolorbuffer;

	int x, y;
	int regionschanged = 0;

	if (!gfxinitted) return;

	// Mark previous mousecursor area changed if mouse moved
	if ((mousepixelx != oldmousepixelx) || (mousepixely != oldmousepixely))
	{
		if ((oldmousepixelx >= 0) && (oldmousepixely >= 0))
		{
			int sy = oldmousepixely / fontheight;
			int ey = (oldmousepixely + mousesizey - 1) / fontheight;
			int sx = oldmousepixelx / fontwidth;
			int ex = (oldmousepixelx + mousesizex - 1) / fontwidth;

			if (ey >= MAX_ROWS) ey = MAX_ROWS - 1;
			if (ex >= MAX_COLUMNS) ex = MAX_COLUMNS - 1;

			for (y = sy; y <= ey; y++)
			{
				for (x = sx; x <= ex; x++)
				{
					prevscrbuffer[y*MAX_COLUMNS + x] = 0xffffffff;
					prevcolorbuffer[y*MAX_COLUMNS + x] = 0xffffffff;
				}
			}
		}
	}

	// If redraw requested, mark whole screen changed
	if (gfx_redraw)
	{
		gfx_redraw = 0;
		memset(prevscrbuffer, 0xff, MAX_COLUMNS*MAX_ROWS * sizeof(unsigned));
		memset(prevcolorbuffer, 0xff, MAX_COLUMNS*MAX_ROWS * sizeof(unsigned));
	}

	if (!gfx_lock()) return;

	// Now redraw text on changed areas
	for (y = 0; y < MAX_ROWS; y++)
	{
		for (x = 0; x < MAX_COLUMNS; x++)
		{
			// Check if char changed
			if ((*sptr != *cmpptr) || (*cptr != *colorcmpptr))
			{
				*cmpptr = *sptr;
				*colorcmpptr = *cptr;

				region[y] = 1;
				regionschanged = 1;

				{

					unsigned char *chptr = &chardata[(*sptr & 0xff) * 16];
					unsigned char *dptr = (unsigned char*)gfx_screen->pixels + y * fontheight * gfx_screen->pitch + x * fontwidth;

					int underline = (*sptr) & UNDERLINE_MASK;
					int underlineForeground = (*sptr) & UNDERLINE_FOREGROUND_MASK;
					unsigned char bgcolor = (*cptr) >> 8;	// (*sptr) >> 20;
					unsigned char fgcolor = (*cptr) & 0xff;				//((*sptr) >> 16) & 0xf;

					int c;
					unsigned char e = *chptr;	//++;

					for (c = 0; c < 14; c++)
					{
						if (c == 13)
						{
							if (underline)
							{
								bgcolor = CPATTERN_DIVIDER_LINE;	//0xf;	// JP Underline test
								fgcolor = bgcolor;
							}
							else if (underlineForeground)
							{
								bgcolor = fgcolor;
							}
						}


						e = *chptr++;

						switch (bigwindow)
						{
						case 1:
							if (e & 128) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 64) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 32) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 16) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 8) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 4) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 2) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							if (e & 1) *dptr++ = fgcolor;
							else *dptr++ = bgcolor;
							dptr += gfx_screen->pitch - fontwidth;
							break;
						case 2:
							if (e & 128) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							break;
						case 3:
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							break;
						case 4:
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							*dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor;
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							if (e & 128) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 64) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 32) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 16) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 8) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 4) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 2) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							if (e & 1) { *dptr++ = bgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; *dptr++ = fgcolor; }
							else { *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; *dptr++ = bgcolor; }
							dptr += gfx_screen->pitch - fontwidth;
							break;
						}
					}
				}
			}
			sptr++;
			cmpptr++;

			cptr++;
			colorcmpptr++;
		}


	}


	// Redraw mouse if text was redrawn
	if (regionschanged)
	{
		int sy = mousepixely / fontheight;
		int ey = (mousepixely + mousesizey - 1) / fontheight;
		if (ey >= MAX_ROWS) ey = MAX_ROWS - 1;

		gfx_drawsprite(mousepixelx, mousepixely, 0x1);
		for (y = sy; y <= ey; y++)
			region[y] = 1;
	}

	// Store current mouse position as old
	oldmousepixelx = mousepixelx;
	oldmousepixely = mousepixely;

	// Redraw changed screen regions
	gfx_unlock();
	gfx_flip();
	/*
	for (y = 0; y < MAX_ROWS; y++)
		{
			if (region[y])
			{
				SDL_UpdateRect(gfx_screen, 0, y*fontheight, MAX_COLUMNS*fontwidth, fontheight);
				region[y] = 0;
			}
		}
	*/
}

void forceRedraw()
{
	for (int y = 0; y < MAX_ROWS; y++)
	{
		region[y] = 1;
		//		regionschanged = 1;
	}
}


int mouseTicks = 0;
int mouseTicksDelta = 0;
int mousebDoubleClick = 0;

int keyDownCount = 0;

void getkey(void)
{
	int c;
	win_asciikey = 0;
	cursorflashdelay += win_getspeed(50);


	prevmouseb = mouseb;

	mou_getpos(&mousepixelx, &mousepixely);
	mouseb = mou_getbuttons();
	mousex = mousepixelx / fontwidth;
	mousey = mousepixely / fontheight;

	if (mouseb)
	{
		mouseheld++;
		if (!prevmouseb)
		{
			mouseTicksDelta = SDL_GetTicks() - mouseTicks;
			mouseTicks = SDL_GetTicks();
			if (mouseTicksDelta < 250)
				mousebDoubleClick = 1;
			else
				mousebDoubleClick = 0;
		}
	}
	else
	{

		mouseheld = 0;
	}

	keyDownCount = 0;
	key = win_asciikey;
	rawkey = 0;
	for (c = 0; c < SDL_NUM_SCANCODES; c++)
	{
		if (win_keystate[c])
			keyDownCount++;
		if (win_keytable[c])
		{
			if ((c != SDL_SCANCODE_LSHIFT) && (c != SDL_SCANCODE_RSHIFT) &&
				(c != SDL_SCANCODE_LCTRL) && (c != SDL_SCANCODE_RCTRL))
			{
				rawkey = c;
				win_keytable[c] = 0;
				break;
			}
		}
	}

	shiftOrCtrlPressed = 0;
	ctrlpressed = 0;
	shiftpressed = 0;
	bothShiftAndCtrlPressed = 0;

	if (win_keystate[SDL_SCANCODE_LCTRL] || win_keystate[SDL_SCANCODE_RCTRL])
	{
		ctrlpressed = 1;
	}

	if ((win_keystate[SDL_SCANCODE_LSHIFT]) || (win_keystate[SDL_SCANCODE_RSHIFT]))
		shiftpressed = 1;

	shiftOrCtrlPressed = shiftpressed | ctrlpressed;
	bothShiftAndCtrlPressed = shiftpressed * ctrlpressed;

	if (rawkey == SDL_SCANCODE_KP_ENTER)
	{
		key = KEY_ENTER;
		rawkey = SDL_SCANCODE_RETURN;
	}

	if (rawkey == SDL_SCANCODE_KP_0) key = '0';
	if (rawkey == SDL_SCANCODE_KP_1) key = '1';
	if (rawkey == SDL_SCANCODE_KP_2) key = '2';
	if (rawkey == SDL_SCANCODE_KP_3) key = '3';
	if (rawkey == SDL_SCANCODE_KP_4) key = '4';
	if (rawkey == SDL_SCANCODE_KP_5) key = '5';
	if (rawkey == SDL_SCANCODE_KP_6) key = '6';
	if (rawkey == SDL_SCANCODE_KP_7) key = '7';
	if (rawkey == SDL_SCANCODE_KP_8) key = '8';
	if (rawkey == SDL_SCANCODE_KP_9) key = '9';
}



// JP - MODIFY CHARSET. THIS CHANGES CHAR 255. USED TO DISPLAY PATTERN DIVIDERS
void modifyChar(int c, char *data);

void modifyChars()
{
	char dividerLine[14] = { 12,12,12,12,12,12,12,12,12,12,12,12,12,12 };
	char borderLeft[14] = { 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0 };
	char borderRight[14] = { 0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf };
	char borderTop[14] = { 0xff,0xff,0xff,0xff,0,0,0,0,0,0,0,0,0,0 };
	char borderBottom[14] = { 0,0,0,0,0,0,0,0,0,0,0xff, 0xff,0xff,0xff };

	char topLeft[14] = { 0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0 };
	char topRight[14] = { 0xff,0xff,0xff,0xff,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf };
	char bottomLeft[14] = { 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xff };
	char bottomRight[14] = { 0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xff,0xff,0xff,0xff };

	char highlightBottom[14] = { 0xff,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
	char highlightRight[14] = { 0x80,0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,0x80 };

	modifyChar(0xff, (char *)&dividerLine);	// DIVIDER LINE
	modifyChar(0xfe, (char *)&borderLeft);
	modifyChar(0xfd, (char *)&borderRight);
	modifyChar(0xfc, (char *)&borderTop);
	modifyChar(0xfb, (char *)&borderBottom);

	modifyChar(0xfa, (char *)&topLeft);
	modifyChar(0xf9, (char *)&topRight);
	modifyChar(0xf8, (char *)&bottomLeft);
	modifyChar(0xf7, (char *)&bottomRight);

	modifyChar(0xf6, (char *)&highlightBottom);
	modifyChar(0xf5, (char *)&highlightRight);

}

void modifyChar(int c, char *data)
{
	unsigned char *chptr = &chardata[c * 16];
	for (int i = 0;i < 14;i++)
	{
		*chptr++ = *data++;
	}

}

void forceFullRedraw()
{

}
