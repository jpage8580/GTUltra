//
// GTUltra sng2wav - headless .sng -> .wav renderer (test/CI fixture generator)
//
// Loads a .sng, drives the player + SID engine offline (no editor, no SDL audio device,
// no GUI/Aqua session) and writes a stereo 16-bit WAV. Built for golden-reference capture
// and A/B comparison across SID engine backends (see notes/new-emu/).
//
// This program is free software; you can redistribute it and/or modify it under the terms of
// the GNU General Public License, version 2 or (at your option) any later version.
//

#define GOATTRK2_C

#ifdef __WIN32__
#include <windows.h>
#endif

// Keep our own main() as the entry on non-Windows (see gt2reloc.c). Unlike gt2reloc, this
// tool does bring up an SDL subsystem indirectly? No - sound_init_offline never calls
// SDL_OpenAudio, so no SDL_Init happens and no SDL_SetMainReady is needed.
#ifndef __WIN32__
#define SDL_MAIN_HANDLED
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include "goattrk2.h"
#include "bme.h"
#include "version.h"

// Shared headless-CLI globals + editor stubs (textually included; see the file header).
#include "cli_common.c"

// ---------------------------------------------------------------------------
// --scan: survey one or many .sng files (a file, or a directory tree)
// ---------------------------------------------------------------------------

// Column meanings (counts are of wavetable/filter-table steps = static instrument
// programming, not per-frame playback):
//   SIDs/ch  : SID chips / channels (exact when the 0x1f settings block is present)
//   wave     : wavetable steps that set a real SID waveform
//   sync/ring: of those, how many enable hard-sync (0x02) / ring-mod (0x04)
//   filt     : filter-table steps in use
//   model..RO: from the 0x1f settings block; blank when the block is absent
// Output is a box-drawing table sized to its content (two passes: fill, then render).

#define SCAN_MAX 4096
#define SCAN_COLS 12            // short columns; the file path is column 13, stored separately
static char g_scanpaths[SCAN_MAX][MAX_PATHNAME];
static char g_cells[SCAN_MAX][SCAN_COLS][20];
static char g_ok[SCAN_MAX];
static int g_scancount = 0;

static const char *g_scanhdr[SCAN_COLS + 1] = {
	"SIDs", "ch", "wave", "sync", "ring", "filt",
	"model", "HR", "spd", "st", "tim", "FV/PO/RO", "file"
};

static int ends_with_sng(const char *n)
{
	size_t l = strlen(n);
	return l > 4 && strcasecmp(n + l - 4, ".sng") == 0;
}

static void collect_sngs(const char *dir)
{
	DIR *d = opendir(dir);
	if (!d) return;
	struct dirent *e;
	while ((e = readdir(d)) != NULL)
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char full[MAX_PATHNAME];
		snprintf(full, sizeof full, "%s/%s", dir, e->d_name);
		struct stat st;
		if (stat(full, &st) != 0) continue;
		if (S_ISDIR(st.st_mode)) collect_sngs(full);
		else if (S_ISREG(st.st_mode) && ends_with_sng(e->d_name) && g_scancount < SCAN_MAX)
		{
			strncpy(g_scanpaths[g_scancount], full, MAX_PATHNAME - 1);
			g_scanpaths[g_scancount][MAX_PATHNAME - 1] = 0;
			g_scancount++;
		}
	}
	closedir(d);
}

static int cmp_path(const void *a, const void *b) { return strcmp((const char *)a, (const char *)b); }

// Load song i and fill its row cells. Settings columns are left blank when the .sng has no
// 0x1f settings block; a load failure leaves all short cells blank (flagged via g_ok).
static void scan_fill(int i)
{
	for (int c = 0; c < SCAN_COLS; c++) g_cells[i][c][0] = 0;

	strncpy(songfilename, g_scanpaths[i], MAX_PATHNAME - 1);
	songfilename[MAX_PATHNAME - 1] = 0;
	clearsong(1, 1, 1, 1, 1, &gtObject);
	editorInfo.maxSIDChannels = 0;
	editorInfo.adparam = 0xffffffff;   // sentinel: detect the 0x1f settings block
	stereoMode = 1;

	if (!loadsong(&gtObject, 1)) { g_ok[i] = 0; return; }
	g_ok[i] = 1;

	int wav = 0, sync = 0, ring = 0, filt = 0;
	for (int t = 0; t < MAX_TABLELEN; t++)
	{
		unsigned char v = ltable[WTBL][t];
		if (v > WAVELASTDELAY && v < WAVESILENT)
		{
			wav++;
			if (v & 0x02) sync++;
			if (v & 0x04) ring++;
		}
		if (ltable[FTBL][t] || rtable[FTBL][t]) filt++;
	}
	int chans = editorInfo.maxSIDChannels > 0 ? editorInfo.maxSIDChannels : 3;
	int sids = (chans + 2) / 3;

	snprintf(g_cells[i][0], 20, "%d", sids);
	snprintf(g_cells[i][1], 20, "%d", chans);
	snprintf(g_cells[i][2], 20, "%d", wav);
	snprintf(g_cells[i][3], 20, "%d", sync);
	snprintf(g_cells[i][4], 20, "%d", ring);
	snprintf(g_cells[i][5], 20, "%d", filt);
	if (editorInfo.adparam != 0xffffffff)   // 0x1f settings block present
	{
		snprintf(g_cells[i][6], 20, "%s", (editorInfo.sidmodel & 1) ? "8580" : "6581");
		snprintf(g_cells[i][7], 20, "%04X", editorInfo.adparam & 0xffff);
		snprintf(g_cells[i][8], 20, "x%u", editorInfo.multiplier);
		snprintf(g_cells[i][9], 20, "%u", stereoMode);
		snprintf(g_cells[i][10], 20, "%s", editorInfo.ntsc ? "NTSC" : "PAL");
		snprintf(g_cells[i][11], 20, "%u/%u/%u",
			editorInfo.usefinevib, editorInfo.optimizepulse, editorInfo.optimizerealtime);
	}
	// else: settings columns stay blank
}

