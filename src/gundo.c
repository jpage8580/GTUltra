//
// GT Ultra Undo
//

#define GUNDO_C

#include "goattrk2.h"
#include "gundo.h"

char jtext[256];

char **undoList;	//[MAX_UNDO];
char **undoAreaList;	// this list is checked after every change to see if there's a dirty table / pattern

int currentUndoPosition = 0;
int currentUndoPackagesCounter = 1;
unsigned int undoBufferSize = 0;
int undoCounter = 0;



int initUndoBufferFlag = 0;
int maxUndoSize = MAX_UNDO;
void initUndoBuffer()
{
	undoList = malloc((sizeof(char*)) * maxUndoSize);
}

int initAreaListFlag = 0;

/*
initAllAreas
Needs to be called on initialisation and when a new song is loaded

create Areas for each part of GT
- one for each pattern
- orderlist
- instruments
- each table
*/
void undoInitAllAreas(GTOBJECT *gt)
{
	if (REMOVE_UNDO)
		return;

	undoFreeAll();

	for (int i = 0;i < MAX_PATT;i++)
	{
		undoInitUndoArea((char*)&pattern[i], MAX_PATTROWS * 4 + 4, UNDO_AREA_PATTERN, i);

	}

	for (int i = 0;i < MAX_PLAY_CH;i++)
	{
		undoInitUndoArea((char*)&gt->editorInfo[i], sizeof(CHN_EDITOR_INFO), UNDO_AREA_CHANNEL_EDITOR_INFO, i);
	}

	undoInitUndoArea((char*)&pattlen, MAX_PATT * sizeof(int), UNDO_AREA_PATTERN_LEN, 0);

	for (int s = 0;s < MAX_SONGS;s++)
	{
		for (int i = 0;i < MAX_CHN;i++)
		{
			undoInitUndoArea((char*)&songorder[s][i], MAX_SONGLEN + 2, UNDO_AREA_ORDERLIST, i + (s*MAX_CHN));
		}
	}

	for (int i = 0;i < MAX_INSTR;i++)
	{
		undoInitUndoArea((char*)&instr[i], sizeof(INSTR), UNDO_AREA_INSTRUMENTS, i);
	}


	for (int i = 0;i < MAX_TABLES;i++)
	{
		undoInitUndoArea((char*)&ltable[i], MAX_TABLELEN, UNDO_AREA_TABLES + i, 0);
		undoInitUndoArea((char*)&rtable[i], MAX_TABLELEN, UNDO_AREA_TABLES + i, 1);
	}

	undoBufferSize = 0;	// reset this. Ignore Area allocations
	currentUndoPosition = 0;
}

void undoFreeAll()
{
	if (REMOVE_UNDO)
		return;

	if (initAreaListFlag)
	{
		initAreaListFlag = 0;

		// Free undoBuffer, including each undoObject within it.
		if (initUndoBufferFlag)
		{
			initUndoBufferFlag = 0;

			GTUNDO_OBJECT *gu;

			for (int i = 0;i < currentUndoPosition;i++)
			{
				gu = (GTUNDO_OBJECT*)undoList[i];
				undoFreeUndoObject(gu);
			}
		}

		free(undoList);

		// Free undo Area List, including each undoObject within it.

		GTUNDO_AREA *gArea;

		for (int i = 0;i < UNDO_AREA_SIZE;i++)
		{
			gArea = (GTUNDO_AREA *)undoAreaList[i];
			if (gArea)
			{
				undoFreeUndoObject(gArea->undoObject);	// free's object and gArea
			}
		}

		free(undoAreaList);
	}


}

// called by undoInitUndoArea if not previously called
void undoInitUndoAreaList()
{
	if (REMOVE_UNDO)
		return;

	int size = (sizeof(char*)) * UNDO_AREA_SIZE;
	undoAreaList = malloc(size);

	for (int i = 0;i < UNDO_AREA_SIZE;i++)
	{
		undoAreaList[i] = NULL;
	}

}

int undoAreaIndex = 0;

/*
example:
undoAreaType = UNDO_AREA_PATTERN
subIndex = pattern index 0-208
*/

