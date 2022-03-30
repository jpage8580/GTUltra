#ifndef GPATTERN_H
#define GPATTERN_H

#define KEYBOARD_POLYPHONY 12

#ifndef GPATTERN_C

extern int playingChannelOnKey[KEYBOARD_POLYPHONY];

extern int MIDINotesHeld;

extern char keyOffsetText[100];
#endif

typedef struct {
// From gtstereo.c
	int editmode;
	int editTableMode;
	int nameIndex;
	int mouseTrack;	// track position of mouse whilst mouse down to modify value
	int mouseTrackX;	// original X position when holding left mouse button
	int mouseTrackY;	// original Y position when holding left mouse button
	int mouseTrackOriginalValue;
	int mouseTrackDoUndo;

	int cursorX;
	int cursorY;

// From order.c
	int eseditpos;
	int esview;
	int escolumn;
	int eschn;
	int esnum;
	int esmarkchn;	// = -1;
	int esmarkstart;
	int esmarkend;
	int enpos;

// From pattern.c
	int eppos;
	int epview;
	int epcolumn;
	int epchn;
	int epoctave;
	int epmarkchn;
	int epmarkstart;
	int epmarkend;

	int highlightLoopStart;
	int highlightLoopEnd;
	int highlightLoopPatternNumber;
	int highlightLoopChannel;

// From ginstr.c
	int einum;
	int eipos;
	int eicolumn;

// From gtable.c
	int etview[MAX_TABLES];
	int etnum;
	int etpos;
	int etcolumn;
	int etlock;	// = 1;
	int etmarknum;	// = -1;
	int etmarkstart;
	int etmarkend;

	int etDetailedWaveTableColumn;

}EDITOR_INFO;

extern EDITOR_INFO editorInfo;
extern EDITOR_INFO editorInfoBackup;	// copy of editorInfo. Used when hitting enter to move to speed table entry from instrument / wavetable.
extern int disableEnterToReturnToLastPos;	// if true, pressing enter wont return cursor to previous position


void patterncommands(GTOBJECT *gt, int midiNote);
void nextpattern(GTOBJECT *gt);
void prevpattern(GTOBJECT *gt);
int patternup(GTOBJECT *gt);
int patterndown(GTOBJECT *gt);
void shrinkpattern(GTOBJECT *gt);
void expandpattern(GTOBJECT *gt);
void splitpattern(GTOBJECT *gt);
void joinpattern(GTOBJECT *gt);

void displayPatternInfo(GTOBJECT *gt);
void handleShiftSpace(GTOBJECT *gt, int c2, int startPatternPos, int follow, int enableLoop);
int handlePolyphonicKeyboard(GTOBJECT *gt);
int handleMIDIPolykeyboard(GTOBJECT *gt, MIDI_MESSAGE midiData);
int getNote(int rawkey);
int findFreePolyChannel(int note);
int clearPolyChannel(int c, GTOBJECT *gt);
void calculateNoteOffsets();
int findNote(int lowestNote);
void autoPitchbendToNextNote(GTOBJECT *gt);
short getNoteFreq(int noteIndex);
void getPlayStartPosition(GTOBJECT *gte, int songNum, int c2, int songPos, int patternPos);
void keyOn(int qwertyKey, int note, GTOBJECT* gt);
void keyOff(int note, GTOBJECT *gt);
int getNoteFromChannel(int c);
int checkAnyPolyPlaying();
void initPolyKeyboard();

#endif
