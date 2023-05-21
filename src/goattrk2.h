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
#include "gfkeys.h"

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

#define MAX_PALETTE_ENTRIES 0x100-FIRST_UI_COLOR-1 //	255 - 32 for first valid palette (FIRST_UI_COLOR). 255 = WHITE for cursor
#define MAX_PALETTE_LOAD_ENTRIES 64
#define MAX_PALETTE_PRESETS 16


#ifndef GOATTRK2_C

extern int SIDTracker64ForIPadIsAmazing;
extern int autoNextPattern;
extern char appFileName[MAX_PATHNAME];
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
//extern int editorInfo.eacolumn;
extern int eamode;
extern unsigned keypreset;
extern unsigned playerversion;
extern int fileformat;
extern int zeropageadr;
extern int playeradr;
extern int debugEnabled;
//extern unsigned editorInfo.sidmodel;
//extern unsigned editorInfo.multiplier;
//extern unsigned editorInfo.adparam;
//extern unsigned editorInfo.ntsc;
extern unsigned patterndispmode;
extern unsigned sidaddress;
//extern unsigned finevibrato;
//extern unsigned editorInfo.optimizepulse;
//extern unsigned editorInfo.optimizerealtime;
//extern unsigned editorInfo.usefinevib;
extern unsigned b;
extern unsigned mr;
extern unsigned writer;
extern unsigned hardsid;
extern unsigned catweasel;
extern unsigned interpolate;
extern unsigned hardsidbufinteractive;
extern unsigned hardsidbufplayback;
extern unsigned monomode;
extern unsigned stereoMode;
extern float basepitch;
extern char configbuf[MAX_PATHNAME];
extern char loadedsongfilename[MAX_PATHNAME];
extern char wavfilename[MAX_PATHNAME];
extern char songfilename[MAX_PATHNAME];
extern char songfilter[MAX_FILENAME];
extern char wavfilter[MAX_FILENAME];
extern char songpath[MAX_PATHNAME];
extern char instrfilename[MAX_FILENAME];
extern char instrfilter[MAX_FILENAME];
extern char instrpath[MAX_PATHNAME];
extern char packedpath[MAX_PATHNAME];
extern char *programname;
extern char *notename[];
extern char *notenameTableView[];
extern char textbuffer[MAX_PATHNAME];
extern char debugTextbuffer[MAX_PATHNAME];
extern unsigned char hexkeytbl[16];
extern unsigned char datafile[];
extern char charsetFilename[MAX_PATHNAME];
extern int jdebug[16];
extern char palettefilter[MAX_FILENAME];
extern char palettepath[MAX_FILENAME];
extern char paletteFileName[MAX_FILENAME];
extern char backupFolderName[MAX_PATHNAME];
extern char backupSngFilename[MAX_PATHNAME];
extern char fkeysFilename[MAX_PATHNAME];

extern int patternOrderArray[256];
extern int patternOrderList[256];
extern int patternRemapOrderIndex;

extern char sourceBackupFolderName[MAX_FILENAME];
extern char destBackupFolderName[MAX_FILENAME];

extern int debugTicks;


extern int forceSave3ChannelSng;
extern int normalizeWAV;

//unsigned short paletteUIDisplay[MAX_PALETTE_ENTRIES];
extern unsigned char paletteLoadRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_LOAD_ENTRIES];

extern float masterVolume;
extern unsigned int lmanMode;
extern unsigned int editPaletteMode;
extern unsigned char paletteRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_ENTRIES];
extern int currentPalettePreset;
extern short tableBackgroundColors[MAX_TABLES][MAX_TABLELEN];
extern unsigned char paletteR[256];
extern unsigned char paletteG[256];
extern unsigned char paletteB[256];
//extern int editorInfo.maxSIDChannels;
extern char infoTextBuffer[256];

extern int SID_StereoPanPositions[4][4];
//extern int SID2_StereoPanPositions[];
//extern int SID3_StereoPanPositions[];
//extern int SID4_StereoPanPositions[];
extern char editPan;

extern int patternRemapOrderIndex;

extern char transportPolySIDEnabled[4];
extern char transportLoopPattern;
extern char transportLoopPatternSelectArea;
extern char transportRecord;
extern char transportPlay;
extern char transportFollowPlay;
extern char transportShowKeyboard;

extern unsigned int enablekeyrepeat;

extern char paletteChanged;
extern WAVEFORM_INFO waveformDisplayInfo;

extern int selectedMIDIPort;
extern unsigned int enableAntiAlias;

extern int useOriginalGTFunctionKeys;

extern float detuneCent;
extern int displayingPanel;
extern int displayStopped;

extern int useRepeatsWhenCompressing;

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
int load(GTOBJECT *gt, char *dragDropFileName);
void save(GTOBJECT *gt, int exportWAVFlag);
void quit(GTOBJECT *gt);
void clear(GTOBJECT *gt);
int prevmultiplier(void);
int nextmultiplier(void);
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
void playUntilEnd(int songNumber);
void playUntilEnd2(int songNumber);
void initRemapArrays();

int mouseTransportBar(GTOBJECT *gt);
int checkMouseRange(int x, int y, int w, int h);
void handleSIDChannelCountChange(GTOBJECT *gt);
void nextSongPos(GTOBJECT *gt);
void previousSongPos(GTOBJECT *gt, int songDffset);
void setSongToBeginning(GTOBJECT *gt);
void playFromCurrentPosition(GTOBJECT *gt, int currentPos);
int mouseTrackModify(int editorWindow);
void mouseTrack();
void ModifyTrackGetOriginalValue();
int checkForMouseInTable(int c,int OX,int OY);
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
void checkForMouseInOrderList(GTOBJECT *gt, int maxCh);
void checkForMouseInExtendedOrderList(GTOBJECT *gt, int maxCh);
int checkMouseInWaveformInfo();
int HzToSIDFreq(float hz);
float noteToHz(int note);
float centToHz(int cent);
void detunePitchTable();
void swapPalettes(int p1, int p2);
void handlePressRewind(int doubleClick, GTOBJECT *gt);
void createFilename(char *filePath, char *newfileName, char *filename);
void backupPatternDisplayInfo(GTOBJECT *gt);
void restorePatternDisplayInfo(GTOBJECT *gt);
void reInitSID();
void validateStereoMode();
void editSIDPan(GTOBJECT *gt);
void convertInsToPans(int sidChips);
void convertPansToInts(int sidChips);
void saveBackupSong();
int createBackupFolder();
int copyBackupFile(char *sourceName, char *destName);
int replacechar(char *str, char orig, char rep);
void handleLoad(GTOBJECT *gt, char *dragdropfile);
void stopScreenDisplay();
void restartScreenDisplay();
void ExportAsPCM(int songNumber, int doNormalize, GTOBJECT *gt);
void playUntilEnd(int songNumber);

#endif
