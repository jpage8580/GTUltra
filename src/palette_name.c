#include "palette_name.h"
#include <stdlib.h>
#include <string.h>

char *palette_name_dup(char **names, int index, const char *name)
{
	if (names[index] != NULL)
	{
		free(names[index]);
		names[index] = NULL;
	}
	if (name == NULL)
		return NULL;
	names[index] = strdup(name);	// NUL-terminated copy; strlen+1 bytes
	return names[index];
}