// Print one horizontal box rule using the given corner/junction glyphs and column widths.
static void scan_rule(const char *left, const char *mid, const char *right, const int *w, int n)
{
	fputs(left, stdout);
	for (int c = 0; c < n; c++)
	{
		for (int k = 0; k < w[c] + 2; k++) fputs("─", stdout);   // horizontal
		fputs(c == n - 1 ? right : mid, stdout);
	}
	fputc('\n', stdout);
}

static void scan_render(void)
{
	const int n = SCAN_COLS + 1;   // +1 for the file column
	int w[SCAN_COLS + 1];
	for (int c = 0; c < n; c++) w[c] = (int)strlen(g_scanhdr[c]);

	char filecell[MAX_PATHNAME + 16];
	for (int i = 0; i < g_scancount; i++)
	{
		for (int c = 0; c < SCAN_COLS; c++)
		{
			int l = (int)strlen(g_cells[i][c]);
			if (l > w[c]) w[c] = l;
		}
		snprintf(filecell, sizeof filecell, "%s%s", g_scanpaths[i], g_ok[i] ? "" : "  [LOAD FAILED]");
		int fl = (int)strlen(filecell);
		if (fl > w[SCAN_COLS]) w[SCAN_COLS] = fl;
	}

	scan_rule("┌", "┬", "┐", w, n);            // top    ┌ ┬ ┐
	for (int c = 0; c < n; c++) printf("│ %-*s ", w[c], g_scanhdr[c]);
	printf("│\n");
	scan_rule("├", "┼", "┤", w, n);            // header ├ ┼ ┤
	for (int i = 0; i < g_scancount; i++)
	{
		for (int c = 0; c < SCAN_COLS; c++) printf("│ %-*s ", w[c], g_cells[i][c]);
		snprintf(filecell, sizeof filecell, "%s%s", g_scanpaths[i], g_ok[i] ? "" : "  [LOAD FAILED]");
		printf("│ %-*s │\n", w[SCAN_COLS], filecell);
		if (i < g_scancount - 1) scan_rule("├", "┼", "┤", w, n);
	}
	scan_rule("└", "┴", "┘", w, n);            // bottom └ ┴ ┘
}

void usage(void)
{
	// Plain stderr (no SDL_Log "INFO:" prefix) - this is help/diagnostic output.
	fprintf(stderr,
		"Usage:\n"
		"  sng2wav <song.sng> <out.wav> [options]   render to WAV (no window/audio device)\n"
		"  sng2wav --scan <song.sng|dir>            survey tune(s): effects + stored settings\n"
		"\n"
		"Options:\n"
		"  --seconds S    render length in seconds (default 10; ignored if --frames)\n"
		"  --frames F     render exactly F player frames (deterministic length)\n"
		"  --subsong N    subsong number (default 0)\n"
		"  --model M      SID model 6581|8580 (default: from .sng)\n"
		"  --stereo K     0=mono 1=SID-stereo 2=TrueStereo (default: from .sng)\n"
		"  --backend B    legacy-resid | legacy-residfp (default legacy-residfp)\n"
		"  --interp I     0=fast 1=interpolate/resample (default 1)\n"
		"  --interpolate V  packed .cfg value 0..3 (0=resid off,1=resid on,2=residfp,3=residfp on)\n"
		"  --rate HZ      sample rate (default 44100)\n"
		"  --channels N   force SID channel count (default: auto from .sng)\n"
		"  --ntsc / --pal timing override (default: from .sng)\n"
		"  --normalize    normalize output level\n"
		"  --seed K       srand seed for reproducible dithering/noise (default 0)\n");
}

