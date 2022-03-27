#ifndef GCONSOLE_H
#define GCONSOLE_H

#define MAX_COLUMNS 100
#define MAX_ROWS 37
#define HOLDDELAY 24
#define DOUBLECLICKDELAY 15
#define UNDERLINE_MASK 0x8000
#define UNDERLINE_FOREGROUND_MASK 0x4000

int initscreen(void);
void closescreen(void);
void clearscreen(int backColor);
void fliptoscreen(void);
void printtext(int x, int y, int color, const char *text);
void printtextc(int y, int color, const char *text);
void printtextcp(int cp, int y, int color, const char *text);
void printblank(int x, int y, int length);
void printblankc(int x, int y, int color, int length);
void drawbox(int x, int y, int color, int sx, int sy);
void printbg(int x, int y, int color, int length);
void getkey(void);
void drawLine(int x, int y, int w, int h, int colr);

void printbyte(int x, int y, int color, char b);
void printbyterow(int x, int y, int color, char b, int length);
void printbytecol(int x, int y, int color, char b, int length);
void modifyChars();
int getColor(int fcolor, int bcolor);
void forceRedraw();

#ifndef GCONSOLE_C
extern int key, rawkey, shiftpressed, cursorflashdelay, ctrlpressed;
extern int bothShiftAndCtrlPressed;
extern int mouseb, prevmouseb;
extern int mouseheld;
extern int mousex, mousey;
extern int mousebDoubleClick;
extern unsigned bigwindow;
extern int keyDownCount;

extern unsigned char *chardata;

#endif

#endif
