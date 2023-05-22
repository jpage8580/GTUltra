//
// GT Ultra Function Keys (User definable actions)
//

#define GFKEYS_C

#include "goattrk2.h"
#include "gfkeys.h"

#define FKEY_REQUIRE_SHIFT 1
#define FKEY_REQUIRE_CTRL 1
#define FKEY_NO_SHIFT 0
#define FKEY_NO_CTRL 0

enum FKEY_ACTIONS
{
	FKEY_SET,
	FKEY_TOGGLE,
	FKEY_FOLLOW,
	FKEY_PLAY_FROM_START_OF_SONG,
	FKEY_DONE,
	FKEY_LOOP,
	FKEY_PLAY_FROM_START_OF_PATTERN,
	FKEY_GTULTRA_PLAY_FROM_CURRENT_POS,			// Play from current pattern step position OR from current orderlist position if currently editing orderlist
	FKEY_GTULTRA_PLAY_FROM_START_OF_PATTERN,	// Play from current pattern step position OR from current orderlist position if currently editing orderlist
	FKEY_STOP,
	FKEY_MUTE_CHANNEL
};


char F1_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,0, FKEY_SET,FKEY_LOOP,0, FKEY_PLAY_FROM_START_OF_SONG,FKEY_DONE };
char F1_SHIFT_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,1,FKEY_SET,FKEY_LOOP,0,FKEY_PLAY_FROM_START_OF_SONG,FKEY_DONE };

char F2_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,0, FKEY_SET,FKEY_LOOP,0, FKEY_PLAY_FROM_START_OF_PATTERN,FKEY_DONE };
char F2_SHIFT_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,1, FKEY_SET,FKEY_LOOP,0, FKEY_PLAY_FROM_START_OF_PATTERN,FKEY_DONE };

char F3_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,0, FKEY_SET,FKEY_LOOP,1, FKEY_PLAY_FROM_START_OF_PATTERN,FKEY_DONE };
char F3_SHIFT_OriginalGT[] = { FKEY_SET,FKEY_FOLLOW,1, FKEY_SET,FKEY_LOOP,1, FKEY_PLAY_FROM_START_OF_PATTERN,FKEY_DONE };

char F4_OriginalGT[] = { FKEY_STOP,FKEY_DONE };
char F4_SHIFT_OriginalGT[] = { FKEY_MUTE_CHANNEL,FKEY_DONE };


GTFKEY_ENTRY GTFKeyInfo[] = {
	{ KEY_F1,FKEY_NO_SHIFT,FKEY_NO_CTRL,FKEYS_NONE,F1_OriginalGT }, { KEY_F1,FKEY_REQUIRE_SHIFT,FKEY_REQUIRE_CTRL,FKEYS_OR,F1_SHIFT_OriginalGT },
	{ KEY_F2,FKEY_NO_SHIFT,FKEY_NO_CTRL,FKEYS_NONE,F2_OriginalGT }, { KEY_F2,FKEY_REQUIRE_SHIFT,FKEY_REQUIRE_CTRL,FKEYS_OR,F2_SHIFT_OriginalGT },
	{ KEY_F3,FKEY_NO_SHIFT,FKEY_NO_CTRL,FKEYS_NONE,F3_OriginalGT }, { KEY_F3,FKEY_REQUIRE_SHIFT,FKEY_REQUIRE_CTRL,FKEYS_OR,F3_SHIFT_OriginalGT },
	{ KEY_F4,FKEY_NO_SHIFT,FKEY_NO_CTRL,FKEYS_NONE,F4_OriginalGT }, { KEY_F4,FKEY_REQUIRE_SHIFT,FKEY_REQUIRE_CTRL,FKEYS_OR,F4_SHIFT_OriginalGT }
};

/*
	FKEY_PANEL,
	FKEY_PREVIOUS_MULTIPLIER,
	FKEY_NEXT_MULTIPLIER,
	FKEY_SELECT_INSTRUMENT_OR_TABLE_PANEL

char F5_OriginalGT[] = { FKEY_SET,FKEY_PANEL,EDIT_PATTERN,FKEY_DONE };
char F5_SHIFT_OriginalGT[] = { FKEY_PREVIOUS_MULTIPLIER,FKEY_DONE };

char F6_OriginalGT[] = { FKEY_SET,FKEY_PANEL,EDIT_ORDERLIST,FKEY_DONE };
char F6_SHIFT_OriginalGT[] = { FKEY_NEXT_MULTIPLIER,FKEY_DONE };

char F7_OriginalGT[] = { FKEY_SELECT_INSTRUMENT_OR_TABLE_PANEL,FKEY_DONE };
char F7_SHIFT_OriginalGT[] = { FKEY_NEXT_MULTIPLIER,FKEY_DONE };
	{ KEY_F5,0,0,FKEYS_NONE,F5_OriginalGT }, { KEY_F5,1,1,FKEYS_OR,F5_SHIFT_OriginalGT },
	{ KEY_F6,0,0,FKEYS_NONE,F6_OriginalGT }, { KEY_F6,1,1,FKEYS_OR,F6_SHIFT_OriginalGT },
	{ KEY_F7,0,0,FKEYS_NONE,F7_OriginalGT }, { KEY_F7,1,1,FKEYS_OR,F7_SHIFT_OriginalGT }
*/




