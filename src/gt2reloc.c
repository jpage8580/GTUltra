//
// GOATTRACKER v2.73 - gt2reloc (commandline relocator/packer)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#define GOATTRK2_C

#ifdef __WIN32__
#include <windows.h>
#endif

#include "goattrk2.h"
#include "bme.h"

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
char loadedsongfilename[MAX_FILENAME];
char wavfilename[MAX_PATHNAME];
char songfilename[MAX_FILENAME];
char songfilter[MAX_FILENAME];
char wavfilter[MAX_FILENAME];
char songpath[MAX_PATHNAME];
char instrfilename[MAX_FILENAME];
char instrfilter[MAX_FILENAME];
char instrpath[MAX_PATHNAME];
char packedpath[MAX_PATHNAME];
char packedsongname[MAX_PATHNAME];
char *programname = "$VER: GTUltra";

char textbuffer[MAX_PATHNAME];
char debugTextbuffer[MAX_PATHNAME];

int useOriginalGTFunctionKeys = 0;
extern GTOBJECT gtObject;
//GTOBJECT gtObject;
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
	SDL_Log("[Log] %s", message);
}

void usage(void)
{
	SDL_Log("Usage: GTULTRA RELOC <songname> <outfile> [options]\n");
	SDL_Log("Options:\n");
	SDL_Log("-Axx Set ADSR parameter for hardrestart in hex. DEFAULT=0F00\n");
	SDL_Log("-Bx  enable/disable buffered SID writes. DEFAULT=disabled\n");
	SDL_Log("-Cx  enable/disable zeropage ghost registers. DEFAULT=disabled\n");
	SDL_Log("-Dx  enable/disable sound effect support. DEFAULT=disabled\n");
	SDL_Log("-Ex  enable/disable volume change support. DEFAULT=disabled\n");
	SDL_Log("-Fxx Set custom SID clock cycles per second (0 = use PAL/NTSC default)\n");
	SDL_Log("-Gxx Set pitch of A-4 in Hz (0 = use default frequencytable, close to 440Hz)\n");
	SDL_Log("-Hx  enable/disable storing of author info. DEFAULT=disabled\n");
	SDL_Log("-Ix  enable/disable optimizations. DEFAULT=enabled\n");
	SDL_Log("-Jx  enable/disable full buffering. DEFAULT=disabled\n");
	SDL_Log("-Lxx SID memory location in hex for 2 SID Chips (2nd, 1st..). DEFAULT=D420D400\n");
	SDL_Log("-N   Use NTSC timing\n");
	SDL_Log("-Oxx Set pulseoptimization/skipping (0 = off, 1 = on) DEFAULT=on\n");
	SDL_Log("-P   Use PAL timing (DEFAULT)\n");
	SDL_Log("-Rxx Set realtime-effect optimization/skipping (0 = off, 1 = on) DEFAULT=on\n");
	SDL_Log("-Sxx Set speed multiplier (0 for 25Hz, 1 for 1x, 2 for 2x etc.) DEFAULT=1\n");
	SDL_Log("-Tx  Enable/disable SidTracker64 mode (0 = off, 1 = on) DEFAULT=off\n");
	SDL_Log("-Ux  Set SID channel count (3,6,9,12) DEFAULT=3\n");
	SDL_Log("-Vxx Set finevibrato conversion (0 = off, 1 = on) DEFAULT=on\n");
	SDL_Log("-Wxx player memory location highbyte in hex. DEFAULT=1000\n");
	SDL_Log("-Zxx zeropage memory location in hex. DEFAULT=FC\n");
	SDL_Log("-?   Show options\n");
}

