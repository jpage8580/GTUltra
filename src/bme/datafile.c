//
// Datafile creator
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL/SDL_types.h>
#include "bme_end.h"

#define MAXFILES 16384
#define MAXFILENAME 64

typedef struct
{
    Uint32 offset;
    Uint32 length;
    char name[13];
} HEADER;

static HEADER header[MAXFILES];
static char fullname[MAXFILES][MAXFILENAME];
static int files;

int main(int argc, char **argv);
int addfile(HEADER *header, FILE *dest, char *name);
void fwrite8(FILE *file, unsigned data);
void fwritele16(FILE *file, unsigned data);
void fwritele32(FILE *file, unsigned data);

int main(int argc, char **argv)
{
    FILE *datafile, *listfile;
    int c;

    if (argc < 3)
    {
        printf("Usage: DATAFILE <datafile> <filelistfile>\n\n"
               "The purpose of this program is to gather many files into one datafile, like\n"
               "usually seen in games. The files can be read by BME's IO functions after\n"
               "opening the datafile first with io_opendatafile(). The filelistfile must\n"
               "contain name (use only 8+3 chars) of each file on its own row, for example:\n\n"
               "editor.spr\n"
               "fonts.spr\n");
        return 0;
    }
    listfile = fopen(argv[2], "rt");
    if (!listfile)
    {
        printf("ERROR: Couldn't open filelist.\n");
        return 1;
    }
    datafile = fopen(argv[1], "wb");
    if (!datafile)
    {
        printf("ERROR: Couldn't create datafile.\n");
        fclose(listfile);
        return 1;
    }
    memset(&header[0], 0, MAXFILES * sizeof(HEADER));
    // Get names from list
    for (;;)
    {
        char searchname[64];
        int d;
        FILE *test;

        if (fscanf(listfile, "%63s", searchname) == EOF) break;
        test = fopen(searchname, "rb");
        if (test)
        {
            fclose(test);

            strcpy(fullname[files], searchname);

            for (c = strlen(fullname[files]) - 1; c >= 0; c--)
            {
                if (fullname[files][c] == '\\')
                {
                    c++;
                    break;
                }
            }
            if (c < 0) c = 0;
            memset(header[files].name, 0, 13);
            d = 0;
            while (fullname[files][c])
            {
                header[files].name[d] = toupper(fullname[files][c]);
                c++;
                d++;
            }
            files++;
            if (files == MAXFILES) break;
        }
        if (files == MAXFILES) break;
    }

    fclose(listfile);
    // Write datafile header
    fwrite("DAT!", 4, 1, datafile);
    fwritele32(datafile, files);
    // Write incomplete fileheaders
    for (c = 0; c < files; c++)
    {
        fwritele32(datafile, header[c].offset);
        fwritele32(datafile, header[c].length);
        fwrite(header[c].name, 13, 1, datafile);
    }
    // Process each file
    for (c = 0; c < files; c++)
    {
        printf("Adding %s...\n", header[c].name);
        if (!addfile(&header[c], datafile, fullname[c]))
        {
            printf("Terminating & deleting datafile...\n");
            fclose(datafile);
            remove(argv[1]);
            return 1;
        }
    }
    // Seek back to start & write correct headers
    fseek(datafile, sizeof files + 4, SEEK_SET);
    for (c = 0; c < files; c++)
    {
        fwritele32(datafile, header[c].offset);
        fwritele32(datafile, header[c].length);
        fwrite(header[c].name, 13, 1, datafile);
    }
    fclose(datafile);
    printf("Everything OK!\n");
    return 0;
}

int addfile(HEADER *header, FILE *dest, char *name)
{
    FILE *src;
    unsigned char *originalbuf;

    src = fopen(name, "rb");
    if (!src)
    {
        printf("ERROR: Couldn't open file %s\n", name);
        return 0;
    }
    header->offset = ftell(dest);
    fseek(src, 0, SEEK_END);
    header->length = ftell(src);
    fseek(src, 0, SEEK_SET);
    originalbuf = malloc(header->length);
    if (!originalbuf)
    {
        printf("ERROR: No memory to load file!\n");
        fclose(src);
        return 0;
    }
    printf("* Loading file\n");
    fread(originalbuf, header->length, 1, src);
    fclose(src);
    printf("* Writing file (size was %d)\n", header->length);
    fwrite(originalbuf, header->length, 1, dest);
    free(originalbuf);
    return 1;
}