int fkeys_check(GTOBJECT *gt, int rawkey)
{
	if (editPaletteMode)
		return 0;

	for (int i = 0;i < sizeof(GTFKeyInfo) / sizeof(GTFKEY_ENTRY);i++)
	{
		int key = GTFKeyInfo[i].key;
		if (key == rawkey)
		{
			int found = 0;
			if (GTFKeyInfo[i].andOr == FKEYS_NONE)
			{
				if (shiftpressed + ctrlpressed == 0)
					found = 1;
			}
			else if (GTFKeyInfo[i].andOr == FKEYS_AND)
			{
				if ((GTFKeyInfo[i].shift == shiftpressed) && (GTFKeyInfo[i].ctrl == ctrlpressed))
					found = 1;
			}
			else
			{
				if ((GTFKeyInfo[i].shift == shiftpressed) || (GTFKeyInfo[i].ctrl == ctrlpressed))
					found = 1;
			}

			if (found == 1)
			{
				fkeys_processActionList(gt, GTFKeyInfo[i].actionList);
				return 1;
			}
		}
	}
	return 0;
}

void fkeys_processActionList(GTOBJECT *gt, char* actionList)
{
//	int i = 0;
	while (*actionList != FKEY_DONE) {
		char action = *actionList;
		actionList++;
		if (action == FKEY_SET)
		{
			fkey_set(actionList);
		}
		else if (action == FKEY_TOGGLE)
			fkey_toggle(actionList);
		else
			fkey_doAction(gt, action);

	};
}

void fkey_toggle(char *actionList)
{
	char parameterToToggle = *actionList;
	actionList++;
	if (parameterToToggle == FKEY_FOLLOW)
		followplay = 1 - followplay;
	else if (parameterToToggle == FKEY_LOOP)
		transportLoopPattern = 1 - transportLoopPattern;
}

void fkey_set(char *actionList)
{
	char parameterToSet = *actionList;
	actionList++;
	char valueToUse = *actionList;
	actionList++;

	if (parameterToSet == FKEY_FOLLOW)
		followplay = valueToUse;
	else if (parameterToSet == FKEY_LOOP)
		transportLoopPattern = valueToUse;
	/*
		else if (parameterToSet == FKEY_PANEL)
			editorInfo.editmode = valueToUse;
	*/
}

void fkey_doAction(GTOBJECT *gt, char action)
{
	if (action == FKEY_PLAY_FROM_START_OF_SONG)
		orderPlayFromPosition(gt, 0, 0, 0, 1);
	else if (action == FKEY_PLAY_FROM_START_OF_PATTERN)
		playFromCurrentPosition(gt, 0);	//  Play from start of pattern
	else if (action == FKEY_GTULTRA_PLAY_FROM_START_OF_PATTERN)
	{
		if (checkForPlayInOrderList(gt) == 0)
			playFromCurrentPosition(gt, 0);	//  Play from start of pattern
	}
	else if (action == FKEY_GTULTRA_PLAY_FROM_CURRENT_POS)
	{
		if (checkForPlayInOrderList(gt) == 0)
			playFromCurrentPosition(gt, editorInfo.eppos);	//  Play from the current pattern pos
	}
	else if (action == FKEY_STOP)
	{
		stopsong(gt);
		setMasterLoopChannel(gt, "debug_9");
	}
	else if (action == FKEY_MUTE_CHANNEL)
		mutechannel(editorInfo.epchn, gt);

	/*
		else if (action == FKEY_PREVIOUS_MULTIPLIER)
			prevmultiplier();
		else if (action == FKEY_NEXT_MULTIPLIER)
			nextmultiplier();
	*/
}

// Is cursor currently in the orderlist panel? If so, play song from that position
int checkForPlayInOrderList(GTOBJECT *gt)
{
	if (editorInfo.editmode == EDIT_ORDERLIST)	// 1.1.7: Fast select / playback when in OrderList. Just press F3 to play from the cursor pos
	{
		orderSelectPatternsFromSelected(gt);
		orderPlayFromPosition(gt, 0, editorInfo.eseditpos, editorInfo.eschn, 1);
		return 1;
	}
	return 0;
}


char keyLineTextBuffer[MAX_PATHNAME];

int processFKeyCFGTextLine(char *str)
{
	//	char str[] = "strtok needs to be called several times to split a string";
	char delim[] = " ";
	char *ptr = strtok(str, delim);

	while (ptr != NULL)
	{
		printf("'%s'\n", ptr);
		ptr = strtok(NULL, delim);
	}
	return 0;
}

FILE *fkeys;

int fkeys_loadCFG()
{
	char *fkeyTextPtr;

	fkeys = fopen(fkeysFilename, "rt");		//Have we a local copy of the charset? If so, use that. Otherwise, use the one in the wad file..
	if (fkeys)
	{
		do {
			fkeyTextPtr = &keyLineTextBuffer[0];
			if (getFkeyLine(fkeys, fkeyTextPtr) == 1)
			{
				processFKeyCFGTextLine(fkeyTextPtr);
			}
			else
				break;
		} while (1);
		fclose(fkeys);
		return 1;
	}
	return 0;
}


int getFkeyLine(FILE *handle, char *textLine)
{
	for (;;)
	{
		if (feof(handle)) return 0;
		if (fgets(textLine, MAX_PATHNAME, handle) == NULL)
			return 0;

		if ((textLine[0]) && (textLine[0] != ';') && (textLine[0] != ' ') && (textLine[0] != 13) && (textLine[0] != 10))
		{
			int l = strlen(textLine);
			int e = l;
			l -= 2;
			if (l < 0)
				l = 0;
			for (int i = l;i < e;i++)
			{
				if (textLine[i] == 10 || textLine[i] == 13)
					textLine[i] = 0;
			}
			return 1;
		}
	}
}
