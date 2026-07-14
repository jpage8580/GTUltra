#ifndef PALETTE_NAME_H
#define PALETTE_NAME_H

// Pure palette-name storage helper, sprouted out of setPaletteName() so it can be
// unit-tested without gpaletteeditor.c's ~58 editor/SDL dependencies. No editor or
// SDL includes here on purpose.
//
// Frees any existing string in names[index], then stores a NUL-terminated heap copy
// of `name` in that slot. Passing name == NULL just clears (frees) the slot.
// Returns the stored pointer (NULL when name is NULL or allocation fails).
char *palette_name_dup(char **names, int index, const char *name);

#endif
