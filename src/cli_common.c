//
// GTUltra headless CLI shell: shared globals + editor stubs for the non-interactive
// command-line tools (gt2reloc, sng2wav, ...).
//
// This file is TEXTUALLY #included (like greloc.c), not compiled on its own. The including
// translation unit MUST, before including this file:
//   #define GOATTRK2_C            // make this TU the owner of the goattrk2.h globals
//   #include "goattrk2.h"
//   #include "bme.h"
//   #include "version.h"
// and (on non-Windows) #define SDL_MAIN_HANDLED before including bme.h so SDL does not rename
// main() into its Cocoa app bootstrap (which hangs from a no-GUI shell; see gt2reloc.c).
//
// The interactive editor (gt2stereo.c / gdisplay.c) provides real implementations of the
// functions stubbed here; the CLI tools link the same object graph (gsong/gplay/gsound/gsid/
// gdisplay/...) but not gt2stereo.o, so these definitions satisfy the linker. Keep this list
// in sync with the globals/functions gt2stereo.c owns; a mismatch fails loudly at link time.
//
// This program is free software; you can redistribute it and/or modify it under the terms of
// the GNU General Public License, version 2 or (at your option) any later version.
//

// ---- globals owned by the CLI shell (mirror gt2stereo.c definitions) ----
int songExportSuccessFlag = 0;
int songExported = 0;
int menu = 0;
int editmode = EDIT_PATTERN;
int recordmode = 1;
int followplay = 0;
int hexnybble = -1;
int stepsize = 4;
int autoadvance = 0;
int defaultpatternlength = 64;
int cursorflash = 0;
int cursorcolortable[] = { 1,2,7,2 };
int exitprogram = 0;
int eacolumn = 0;
int eamode = 0;
int displayingPanel = 0;
int displayStopped = 0;
int debugEnabled = 0;
unsigned keypreset = KEY_TRACKER;
unsigned playerversion = 0;
int fileformat = FORMAT_PRG;
int zeropageadr = 0xfc;
int playeradr = 0x1000;
unsigned sidmodel = 0;
unsigned multiplier = 1;
unsigned adparam = 0x0f00;
unsigned ntsc = 0;
unsigned patterndispmode = 0;
unsigned sidaddress = 0xd420d400;
unsigned finevibrato = 1;
unsigned optimizepulse = 1;
unsigned optimizerealtime = 1;
unsigned customclockrate = 0;
unsigned usefinevib = 0;
unsigned b = DEFAULTBUF;
unsigned mr = DEFAULTMIXRATE;
unsigned writer = 0;
unsigned hardsid = 0;
unsigned catweasel = 0;
unsigned interpolate = 0;
unsigned residdelay = 0;
unsigned hardsidbufinteractive = 20;
unsigned hardsidbufplayback = 400;
float basepitch = 0.0f;
int SIDTracker64ForIPadIsAmazing = 0;
float masterVolume = 1.0f;
float detuneCent = 0;
unsigned monomode = 0;
char editPan = 0;

int patternOrderArray[256];
int patternOrderList[256];
int patternRemapOrderIndex;

char configbuf[MAX_PATHNAME];
char loadedsongfilename[MAX_PATHNAME];
char wavfilename[MAX_PATHNAME];
char songfilename[MAX_PATHNAME];	// MAX_PATHNAME to match goattrk2.h extern + gt2stereo.c
									// (was MAX_FILENAME: too small for long paths, and
									// gfile.c memsets sizeof=MAX_PATHNAME into it -> overflow)
char songfilter[MAX_FILENAME];
char wavfilter[MAX_FILENAME];
char songpath[MAX_PATHNAME];
char instrfilename[MAX_FILENAME];
char instrfilter[MAX_FILENAME];
char instrpath[MAX_PATHNAME];
char packedpath[MAX_PATHNAME];
char packedsongname[MAX_PATHNAME];
char *programname = GTULTRA_VERSION_STRING;