int main(int argc, char **argv)
{
	int c;

#ifdef __WIN32__
	/*
	  SDL_Init() reroutes stdout and stderr, either to stdout.txt and stderr.txt
	  or to nirwana. simply reopening these handles does, other than suggested on
	  some web pages, not work reliably - opening new files on CON using different
	  handles however does.
	*/
	STDOUT = fopen("CON", "w");
	STDERR = fopen("CON", "w");

#endif

	//SDL_LogSetOutputFunction(&Log, NULL);


	programname += sizeof "$VER:";
	// Open datafile
	io_openlinkeddatafile(datafile);


	// Reset channels/song
	gtObject.songinit = PLAY_STOPPED;
	initchannels(&gtObject);
	clearsong(1, 1, 1, 1, 1, &gtObject);

	// get input- and output file names
	if (argc >= 3) {
		strcpy(songfilename, argv[1]);
		strcpy(packedsongname, argv[2]);
	}
	else {
		usage();
#ifdef __WIN32__
		// ENTER key down
		keybd_event(VK_RETURN, 0x9C, 0, 0);
		// ENTER key up
		keybd_event(VK_RETURN, 0x9C, KEYEVENTF_KEYUP, 0);
#endif
		exit(-1);

	}


	// Load song
	if (strlen(songfilename)) {
		loadsong(&gtObject, 1);
	}
	else {

		fprintf(STDERR, "error: no song filename given.\n");
		exit(-1);
	}


	c = strlen(packedsongname);
	if (strlen(packedsongname) <= 0) {
		fprintf(STDERR, "error: no output filename given.\n");
		exit(-1);
	}


	editorInfo.maxSIDChannels = 3;	// set as default

	// determine output format from file extension of the output filename
	c--;
	while ((c > 0) && (packedsongname[c] != '.')) c--;
	if (packedsongname[c] == '.') c++;

	if (!strcmp(&packedsongname[c], "sid")) {
		fileformat = FORMAT_SID;
	}
	else if (!strcmp(&packedsongname[c], "prg")) {
		fileformat = FORMAT_PRG;
	}
	else if (!strcmp(&packedsongname[c], "bin")) {
		fileformat = FORMAT_BIN;
	}
	else {
		fileformat = FORMAT_PRG;
	}

	SDL_Log("%s Packer/Relocator\n", programname);
	SDL_Log("song file:       %s\n", loadedsongfilename);
	SDL_Log("output file:     %s\n", packedsongname);
	SDL_Log("output format:   ");
	if (fileformat == FORMAT_SID) {
		SDL_Log("sid\n");
	}
	else if (fileformat == FORMAT_BIN) {
		SDL_Log("bin\n");
	}
	else {
		SDL_Log("prg\n");
	}

	// Scan command line
	for (c = 3; c < argc; c++)
	{
#ifdef __WIN32__
		if ((argv[c][0] == '-') || (argv[c][0] == '/'))
#else
		if (argv[c][0] == '-')
#endif
		{
			switch (toupper(argv[c][1]))
			{
			case '?':
				return 0;

			case 'A':
				sscanf(&argv[c][2], "%x", &adparam);
				break;

			case 'G':
				sscanf(&argv[c][2], "%f", &basepitch);
				break;

			case 'L':
				sscanf(&argv[c][2], "%x", &sidaddress);
				break;

			case 'O':
				sscanf(&argv[c][2], "%u", &optimizepulse);
				break;

			case 'R':
				sscanf(&argv[c][2], "%u", &optimizerealtime);
				break;

			case 'V':
				sscanf(&argv[c][2], "%u", &finevibrato);
				break;

			case 'S':
				sscanf(&argv[c][2], "%u", &multiplier);
				break;

			case 'T':
				sscanf(&argv[c][2], "%x", &SIDTracker64ForIPadIsAmazing);
				break;

			case 'U':
				sscanf(&argv[c][2], "%x", &editorInfo.maxSIDChannels);
				break;

				// NTSC timing
			case 'N':
				ntsc = 1;
				customclockrate = 0;
				break;
				// PAL timing
			case 'P':
				ntsc = 0;
				customclockrate = 0;
				break;
				// custom clock rate
			case 'F':
				sscanf(&argv[c][2], "%u", &customclockrate);
				break;

				// player options (first menu)
				// 0: Buffered SID-writes
			case 'B':
				if (argv[c][2] == '1') {
					playerversion |= PLAYER_BUFFERED;
				}
				else {
					playerversion &= ~PLAYER_BUFFERED;
				}
				break;
				// 1: Sound effect support
			case 'D':
				if (argv[c][2] == '1') {
					playerversion |= PLAYER_SOUNDEFFECTS;
				}
				else {
					playerversion &= ~PLAYER_SOUNDEFFECTS;
				}
				break;
				// 2: Volume change support
			case 'E':
				if (argv[c][2] == '1') {
					playerversion |= PLAYER_VOLUME;
				}
				else {
					playerversion &= ~PLAYER_VOLUME;
				}
				break;
				// 3: Store author-info
			case 'H':
				if (argv[c][2] == '1') {
					playerversion |= PLAYER_AUTHORINFO;
				}
				else {
					playerversion &= ~PLAYER_AUTHORINFO;
				}
				break;
				// 4: Use zeropage ghostregs
			case 'C':
				if (argv[c][2] == '1') {
					playerversion |= PLAYER_ZPGHOSTREGS;
				}
				else {
					playerversion &= ~PLAYER_ZPGHOSTREGS;
				}
				break;
				// 5: Disable optimization
			case 'I':
				if (argv[c][2] == '1') {
					playerversion &= ~PLAYER_NOOPTIMIZATION;
				}
				else {
					playerversion |= PLAYER_NOOPTIMIZATION;
				}
				// 6: Full buffering
			case 'J':
				if (argv[c][2] == '1') {
					playerversion &= ~PLAYER_FULLBUFFERED;
				}
				else {
					playerversion |= PLAYER_FULLBUFFERED;
				}
				break;

				// start address (second menu)
			case 'W':
				sscanf(&argv[c][2], "%x", &playeradr);
				playeradr <<= 8;
				break;

				// zeropage address (third menu)
			case 'Z':
				sscanf(&argv[c][2], "%x", &zeropageadr);
				break;
			}
		}
		else
		{
			fprintf(STDERR, "error: unknown option\n");
			usage();
			exit(-1);
		}
	}

	// Validate parameters
	sidmodel &= 1;
	adparam &= 0xffff;
	zeropageadr &= 0xff;
	playeradr &= 0xff00;
	sidaddress &= 0xffff;

	if (multiplier > 16) multiplier = 16;
	if ((finevibrato == 1) && (multiplier < 2)) usefinevib = 1;
	if (finevibrato > 1) usefinevib = 1;
	if (optimizepulse > 1) optimizepulse = 1;
	if (optimizerealtime > 1) optimizerealtime = 1;
	if (customclockrate < 100) customclockrate = 0;

	// Calculate frequencytable if necessary
	if (basepitch < 0.0f)
		basepitch = 0.0f;
	if (basepitch > 0.0f)
		calculatefreqtable();

	// perform relocation
	relocator(&gtObject, 1,0);

#ifdef __WIN32__
	// ENTER key down
	keybd_event(VK_RETURN, 0x9C, 0, 0);
	// ENTER key up
	keybd_event(VK_RETURN, 0x9C, KEYEVENTF_KEYUP, 0);
#endif
	// Exit
	exit(0);
	return 0;
}

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

	//	printf("---- SubSong %x ----\n", songNumber);

	int allDone;
	do {
		playroutine(gte);

		// Create arrays that are used to remap exported SID patterns in playing order
		for (int i = 0;i < editorInfo.maxSIDChannels;i++)
		{
			int pat = gte->chn[i].pattnum;
			if (patternOrderArray[pat] == -1)
			{
				//		printf("Pattern %x\n", pat);
				patternOrderList[patternRemapOrderIndex] = pat;	// contains list of patterns in order of playing
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


#define GT2RELOC

#include "greloc.c"
