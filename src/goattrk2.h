#ifndef GOATTRK2_H
#define GOATTRK2_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include "bme.h"


#include "gmidi.h"
#include "gmidiselect.h"

#include "gcommon.h"
#include "gconsole.h"
#include "gplay.h"
#include "gsound.h"
#include "gsid.h"
#include "gsong.h"

#include "gdisplay.h"
#include "greloc.h"
#include "gfile.h"
#include "gpattern.h"
#include "gorder.h"
#include "ginstr.h"
#include "gtable.h"
#include "ginfo.h"
#include "gundo.h"
#include "gchareditor.h"
#include "gtabledisplay.h"
#include "gpaletteeditor.h"

#define REMOVE_UNDO 0

#define EDIT_PATTERN 0
#define EDIT_ORDERLIST 1
#define EDIT_INSTRUMENT 2
#define EDIT_TABLES 3
#define EDIT_NAMES 4

//For EDITOR_INFO.editTableMode 
#define EDIT_TABLE_NONE 0
#define EDIT_TABLE_WAVE 1
#define EDIT_TABLE_PULSE 2
#define EDIT_TABLE_FILTER 3
#define EDIT_TABLE_SPEED 4

#define KEY_TRACKER 0
#define KEY_DMC 1
#define KEY_JANKO 2				   

#define VISIBLEPATTROWS 29	//31
#define VISIBLEORDERLIST 11	// 11
#define VISIBLETABLEROWS 14
#define VISIBLEFILES 24

#define PGUPDNREPEAT 8

#define MAX_PALETTE_ENTRIES 32
#define MAX_PALETTE_PRESETS 4

#ifndef GOATTRK2_C
extern int menu;
//extern int editmode;
extern int recordmode;
extern int followplay;
extern int hexnybble;
extern int stepsize;
extern int autoadvance;
extern int defaultpatternlength;
extern int cursorflash;
extern int cursorcolortable[];
extern int exitprogram;
extern int eacolumn;
extern int eamode;
extern unsigned keypreset;
extern unsigned playerversion;
extern int fileformat;
extern int zeropageadr;
extern int playeradr;
extern unsigned sidmodel;
extern unsigned multiplier;
extern unsigned adparam;
extern unsigned ntsc;
extern unsigned patterndispmode;
extern unsigned sidaddress;
extern unsigned finevibrato;
extern unsigned optimizepulse;
extern unsigned optimizerealtime;
extern unsigned usefinevib;
extern unsigned b;
extern unsigned mr;
extern unsigned writer;
extern unsigned hardsid;
extern unsigned catweasel;
extern unsigned interpolate;
extern unsigned hardsidbufinteractive;
extern unsigned hardsidbufplayback;
extern unsigned monomode;
extern float basepitch;
extern char configbuf[MAX_PATHNAME];
extern char loadedsongfilename[MAX_FILENAME];
extern char songfilename[MAX_FILENAME];
extern char songfilter[MAX_FILENAME];
extern char songpath[MAX_PATHNAME];
extern char instrfilename[MAX_FILENAME];
extern char instrfilter[MAX_FILENAME];
extern char instrpath[MAX_PATHNAME];
extern char packedpath[MAX_PATHNAME];
extern char *programname;
extern char *notename[];
extern char *notenameTableView[];
extern char textbuffer[MAX_PATHNAME];
extern unsigned char hexkeytbl[16];
extern unsigned char datafile[];
//unsigned short paletteUIDisplay[MAX_PALETTE_ENTRIES];

extern float masterVolume;
extern unsigned int lmanMode;
extern unsigned int editPaletteMode;
extern unsigned char paletteRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_ENTRIES];
extern int currentPalettePreset;
extern short tableBackgroundColors[MAX_TABLES][MAX_TABLELEN];
extern unsigned char paletteR[256];
extern unsigned char paletteG[256];
extern unsigned char paletteB[256];
extern int maxSIDChannels;
extern char infoTextBuffer[256];

extern char transportPolySIDEnabled[4];
extern char transportLoopPattern;
extern char transportRecord;
extern char transportPlay;
extern char transportFollowPlay;
extern char transportShowKeyboard;

extern unsigned int enablekeyrepeat;

extern char paletteChanged;
extern WAVEFORM_INFO waveformDisplayInfo;


extern int selectedMIDIPort;


extern float detuneCent;

#endif

void getparam(FILE *handle, unsigned *value);
void getfloatparam(FILE *handle, float *value);
void getstringparam(FILE *handle, char *value);
void waitkey(GTOBJECT *gt);
void waitkeymouse(GTOBJECT *gt);
void waitkeynoupdate(void);
void waitkeymousenoupdate(void);
void converthex(void);
void docommand(void);
void onlinehelp(int standalone, int context, GTOBJECT *gt);
void mousecommands(GTOBJECT *gt);
void generalcommands(GTOBJECT *gt);
void load(GTOBJECT *gt);
void save(GTOBJECT *gt);
void quit(GTOBJECT *gt);
void clear(GTOBJECT *gt);
void prevmultiplier(void);
void nextmultiplier(void);
void editadsr(GTOBJECT *gt);
void calculatefreqtable(void);
void setspecialnotenames(void);
void readscalatuningfile(void);
void handlePaletteDisplay(GTOBJECT *gt, int palettePreset);
void setPaletteRGB(int presetIndex, int paletteIndex, int r, int g, int b);
void copyPaletteToOrderList(int palettePreset);
void initPaletteDisplay();
void setGFXPaletteRGBFromPaletteRGB(int presetIndex, int paletteIndex);
void setSkin(int palettePreset);
int isMatchingRGB(int presetIndex, int color);
void setTableColour(int instrumentTablePtr, int t, int startTableOffset, int endTableOffset, int color);
void setTableBackgroundColours(int currentInstrument);
void highlightInstrument(int t, int instrumentTablePtr);
int quickSave();
void playUntilEnd();
int mouseTransportBar(GTOBJECT *gt);
int checkMouseRange(int x, int y, int w, int h);
void handleSIDChannelCountChange(GTOBJECT *gt);
void nextSongPos(GTOBJECT *gt);
void previousSongPos(GTOBJECT *gt, int songDffset);
void setSongToBeginning(GTOBJECT *gt);
void playFromCurrentPosition(GTOBJECT *gt);
int mouseTrackModify(int editorWindow);
void mouseTrack();
void ModifyTrackGetOriginalValue();
int checkForMouseInTable(int c);
int checkForMouseInDetailedWaveTable();
int checkForMouseInDetailedFilterTable();
int checkForMouseInDetailedPulseTable();
void detailedWaveTableChangeRelativeNote(int x, int y);
void detailedWaveTableChangeData(int x, int y);
void detailedWaveTableChangeCommand(int x, int y);
void detailedFilterTableChangeCommand(int x, int y);
void detailedFilterTableChangeSign(int x, int y);
void detailedFilterTableChangeFilterType(int x, int y);
void detailedPulseTableChangeSign(int x, int y);
void detailedPulseTableChangeCommand(int x, int y);
int checkMouseInWaveformInfo();
int HzToSIDFreq(float hz);
float noteToHz(int note);
float centToHz(int cent);
void detunePitchTable();
void swapPalettes(int p1, int p2);
void handlePressRewind(int doubleClick);

#endif