char textbuffer[MAX_PATHNAME];
char debugTextbuffer[MAX_PATHNAME];

int useOriginalGTFunctionKeys = 0;
extern GTOBJECT gtObject;
int useRepeatsWhenCompressing = 1;
char infoTextBuffer[256];
int midiEnabled = 0;
int forceSave3ChannelSng = 0;
int normalizeWAV = 0;
int jdebug[16];
char transportLoopPattern = 0;
char transportPolySIDEnabled[4];	// 0 = OFF 1 = ON (all OFF = mono)
int currentPalettePreset = 0;
WAVEFORM_INFO waveformDisplayInfo;
int paletteChanged = 0;
unsigned int editPaletteMode = 0;
int autoNextPattern = 0;
int sidAddr1 = 0xd400;
int sidAddr2 = 0xd420;
int sidAddr3 = 0xd440;
int sidAddr4 = 0xd460;

char fkeysFilename[MAX_PATHNAME];
unsigned char paletteRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_ENTRIES];
char paletteFileName[MAX_FILENAME] = { "palette_" };
char palettefilter[MAX_FILENAME];
char palettepath[MAX_FILENAME];
int selectedMIDIPort = 0;
unsigned char paletteLoadRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_LOAD_ENTRIES];

unsigned short tableBackgroundColors[MAX_TABLES][MAX_TABLELEN];
unsigned char paletteR[256];
unsigned char paletteG[256];
unsigned char paletteB[256];
extern unsigned char datafile[];
int debugTicks;	// used to measure CPU use when looking to improve performance
char appFileName[MAX_PATHNAME];
unsigned int enablekeyrepeat = 0;
unsigned int enableAntiAlias = 1;

int SID_StereoPanPositions[4][4] = {
									{ 7,0,0,0 },
									{ 0,14,0,0 },
									{0, 14, 7, 0},
									{0, 14, 0, 14}
};

char transportLoopPatternSelectArea = 0;
char transportShowKeyboard = 0;

int sidPanInts[4] = { 0x0007, 0x00e0, 0x07e0, 0xe0e0 };
unsigned stereoMode = 1;
char charsetFilename[MAX_PATHNAME];

#ifdef __WIN32__
FILE *STDOUT, *STDERR;
#else
#define STDOUT stdout
#define STDERR stderr
#endif


void Log(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
	// Uniform cross-platform log line. Without a custom output function, macOS SDL_Log
	// routes to NSLog and prepends a "timestamp process[pid:tid]" prefix, differing from
	// the plain "INFO: <msg>" that Linux/Windows produce. (Do NOT call SDL_Log here - that
	// recurses through this callback.)
	// On Windows STDERR is a global reopened via fopen("CON") which can be NULL in a
	// headless/CI shell; fall back to the real stderr so we never fprintf(NULL) -> crash.
	FILE *out = STDERR ? STDERR : stderr;
	size_t n = strlen(message);
	if (n && message[n - 1] == '\n') n--;   // normalize a trailing newline
	fprintf(out, "INFO: %.*s\n", (int)n, message);
	fflush(out);
}

// ---- editor-function stubs (real bodies live in gt2stereo.c/gdisplay.c) ----

void waitkeymousenoupdate(void)
{
}

void waitkeynoupdate(void)
{

}

