#ifndef GORDER_H
#define GORDER_H

#ifndef GORDER_C

int instrumentCount[MAX_INSTR];
#endif

int calculateLoopInfo2(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop);
int addOrRemoveInterPatternLoop();
int calculateLoopInfo(int startPatternPos, int testForRemoveLoop);
int calcStartofInterPatternLoop(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop);
void setMasterLoopChannel(GTOBJECT *gt);

void orderPlayFromPosition(GTOBJECT *gt, int startPatternPos, int startSongPos, int focusChannel);
void orderSelectPatternsFromSelected(GTOBJECT *gt);
void updateviewtopos(GTOBJECT *gt);
void orderlistcommands(GTOBJECT *gt);
void namecommands(GTOBJECT *gt);
void nextsong(GTOBJECT *gt);
void prevsong(GTOBJECT *gt);
void songchange(GTOBJECT *gt, int resetEditingPositions);
void orderleft(void);
void orderright(void);
void deleteorder(GTOBJECT *gt);
void insertorder(unsigned char byte, GTOBJECT *gt);
void countInstruments();
void calculateTotalInstrumentsFromAllPatterns();
void countInstrumentsInPattern(int pat);
void resetOrderView(GTOBJECT *gt);


#endif
