#ifndef GSONG_H
#define GSONG_H

#ifndef GSONG_C
extern INSTR instr[MAX_INSTR];



extern unsigned char detailedTableRValue[MAX_TABLELEN];
extern unsigned char detailedTableMaxRValue[MAX_TABLELEN];
extern unsigned char detailedTableMinRValue[MAX_TABLELEN];
extern int detailedTableBaseRValue[MAX_TABLELEN];

extern unsigned int detailedTableLValue[MAX_TABLELEN];
extern unsigned char detailedTableMaxLValue[MAX_TABLELEN];
extern unsigned char detailedTableMinLValue[MAX_TABLELEN];
extern int detailedTableBaseLValue[MAX_TABLELEN];

extern unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];

extern unsigned char songOrderPatterns[MAX_SONGS][MAX_CHN][MAX_SONGLEN_EXPANDED];
extern unsigned short songOrderTranspose[MAX_SONGS][MAX_CHN][MAX_SONGLEN_EXPANDED];
extern unsigned int songOrderLength[MAX_SONGS][MAX_CHN];
extern unsigned int songCompressedSize[MAX_SONGS][MAX_CHN];

extern unsigned char songOrderPatternsCopyPaste[MAX_CHN][MAX_SONGLEN_EXPANDED];
extern unsigned short songOrderTransposeCopyPaste[MAX_CHN][MAX_SONGLEN_EXPANDED];
extern int copyPasteW;
extern int copyPasteH;
extern int copyExpandedSongValidFlag;	// 1 - we've performed a copy/cut already. 0 - we haven't

extern unsigned char pattern[MAX_PATT][MAX_PATTROWS*4+4];
extern char songname[MAX_STR];
extern char authorname[MAX_STR];
extern char copyrightname[MAX_STR];
extern int pattlen[MAX_PATT];
extern int songlen[MAX_SONGS][MAX_CHN];
extern int highestusedpattern;
extern int highestusedinstr;
extern int loadedSongCount;
#endif

int loadsong(GTOBJECT *gt);
int mergesong(GTOBJECT *gt);
void loadinstrument(GTOBJECT *gt);
int savesong(void);
int saveinstrument(void);
void clearsong(int cs, int cp, int ci, int cf, int cn,GTOBJECT *gt);
void countpatternlengths(void);
void countthispattern(GTOBJECT *gt);
void clearpattern(int p);
int insertpattern(int p, GTOBJECT *gt);
void deletepattern(int p, GTOBJECT *gt);
void findusedpatterns(void);
void findduplicatepatterns(GTOBJECT *gt);
void optimizeeverything(int oi, int ot,GTOBJECT *gt);
void initQuickPlay();
void setQuickPlay(int song, int channel, int patternOffset, GTOBJECT *gt, int updateCounter);
int getQuickPlayChannels(int song, int channel, int patternOffset, GTOBJECT *gt, int updateCounter);
int checkFor3ChannelSong();
int patternContainsData(int p);
void clearExpandedSongChannel(int s, int c);
void generateExpandedSongChannel(int s, int c);
void expandAllSongs();
void compressAllSongs();
void compressSong(int s);
int generateCompressedSongChannel(int s, int c, int validateOnly);
void compressChannel(int s, int c, int startIndex, int endIndex, int *p, int *lastTranspose, int validateOnly);
int validateAllSongs();


typedef struct
{
	CHN chn[MAX_PLAY_CH];
	int updateCounter;	// if this doesn't match the undo counter, we need to do a play song from beginning to calculate chn struct info
} CHN_QUICKPLAY;

#endif