void getparam(FILE *handle, unsigned *value)
{
	char *configptr;

	for (;;)
	{
		if (feof(handle)) return;
		fgets(configbuf, MAX_PATHNAME, handle);
		if ((configbuf[0]) && (configbuf[0] != ';') && (configbuf[0] != ' ') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
	}

	configptr = configbuf;
	if (*configptr == '$')
	{
		*value = 0;
		configptr++;
		for (;;)
		{
			char c = tolower(*configptr++);
			int h = -1;

			if ((c >= 'a') && (c <= 'f')) h = c - 'a' + 10;
			if ((c >= '0') && (c <= '9')) h = c - '0';

			if (h >= 0)
			{
				*value *= 16;
				*value += h;
			}
			else break;
		}
	}
	else
	{
		*value = 0;
		for (;;)
		{
			char c = tolower(*configptr++);
			int d = -1;

			if ((c >= '0') && (c <= '9')) d = c - '0';

			if (d >= 0)
			{
				*value *= 10;
				*value += d;
			}
			else break;
		}
	}
}

void getfloatparam(FILE *handle, float *value)
{
	char *configptr;

	for (;;)
	{
		if (feof(handle)) return;
		fgets(configbuf, MAX_PATHNAME, handle);
		if ((configbuf[0]) && (configbuf[0] != ';') && (configbuf[0] != ' ') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
	}

	configptr = configbuf;
	*value = 0.0f;
	sscanf(configptr, "%f", value);
}

void calculatefreqtable()
{
	double basefreq = (double)basepitch * (16777216.0 / 985248.0) * pow(2.0, 0.25) / 32.0;
	int c;

	for (c = 0; c < 8 * 12; c++)
	{
		double note = c;
		double freq = basefreq * pow(2.0, note / 12.0);
		int intfreq = freq + 0.5;
		if (intfreq > 0xffff)
			intfreq = 0xffff;
		freqtbllo[c] = intfreq & 0xff;
		freqtblhi[c] = intfreq >> 8;
	}
}

void playFromCurrentPosition(GTOBJECT *gt, int currentPos)
{

}

void setGFXPaletteRGBFromPaletteRGB(int presetIndex, int paletteIndex)
{

}

void createFilename(char *filePath, char *newfileName, char *filename)
{
}
void converthex()
{

}

void setTableBackgroundColours(int currentInstrument)
{

}

void backupPatternDisplayInfo(GTOBJECT *gt)
{

}
void restorePatternDisplayInfo(GTOBJECT *gt)
{

}

int isMatchingRGB(int presetIndex, int color)
{
	return 0;
}

void handleSIDChannelCountChange(GTOBJECT *gt)
{

}
void validateStereoMode()
{

}

void reInitSID()
{

}

void setSkin(int palettePreset)
{

}

void stopScreenDisplay()
{
}
void restartScreenDisplay()
{
}

void playUntilEnd(int songNumber)
{

}

// Pattern play-order remap, used by the packer (greloc.c). Real editor bodies live in
// gt2stereo.c; the CLI tools need these because they link/compile greloc.
void initRemapArrays()
{
	for (int i = 0;i < 256;i++)
	{
		patternOrderArray[i] = -1;
		patternOrderList[i] = -1;
	}
}

void playUntilEnd2(int songNumber)
{
	int sng = getActualSongNumber(songNumber, 0);	// editorInfo.esnum
	GTOBJECT *gte = &gtEditorObject;

	initsong(sng, PLAY_BEGINNING, gte);	// JP FEB
	gte->loopEnabledFlag = 0;

	int allDone;
	do {
		playroutine(gte);

		// Create arrays that are used to remap exported SID patterns in playing order
		for (int i = 0;i < editorInfo.maxSIDChannels;i++)
		{
			int pat = gte->chn[i].pattnum;
			if (patternOrderArray[pat] == -1)
			{
				patternOrderList[patternRemapOrderIndex] = pat;	// list of patterns in play order
				patternOrderArray[pat] = patternRemapOrderIndex;
				patternRemapOrderIndex++;
			}
		}
		if (gte->songinit == PLAY_STOPPED)	// Error in song data
		{
			break;
		}

		allDone = 1;
		for (int i = 0;i < editorInfo.maxSIDChannels;i++)
		{
			if (gte->chn[i].loopCount == 0)	// wait until all channels have looped (or song ends)
			{
				allDone = 0;	// hasn't looped
				break;
			}
		}
	} while (allDone == 0);
}
