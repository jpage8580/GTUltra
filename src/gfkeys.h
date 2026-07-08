#ifndef GFKEYS_H
#define GFKEYS_H

typedef struct GTFKEY_ENTRY GTFKEY_ENTRY;

enum GTFKEY_ANDOR
{
	FKEYS_AND = 0,
	FKEYS_OR,
	FKEYS_NONE
};

struct GTFKEY_ENTRY
{
	int key;
	int shift;
	int ctrl;
	int andOr;
	char *actionList;
};




#ifndef GFKEYS_C

#endif

int fkeys_check(GTOBJECT *gt, int rawkey);
void fkeys_processActionList(GTOBJECT *gt, char *actionList);
void fkey_doAction(GTOBJECT *gt, char action);
void fkey_set(char *actionList);
void fkey_toggle(char *actionList);
int checkForPlayInOrderList(GTOBJECT *gt);
int processFKeyCFGTextLine(char *str);
int fkeys_loadCFG();
int getFkeyLine(FILE *handle, char *textLine);

#endif
