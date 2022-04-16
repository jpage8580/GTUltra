#ifndef GUNDO_H
#define GUNDO_H

#define MAX_UNDO 20

typedef struct GTUNDO_OBJECT GTUNDO_OBJECT;
typedef struct GTUNDO_AREA GTUNDO_AREA;

struct GTUNDO_OBJECT
{
	char *jtest;
	char *dest;
	char *data;
	int offset;
	int size;
	int counter;
	int type;
	GTUNDO_AREA *parentArea;
	
} ;

#define UNDO_ALL_SUBINDEXES -1

struct GTUNDO_AREA
{
	int undoAreaSubIndex;
	int undoAreaType;	//UNDO_AREA.*
	int checkForChangeFlag;
	GTUNDO_OBJECT *undoObject;
	int jtest;

};

enum UNDO_AREA {
	UNDO_AREA_PATTERN = 0,
	UNDO_AREA_ORDERLIST,
	UNDO_AREA_PATTERN_LEN,
	UNDO_AREA_INSTRUMENTS,
	UNDO_AREA_TABLES,	
	UNDO_AREA_WAVE_TABLE = UNDO_AREA_TABLES,
	UNDO_AREA_PULSE_TABLE,
	UNDO_AREA_FILTER_TABLE,
	UNDO_AREA_SPEED_TABLE,
	UNDO_AREA_CHANNEL_EDITOR_INFO,
	UNDO_AREA_ORDERLIST_LEN	
};

#define UNDO_AREA_SIZE 512	// guess.. 256 patterns.. plus other stuff
#define UNDO_AREA_DIRTY_CHECK 0x5a
#define UNDO_AREA_CLEAN 0


#ifndef GUNDO_C
extern GTUNDO_OBJECT undoList[MAX_UNDO];
extern int currentUndoPosition;
extern int currentUndoCounter;
extern int maxUndoSize;
extern int initAreaListFlag;
extern int initUndoBufferFlag;
extern int jcounter;
extern int undoCounter;
extern unsigned int undoBufferSize;
#endif

void initUndoBuffer();
void undoInitAllAreas(GTOBJECT *gt);
void undoInitUndoAreaList();
void undoInitUndoArea(char *mem, int size, int undoAreaType, int subIndex);
void undoAreaSetCheckForChange(int areaType, int areaIndex, int onOff);
void* undoCreateUndoObject(char *mem, int size,int offset);
int undoValidateUndoAreas(GTUNDO_OBJECT *editorSettings);
int undoAddUndoObjectToList(GTUNDO_OBJECT *gu, GTUNDO_AREA *gArea, char *mem, int size,int offset);
GTUNDO_OBJECT* undoCreateEditorInfo();
int undoPerform();
void undoFreeUndoObject(GTUNDO_OBJECT *gu);
void undoFinalizeUndoPackage(GTUNDO_OBJECT *editorSettings);
void undoFreeAll();
int quickAddObjectToList(char *m, GTUNDO_AREA *gArea);
int getUndoPacket(int *currentOffset, char *cmp1, char *cmp2, int *startOffset, int*endOffset, int areaSize);
void undoDisplay();
void refreshVariables();
void updateUndoBuffer(int undoAreaType);


#endif
