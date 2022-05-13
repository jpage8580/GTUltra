#ifndef GPALETTEEDITOR_H
#define GPALETTEEDITOR_H

#ifndef GPALETTEEDITOR_C

extern char* paletteText[];
extern struct dirent *paletteFolderEntry;
extern char paletteFile[256];
extern char *paletteNames[16];
extern char paletteStringBuffer[MAX_PATHNAME];
extern int currentLoadedPresetIndex;
#endif

void displayPaletteEditorWindow(GTOBJECT *gt);
int getPaletteTextArraySize();
int paletteEdit(int *cx, int *cy, GTOBJECT *gt);
void changePalettePreset(int change, GTOBJECT *gt);
void process32EntryPalette(int maxPresets, int maxPaletteEntries, char* tempPalette);
void copyRGBInfo();
void handlePaste(int *cx, GTOBJECT *gt);
void rememberCurrentRGB(int *cx);
int savePalette(GTOBJECT *gt);
void convert4BitPaletteTo8Bit();
int convertStringToHex(char *str);
int loadPalette(char *paletteName);
int loadPalettes();
int savePaletteText();
int readPaletteData(char *paletteMem);
char *sgets(char *s, int n, char **strp);

#endif