void undoInitUndoArea(char *mem, int size, int undoAreaType, int subIndex)
{
	if (REMOVE_UNDO)
		return;

	if (!initAreaListFlag)
	{
		initAreaListFlag++;
		undoInitUndoAreaList();
		undoAreaIndex = 0;
	}

	/*
	Each area is for a specific part of GT (patterns, speed table, instruments, order table..)
	When we edit, say, patterns, we set the checkForChangeFlag=1.
	When we return from the pattern user control routine, we know to check that buffer for changes.
		- If we add the buffer to the undo list due to it being different, we then need to perform the memcpy again
			to get a copy of this data into the GTUNDO_OBJECT for this specific area
	*/
	char* m = malloc(sizeof(GTUNDO_AREA));
	GTUNDO_AREA *ga = (GTUNDO_AREA*)m;

	GTUNDO_OBJECT *gu = undoCreateUndoObject(mem, size, 0);
	ga->undoObject = gu;
	ga->undoAreaType = undoAreaType;
	ga->undoAreaSubIndex = subIndex;
	ga->checkForChangeFlag = 0;

	gu->jtest = (char*)&jtext[0];

	ga->jtest = 1234;
	undoAreaList[undoAreaIndex] = (char*)ga;
	undoAreaIndex++;

}

void undoAreaSetCheckForChange(int areaType, int areaIndex, int onOff)
{
	if (REMOVE_UNDO)
		return;

	for (int i = 0;i < UNDO_AREA_SIZE;i++)
	{
		GTUNDO_AREA *gArea = (GTUNDO_AREA *)undoAreaList[i];
		if (gArea != NULL)
		{
			if (gArea->undoAreaType == areaType)
			{
				if (gArea->undoAreaSubIndex == areaIndex || areaIndex == UNDO_ALL_SUBINDEXES)
				{
					gArea->checkForChangeFlag = onOff;
				}
			}
		}
	}
}



void* undoCreateUndoObject(char *mem, int size, int offset)
{
	if (REMOVE_UNDO)
		return NULL;

	if (!initUndoBufferFlag)
	{
		initUndoBufferFlag++;
		initUndoBuffer();
	}

	char *mem2 = mem + offset;

	char* m = malloc(sizeof(GTUNDO_OBJECT));
	GTUNDO_OBJECT *gu = (GTUNDO_OBJECT*)m;

	gu->data = malloc(size);
	memcpy(gu->data, mem2, size);
	gu->size = size;
	gu->dest = mem;
	gu->offset = offset;

	undoBufferSize += size;	// ignoring mallocs for GTUNDO_OBJECT

	return gu;
}

int undoPackageCounter = 0;
int jcounter = 0;

int undoValidateUndoAreas(GTUNDO_OBJECT *editorSettings)
{
	if (REMOVE_UNDO)
		return 0;

	int areaDirty = 0;
	for (int i = 0;i < UNDO_AREA_SIZE;i++)
	{
		GTUNDO_AREA *gArea = (GTUNDO_AREA *)undoAreaList[i];

		if (gArea != NULL)
		{

			if (gArea->checkForChangeFlag == UNDO_AREA_DIRTY_CHECK)
			{
				gArea->checkForChangeFlag = 0;

				int diff = memcmp(gArea->undoObject->dest, gArea->undoObject->data, gArea->undoObject->size);
				if (diff)
				{

					char *cmp1 = gArea->undoObject->data;
					char *cmp2 = gArea->undoObject->dest;

					int size = gArea->undoObject->size;
					int startOffset = -1;
					int currentOffset = 0;
					int endOffset = 0;


					while (currentOffset < gArea->undoObject->size)
					{
						if (getUndoPacket(&currentOffset, cmp1, cmp2, &startOffset, &endOffset, size))
						{
							int sizeToUndo = (endOffset - startOffset) + 1;

							undoAddUndoObjectToList(gArea->undoObject, gArea, gArea->undoObject->data, sizeToUndo, startOffset);

							// Now copy the new buffer to the undo area, so this is used for comparrison next time
							char *dest = gArea->undoObject->data;
							dest += startOffset;
							char *source = gArea->undoObject->dest;
							source += startOffset;

							memcpy(dest, source, sizeToUndo);

							// Mark as dirty. So we know to finalize the packets (so they all process with a single ctrl-z)
							areaDirty++;
						}
						else
							break;
					}
				}
			}
		}
	}

	if (areaDirty)
	{
		// mark the packets so that they all update with ctrl-z is pressed. Including editor update so cursor is in correct pos, etc.
		undoFinalizeUndoPackage(editorSettings);
		return 1;
	}

	// No data has changed
	return 0;
}


int getUndoPacket(int *currentOffset, char *cmp1, char *cmp2, int *startOffset, int*endOffset, int areaSize)
{
	if (REMOVE_UNDO)
		return 0;

	int co = *currentOffset;

	*startOffset = -1;
	int foundPacket = 0;
	int waitForPacketCounter = 0;
	while (co < areaSize)
	{
		char a = cmp1[co];
		char b = cmp2[co];

		if (a != b)
		{
			waitForPacketCounter = 0;		// reset counter
			foundPacket = 1;
			if (*startOffset == -1)
			{
				*startOffset = co;
			}
			*endOffset = co;
		}
		else if (foundPacket == 1)
		{
			waitForPacketCounter++;		// check to see if there's any more changes around this area (saves on data packets if we can catch as much as possible
			if (waitForPacketCounter == 4)
			{
				co++;
				*currentOffset = co;	// Can't use *currentOffset++, as this adds 4.. duh..
				return 1;	// we've got a valid packet 
			}
		}


		co++;

	}
	*currentOffset = co;
	return foundPacket;	// end of buffer to search. Could have had data changed at the end where waitForPacketCounter was <4

}


