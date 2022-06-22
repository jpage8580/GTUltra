#ifndef GDISPLAY_H
#define GDISPLAY_H


#define PANEL_NAMES_X 1	//60
#define PANEL_NAMES_Y 37 //33
#define PANEL_TABLES_X 60
#define PANEL_TABLES_Y 25 //17
#define PANEL_INSTR_X 60
#define PANEL_INSTR_Y 18
#define PANEL_ORDER_X 60
#define PANEL_ORDER_Y 2
#define EXTENDEDVISIBLEORDERLIST 13

#ifndef GDISPLAY_C


extern char debugtext[256];
extern char* paletteText[];
extern int displayOriginal3Channel;
extern int timemin;
extern int timesec;
extern int timeframe;
extern int lastDisplayChanCount;
extern int expandOrderListView;
#endif

 
#define FIRST_UI_COLOR 0x20

enum UI_COLORS {
	CPATTERN_BACKGROUND1 = FIRST_UI_COLOR,
	CPATTERN_FOREGROUND1,
	CPATTERN_BACKGROUND2,
	CPATTERN_FOREGROUND2,
	CPATTERN_HIGHLIGHT_BACKGROUND,
	CPATTERN_HIGHLIGHT_FOREGROUND,
	CPATTERN_NOTE_FOREGROUND,	// uses background from above
	CPATTERN_COMMAND_FOREGROUND,
	CPATTERN_DIVIDER_LINE,
	CPATTERN_HIGHLIGHT_PLAYING_LINE_BACKGROUND,
	CPATTERN_HIGHLIGHT_PLAYING_LINE_FOREGROUND,
	CUNUSED_MUTED_BACKGROUND,
	CUNUSED_MUTED_FOREGROUND,
	CSELECT_TO_COPY_BACKGROUND,
	CSELECT_TO_COPY_FOREGROUND,
	CTABLE_BACKGROUND1,
	CTABLE_FOREGROUND1,
	CTABLE_BACKGROUND2,
	CTABLE_FOREGROUND2,
	CTABLE_UNUSED_BACKGROUND,
	CTABLE_UNUSED_FOREGROUND,
	CTABLE_SELECTED_INSTRUMENT_FOREGROUND,
	CTABLE_SELECTED_INSTRUMENT_BACKGROUND,
	CGENERAL_BACKGROUND,
	CTITLES_FOREGROUND,
	CINFO_FOREGROUND,
	CGENERAL_HIGHLIGHT,
	CTRANSPORT_BUTTON_BACKGROUND,
	CTRANSPORT_FOREGROUND,
	CTRANSPORT_ENABLED,
	CCOLOR_RED,
	CPATTERN_LOOP_MARKER_BACKGROUND,
	CPATTERN_LOOP_MARKER_FOREGROUND,
	CPATTERN_INSTRUMENT_FOREGROUND,
	CPATTERN_FIRST_BACKGROUND1,
	CPATTERN_FIRST_FOREGROUND1,
	CPATTERN_FIRST_BACKGROUND2,
	CPATTERN_FIRST_FOREGROUND2,
	CPATTERN_INDEX_HIGHLIGHT,
	CTRANSPORT_BACKGROUND2,
	TOPBAR_BACKGROUND,
	CTRANSPORT_BETWEEN_BUTTONS,
	CCOLOR_EXCLAMATION,
	CORDER_INST_BACKGROUND,
	CORDER_INST_FOREGROUND,
	CORDER_INST_TABLE_EDITING,
	CORDER_TRANS_REPEAT,
	TOPBAR_FOREGROUND,
	TOPBAR_FOREGROUND_OFF,
	CTRANSPORT_BUTTON_FOREGROUND
};


extern int UIUnderline;


#define PATTERN_X 0
#define PATTERN_Y 2+4+1-4-1

#define TRANSPORT_BAR_X 14
#define TRANSPORT_BAR_Y 33

#define CNORMAL 8
#define CMUTE 3
#define CEDIT 10
#define CPLAYING 12
#define CCOMMAND 7
#define CTITLE 15

void displayTransportBar(GTOBJECT *gt);
void displayTransportBarLoopPattern(int x, int y);
void displayTransportBarFollow(int x, int y);
void displayTransportBarRecord(int x, int y);
void displayTransportBarPlaying(GTOBJECT *gt, int x, int y);
void displayTransportBarPolyChannels(int x, int y);
void displayTransportBarFastForward(int x, int y);
void displayTransportBarRewind(int x, int y);
void displayTransportBarOctave(int x, int y);
void displayTransportBarSkinning(int x, int y);
void displayTransportBarSIDCount(int x, int y);
void displayTransportBarKeyboard(int x, int y);
void displayTransportBarMasterVolume(int x, int y);
void displayTransportBarDetune(int x, int y);
void displayTransportBarMonoStereo(int x, int y);

int getTableTitleColour(int c);
void displayOriginalTableView(int cc,int OX,int OY);
void displayDetailedWaveTable(int cc, int OX, int OY);
void displayDetailedFilterTable(int cc, int OX, int OY);
void displayDetailedPulseTable(int cc, int OX, int OY);


void displayPattern(GTOBJECT *gt);
void displayPattern3Chn(GTOBJECT *gt);
void displayPattern6Chn(GTOBJECT *gt);
void printmainscreen(GTOBJECT *gt);
void displayupdate(GTOBJECT *gt);
void printstatus(GTOBJECT *gt);
void resettime(GTOBJECT *gt);
void incrementtime(GTOBJECT *gt);
void displayOrderList(GTOBJECT *gt, int cc,int OX, int OY);
void displayPaletteInfo(int cc);
void clearOrderListDisplay();
int getPaletteTextArraySize();
void setSongLengthTime(GTOBJECT *gt);
void displayInstrument(GTOBJECT *gt, int cc,int OX,int OY);
void displaySongInfo(int cc,int OX,int OY);
void updateDisplayWhenFollowingAndPlaying(GTOBJECT *gt);
void displayTopBar(int menu, int cc);
void displayExpandedOrderList(GTOBJECT *gt, int cc, int OX, int OY);

void displayKeyboard();
void setNote(int noteNumber);
void resetKeyboardDisplay();
void displayNotes(GTOBJECT *gt);
void displayTables(int OX,int OY);
void displayTable(int c,int OX,int OY);

void displayWaveformInfo(int x, int y);
int getWaveforumColour(int bit, int value);



#endif
