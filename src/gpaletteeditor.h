#ifndef GPALETTEEDITOR_H
#define GPALETTEEDITOR_H

#ifndef GPALETTEEDITOR_C

extern char* paletteText[];
#endif

void displayPaletteEditorWindow(GTOBJECT *gt);
int getPaletteTextArraySize();
int paletteEdit(int *cx, int *cy, GTOBJECT *gt);
void changePalettePreset(int change, GTOBJECT *gt);


#endif