// offset = offset into dest to write data
int undoAddUndoObjectToList(GTUNDO_OBJECT *gu, GTUNDO_AREA *gArea, char *mem, int size, int offset)
{
	if (REMOVE_UNDO)
		return 0;

	char *m = undoCreateUndoObject(mem, size, offset);	//gu->data, gu->size);	// dest

	return quickAddObjectToList(m, gArea);
}


int quickAddObjectToList(char *m, GTUNDO_AREA *gArea)
{
	if (REMOVE_UNDO)
		return 0;

	// Getting close to running out of memory for undo pointers? increase it
	if (currentUndoPosition >= maxUndoSize - 10)
	{
		int newMaxUndoSize = maxUndoSize + 500;	//500
		char **newUndoList = malloc((sizeof(char*)) * newMaxUndoSize);
		memcpy(newUndoList, undoList, (sizeof(char*)) * maxUndoSize);

		free(undoList);
		undoList = newUndoList;
		maxUndoSize = newMaxUndoSize;
	}

	GTUNDO_OBJECT *uo = (GTUNDO_OBJECT*)m;
	uo->parentArea = gArea;

	if (gArea != NULL)
		uo->type = gArea->undoAreaType;
	else
		uo->type = 0x4567;

	undoList[currentUndoPosition] = m;
	currentUndoPosition++;
	undoPackageCounter++;

	return currentUndoPosition;
}

GTUNDO_OBJECT* undoCreateEditorInfo()
{
	if (REMOVE_UNDO)
		return 0;

	char *m = undoCreateUndoObject((char*)&editorInfo, sizeof(EDITOR_INFO), 0);
	//	undoBufferSize -= sizeof(EDITOR_INFO);
	GTUNDO_OBJECT *ed = (GTUNDO_OBJECT*)m;

	return ed;
}


void undoFinalizeUndoPackage(GTUNDO_OBJECT *editorSettings)
{
	if (REMOVE_UNDO)
		return;

	if (undoPackageCounter == 0)
		return;

	undoCounter++;	// we can use this to know if anything has been modified in the editor

	quickAddObjectToList((char*)editorSettings, NULL);

	EDITOR_INFO *ed = (EDITOR_INFO*)editorSettings->data;

	int c = 0;
	for (int i = 0;i < undoPackageCounter;i++)
	{
		GTUNDO_OBJECT *ed = (GTUNDO_OBJECT*)undoList[currentUndoPosition - (1 + i)];

		ed->counter = (undoPackageCounter - 1) - i;
		c++;
	}

	undoPackageCounter = 0;
}


int undoPerform()
{
	if (REMOVE_UNDO)
		return 0;

	GTUNDO_OBJECT *gu;


	if (currentUndoPosition == 0)
		return 0;


	gu = (GTUNDO_OBJECT*)undoList[currentUndoPosition - 1];


	int counter;
	int c = 0;
	do {
		currentUndoPosition--;

		gu = (GTUNDO_OBJECT*)undoList[currentUndoPosition];
		counter = gu->counter;

		char *dest = gu->dest;
		dest += gu->offset;
		memcpy(dest, gu->data, gu->size);	// overwrite original buffer

		if (gu->parentArea != NULL)	// NULL if overwriting editor UI
		{
			dest = gu->parentArea->undoObject->dest;
			dest += gu->offset;
			memcpy(dest, gu->data, gu->size);	// overwrite undo area data too, so that's now updated
		}
		undoFreeUndoObject(gu);

		undoCounter++;	// we can use this to know if anything has been modified in the editor


		c++;

	} while (counter > 0);

	// These need to be in their own routine, called on init, load and here.

	refreshVariables();
	undoDisplay();

}

void refreshVariables()
{
	if (REMOVE_UNDO)
		return;

	countInstruments();
	setTableBackgroundColours(editorInfo.einum);
}

void undoFreeUndoObject(GTUNDO_OBJECT *gu)
{
	if (REMOVE_UNDO)
		return;

	free(gu->data);
	undoBufferSize -= gu->size;
	free(gu);

}


void undoDisplay()
{
	if (REMOVE_UNDO)
		return;

	sprintf(infoTextBuffer, "undo buffer size:%d        ", undoBufferSize);

}






