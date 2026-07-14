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

// Keep our own main() as the program entry on non-Windows. Without this, SDL_main.h
// renames main -> SDL_main and SDL runs a Cocoa app bootstrap first; that bootstrap needs
// an Aqua (GUI login) session, so from a no-GUI shell (mac CI, ssh w/o console) it blocks on
// an NSAlert and hangs. gt2reloc never calls SDL_Init, so no SDL_SetMainReady is needed.
// Windows keeps the SDL main shim (it links -lSDL2main / uses WinMain).
#ifndef __WIN32__
#define SDL_MAIN_HANDLED
#endif

#include "goattrk2.h"
#include "bme.h"
#include "version.h"

// Shared headless-CLI globals + editor stubs (textually included; see the file header).
#include "cli_common.c"

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

	SDL_LogSetOutputFunction(&Log, NULL);


	programname += sizeof "$VER:";
	// Open datafile
	io_openlinkeddatafile(datafile);


	// Reset channels/song
	gtObject.songinit = PLAY_STOPPED;
	initchannels(&gtObject);
	clearsong(1, 1, 1, 1, 1, &gtObject);

	// gtEditorObject drives the relocation-time playback (playUntilEnd2) but, unlike
	// gtObject, its sidreg[] pointers are never wired via initSID (mirrors gt2stereo.c).
	// Mark it silent so playroutine skips SID register writes; otherwise it dereferences
	// NULL sidreg[i] and segfaults (gplay.c:435).
	gtEditorObject.noSIDWrites = 1;

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

#define GT2RELOC

#include "greloc.c"
