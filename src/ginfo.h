#ifndef GINFO_H
#define GINFO_H

enum INFO_TYPE {
	INFO_CLEAR = 0,
	INFO_UNDO_SIZE,
	INFO_PATTERN_NOTE,
	INFO_INSTRUMENT,
	INFO_CHORD
};


typedef struct
{
	int displayOnOff;
	int value;
	unsigned char *destAddress;
}WAVEFORM_INFO;



#ifndef GINFO_C
extern int lastInfoDisplayed;
extern int clearInfoLine;
extern int forceInfoLine;
extern int lastEditWindow;

extern int lastInfoPatternCh;
extern int lastInfoPattern;
extern int lastInfoPatternPos;
extern int infoWaitMS;
extern int msDelta;
extern int lastMS;

#endif

void displayPatternInfo(GTOBJECT *gt);
void displayInstrumentInfo(GTOBJECT *gt);
void displayTableInfo(GTOBJECT *gt);
void displayWaveTableInfo(GTOBJECT *gt);
void displayPulseTableInfo(GTOBJECT *gt);
void displayFilterTableInfo(GTOBJECT *gt);
void displaySpeedTableInfo(GTOBJECT *gt);
void displayOrderTableInfo(GTOBJECT *gt);

void displayWaveTableLeft(GTOBJECT *gt, char *leftright);
void displayWaveTableRight(GTOBJECT *gt);




#endif
