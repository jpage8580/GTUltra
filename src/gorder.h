#ifndef GORDER_H
#define GORDER_H

#ifndef GORDER_C

extern int instrumentCount[MAX_INSTR];
#endif

int calculateLoopInfo2(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop);
int addOrRemoveInterPatternLoop();
int calculateLoopInfo(int startPatternPos, int testForRemoveLoop);
int calcStartofInterPatternLoop(int songNum, int channelNum, int startSongPos, GTOBJECT *gtloop);
void setMasterLoopChannel(GTOBJECT *gt,char *debugText);

void orderPlayFromPosition(GTOBJECT *gt, int startPatternPos, int startSongPos, int focusChannel, int enableSIDWrites);
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
void orderListHandleHexInputExpandedView(GTOBJECT *gt);
void orderListHandleHexInputOriginalView(GTOBJECT *gt);
void resetSongInfo(GTOBJECT *gt, int jc2);
int findFirstEndMarkerIndex(int sng, int chn);
void updateTransposeToPlayingSong(GTOBJECT *gt);
int handleEnterInCompressedView(GTOBJECT *gt);
int handleEnterInExpandedView(GTOBJECT *gt);
void initEditorSongInfo(GTOBJECT *gt);
void orderListCopyMarkedArea();
void orderListCopyMarkedArea_Expanded();
void getExpandedSelectedArea(int *x, int *y, int *w, int *h);
void orderListPasteToCursor(GTOBJECT *gt);
void orderListPasteToCursor_External(GTOBJECT *gt, int insert, int transposeOnly);
void orderListInsert_External(GTOBJECT *gt);
void orderListInsertRowAtCursor_External(GTOBJECT *gt, int sng, int chn, int row);
void orderListDeleteRowAtCursor_External(int sng,int chn, int row);
void orderListDelete_External();
#endif
