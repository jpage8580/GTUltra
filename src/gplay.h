#ifndef GPLAY_H
#define GPLAY_H

#define PLAY_PLAYING 0x00
#define PLAY_BEGINNING 0x01
#define PLAY_POS 0x02
#define PLAY_PATTERN 0x03
#define PLAY_STOP 0x04
#define PLAY_STOPPED 0x80

typedef struct
{
	int espos;	//[MAX_CHN];
	int esend;	//[MAX_CHN];
	int epnum;	//int epnum[MAX_CHN];
}CHN_EDITOR_INFO;

typedef struct
{
	unsigned char requestKeyOff;

  unsigned char trans;
  unsigned char instr;
  unsigned char note;
  unsigned char lastnote;  
  unsigned char newnote;
  unsigned pattptr;
  unsigned char pattnum;
  unsigned char songptr;
  unsigned char songLoopPtr;
  unsigned char repeat;
  unsigned short freq;
  unsigned char gate;
  unsigned char wave;
  unsigned short pulse;
  unsigned char ptr[2];
  unsigned char pulsetime;
  unsigned char wavetime;
  unsigned char vibtime;
  unsigned char vibdelay;
  unsigned char command;
  unsigned char cmddata;
  unsigned char newcommand;
  unsigned char newcmddata;
  unsigned char tick;
  unsigned char tempo;
  unsigned char mute;
  unsigned char advance;
  unsigned char gatetimer;
  unsigned char loopCount;

  int releaseTime;

  int portCounter;

  unsigned  lastpattptr;
  unsigned char lastsongptr;
  unsigned char lastpattnum;

} CHN;



typedef struct
{
	unsigned char filterctrl;	// = 0;
	unsigned char filtertype;	// = 0;
	unsigned char filtercutoff;	// = 0;
	unsigned char filtertime;	// = 0;
	unsigned char filterptr;	// = 0;
}FILTERINFO;

typedef struct 
{
	char *sidreg[4];
	FILTERINFO filterInfo[4];	
	unsigned char funktable[2];
	unsigned char masterfader;	// = 0x0f;

	int psnum;
	int startpattpos;
	int songinit;
	int lastsonginit;
	CHN chn[MAX_PLAY_CH];
	CHN_EDITOR_INFO editorInfo[MAX_PLAY_CH];


	// Loop info
	/*
	Generate start / end positions based on checking the editing channel (eg. When we reach the end of the editing channel, we finish)
	We then compare each chn[n] with loopEndChn[n] info in play.c and memcopy the loopStartChn[n] if they match
	*/
	CHN loopStartChn[MAX_PLAY_CH];
	CHN loopEndChn[MAX_PLAY_CH];
	CHN patternLoopStartChn[MAX_PLAY_CH];
	CHN patternLoopEndChn[MAX_PLAY_CH];
	CHN tempPlayPatternChn[MAX_PLAY_CH];
	int masterLoopChannel;	// This is the channel (0-12) that is checked to know if there's a loop END. If so, all channels reset to the correct position
	int loopEnabledFlag;
	int interPatternLoopEnabledFlag;
	int disableLoopSearch;
	int debug;

	int looptimemin;
	int looptimesec;
	int looptimeframe;

	int timemin;
	int timesec;
	int timeframe;

	int totalMin;
	int totalSec;
	int totalFrame;


	unsigned char controlEditor;	// eg. allow for editor to stop following 
	int noSIDWrites;

} GTOBJECT;

#ifndef GPLAY_C
extern GTOBJECT gtObject;
extern GTOBJECT gtEditorObject;
extern GTOBJECT gtLoopObject;
extern GTOBJECT gtEditorLoopObject;

extern int jtest;
extern CHN chn[MAX_PLAY_CH];
extern unsigned char masterfader;
extern unsigned char freqtbllo[];
extern unsigned char freqtblhi[];
extern int lastsonginit;
#endif


void initchannels(GTOBJECT *gt);
void initsong(int num, int mode, GTOBJECT *gt);
//void initsong(int num, int playmode);
void initsongpos(int num, int playmode, int pattpos, GTOBJECT *gt);
void stopsong(GTOBJECT *gt);
void rewindsong(GTOBJECT *gt);
void playtestnote(int note, int ins, int chnnum, GTOBJECT *gt);
void releasenote(int chnnum, GTOBJECT *gt);
void mutechannel(int chnnum, GTOBJECT *gt);
int isplaying(GTOBJECT *gt);
void playroutine(GTOBJECT *gt);

int getActualSongNumber(int currentSong, int channel);
int getActualChannel(int currentSong, int channel);
void initSID(GTOBJECT *gt);
int getFilterOnOff(GTOBJECT *gt, int ch);
int getFilterType(GTOBJECT *gt, int ch);
int getFilterResonance(GTOBJECT *gt, int ch);
int getFilterCutoff(GTOBJECT *gt, int ch);

int getClosestNote(int freq);
#endif