int main(int argc, char **argv)
{
	// Options + defaults.
	const char *inPath = NULL, *outPath = NULL;
	double seconds = 10.0;
	long frames = -1;               // -1 => derive from seconds
	int subsong = 0;
	// -1 sentinels mean "use the value stored in the .sng" (loaded by loadsong); a CLI flag
	// overrides. The .sng header (block 0x1f) carries sidmodel/ntsc/multiplier/stereoMode.
	int model = -1;                 // -1 => song; else 0=6581 1=8580
	int stereo = -1;                // -1 => song; else 0=mono 1=SID-stereo 2=TrueStereo
	int ntscOpt = -1;               // -1 => song; 0=PAL 1=NTSC
	int backend = 1;                // 0 = legacy-resid, 1 = legacy-residfp
	int interp = 1;
	unsigned rate = 44100;
	int forceChannels = 0;          // 0 => auto from .sng
	int normalize = 0;
	unsigned seed = 0;
	int scanMode = 0;               // --scan: report sync/ring-mod/filter use, don't render

#ifdef __WIN32__
	STDOUT = fopen("CON", "w");
	STDERR = fopen("CON", "w");
#endif
	SDL_LogSetOutputFunction(&Log, NULL);
	programname += sizeof "$VER:";

	if (argc < 2)
	{
		usage();
		return 1;
	}

	// Flags may appear anywhere; the first non-flag arg is the input (.sng or a directory
	// for --scan), the second is the output .wav.
	for (int i = 1; i < argc; i++)
	{
		const char *a = argv[i];
		if (a[0] == '-')
		{
			if (!strcmp(a, "--seconds") && i + 1 < argc) seconds = atof(argv[++i]);
			else if (!strcmp(a, "--frames") && i + 1 < argc) frames = atol(argv[++i]);
			else if (!strcmp(a, "--subsong") && i + 1 < argc) subsong = atoi(argv[++i]);
			else if (!strcmp(a, "--model") && i + 1 < argc) model = (atoi(argv[++i]) >= 8580) ? 1 : 0;
			else if (!strcmp(a, "--stereo") && i + 1 < argc) stereo = atoi(argv[++i]);
			else if (!strcmp(a, "--backend") && i + 1 < argc) {
				const char *b2 = argv[++i];
				if (!strcmp(b2, "legacy-resid")) backend = 0;
				else if (!strcmp(b2, "legacy-residfp")) backend = 1;
				else { fprintf(stderr, "error: unknown backend '%s'\n", b2); return 1; }
			}
			else if (!strcmp(a, "--interp") && i + 1 < argc) interp = atoi(argv[++i]) ? 1 : 0;
			else if (!strcmp(a, "--interpolate") && i + 1 < argc) {
				// Packed reSID config value (the .cfg "interpolate" key): bit1 = engine
				// (0=reSID,1=reSID-FP/"distortion"), bit0 = interpolation on/off.
				int v = atoi(argv[++i]);
				backend = (v >> 1) & 1;
				interp = v & 1;
			}
			else if (!strcmp(a, "--rate") && i + 1 < argc) rate = (unsigned)atoi(argv[++i]);
			else if (!strcmp(a, "--channels") && i + 1 < argc) forceChannels = atoi(argv[++i]);
			else if (!strcmp(a, "--ntsc")) ntscOpt = 1;
			else if (!strcmp(a, "--pal")) ntscOpt = 0;
			else if (!strcmp(a, "--normalize")) normalize = 1;
			else if (!strcmp(a, "--seed") && i + 1 < argc) seed = (unsigned)strtoul(argv[++i], NULL, 0);
			else if (!strcmp(a, "--scan")) scanMode = 1;
			else { fprintf(stderr, "error: unknown/incomplete option '%s'\n", a); usage(); return 1; }
		}
		else if (!inPath) inPath = a;
		else if (!outPath) outPath = a;
		else { fprintf(stderr, "error: unexpected argument '%s'\n", a); usage(); return 1; }
	}

	if (!inPath)
	{
		usage();
		return 1;
	}
	if (!scanMode && !outPath)
	{
		fprintf(stderr, "error: no output .wav given (or use --scan)\n");
		usage();
		return 1;
	}

	srand(seed);   // reproducible: filter noise / pan spread / badline all use rand()

	// Song model setup (mirrors gt2reloc's headless load path).
	io_openlinkeddatafile(datafile);
	gtObject.songinit = PLAY_STOPPED;
	initchannels(&gtObject);
	gtEditorObject.noSIDWrites = 1;

	if (scanMode)
	{
		// Collect a single .sng or a whole directory tree, then render one box table.
		struct stat st;
		int isdir = (stat(inPath, &st) == 0 && S_ISDIR(st.st_mode));
		if (isdir)
		{
			collect_sngs(inPath);
			qsort(g_scanpaths, g_scancount, MAX_PATHNAME, cmp_path);
		}
		else
		{
			strncpy(g_scanpaths[0], inPath, MAX_PATHNAME - 1);
			g_scanpaths[0][MAX_PATHNAME - 1] = 0;
			g_scancount = 1;
		}
		if (g_scancount == 0)
		{
			printf("(no .sng files under %s)\n", inPath);
			return 0;
		}
		for (int i = 0; i < g_scancount; i++) scan_fill(i);
		scan_render();
		return 0;
	}

	// ---- render path (single .sng -> .wav) ----
	clearsong(1, 1, 1, 1, 1, &gtObject);
	strncpy(songfilename, inPath, MAX_PATHNAME - 1);
	songfilename[MAX_PATHNAME - 1] = 0;

	editorInfo.maxSIDChannels = 0;   // 0 => loadsong fills it from the song header
	if (!loadsong(&gtObject, 1))
	{
		fprintf(stderr, "error: could not load song '%s'\n", inPath);
		return 1;
	}
	if (forceChannels) editorInfo.maxSIDChannels = forceChannels;
	if (editorInfo.maxSIDChannels <= 0) editorInfo.maxSIDChannels = 3;

	// Playback / render parameters. Defaults come from the loaded song; CLI flags override.
	if (model >= 0) editorInfo.sidmodel = model;
	editorInfo.sidmodel &= 1;
	if (ntscOpt >= 0) editorInfo.ntsc = ntscOpt;
	if (stereo >= 0) stereoMode = stereo;
	if (editorInfo.adparam == 0) editorInfo.adparam = adparam;   // hardrestart fallback 0x0f00
	mr = rate;
	interpolate = (unsigned)((backend << 1) | interp);

	if (!sound_init_offline(rate, editorInfo.sidmodel, editorInfo.ntsc, editorInfo.multiplier, interpolate, customclockrate))
	{
		fprintf(stderr, "error: sound_init_offline failed\n");
		return 1;
	}
	initSID(&gtObject);              // wire gtObject.sidreg[] -> engine register arrays
	gtObject.noSIDWrites = 0;        // we DO want register writes for rendering

	int sng = getActualSongNumber(subsong, 0);
	initsong(sng, PLAY_BEGINNING, &gtObject);
	gtObject.loopEnabledFlag = 0;
	followplay = 1;

	// Output file: convertRAWToWAV derives the temp .raw name from wavfilename.
	strncpy(wavfilename, outPath, MAX_PATHNAME - 1);
	wavfilename[MAX_PATHNAME - 1] = 0;
	GenerateExportFileName();
	OpenExportFileNameForWriting();

	// Samples produced per player frame (matches ExportAsPCM).
	int samplesPerFrame = (int)((mr * 2) / 100);
	if (editorInfo.multiplier == 0) samplesPerFrame *= 2;
	else samplesPerFrame /= editorInfo.multiplier;
	if (samplesPerFrame < 1) samplesPerFrame = 1;

	long targetFrames = frames;
	if (targetFrames < 0)
	{
		long targetSamples = (long)(seconds * (double)rate + 0.5);
		targetFrames = (targetSamples + samplesPerFrame - 1) / samplesPerFrame;
	}

	SDL_Log("%s\n", programname);
	SDL_Log("song:    %s (%d SID channels)\n", loadedsongfilename, editorInfo.maxSIDChannels);
	SDL_Log("out:     %s\n", outPath);
	SDL_Log("backend: %s  model: %s  stereo: %u  timing: %s  rate: %u  interp: %d\n",
		backend ? "legacy-residfp" : "legacy-resid", editorInfo.sidmodel ? "8580" : "6581",
		stereoMode, editorInfo.ntsc ? "NTSC" : "PAL", rate, interp);
	SDL_Log("hardrestart: %04X  speed x%u  (from .sng unless flagged)\n",
		editorInfo.adparam & 0xffff, editorInfo.multiplier);
	SDL_Log("render:  %ld frames x %d samples (~%.2fs)\n",
		targetFrames, samplesPerFrame, (double)(targetFrames * samplesPerFrame) / (double)rate);

	long framesDone = 0;
	for (long f = 0; f < targetFrames; f++)
	{
		playroutine(&gtObject);
		ExportSIDToPCMFile(samplesPerFrame, normalize);
		framesDone++;
		if (gtObject.songinit == PLAY_STOPPED)   // song data error / stop
			break;
	}

	ExportCloseFileHandle();
	convertRAWToWAV(normalize);

	SDL_Log("done:    %ld frames rendered\n", framesDone);
	return 0;
}
