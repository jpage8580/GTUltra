#ifndef GSOUND_H
#define GSOUND_H

#ifdef GSOUND_C

#ifdef __WIN32__
#include <winioctl.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "cwsid.h"
#endif

#endif

#define MINMIXRATE 11025
#define MAXMIXRATE 48000
#define MINBUF 20
#define MAXBUF 2000
#define DEFAULTBUF 100
#define DEFAULTMIXRATE 44100

#define PALFRAMERATE 50
#define PALCLOCKRATE 985248
#define NTSCFRAMERATE 60
#define NTSCCLOCKRATE 1022727

#define MIXBUFFERSIZE 65536	//16384	//65536

extern int bypassPlayRoutine;
extern int largestExportValue;

int sound_init(unsigned b, unsigned mr, unsigned writer, unsigned hardsid, unsigned m, unsigned ntsc, unsigned multiplier, unsigned catweasel, unsigned interpolate, unsigned customclockrate);
void sound_uninit(void);
void sound_suspend(void);
void sound_flush(void);

void ExportCloseFileHandle();
void GenerateExportFileName();
void ExportSIDToPCMFile(int samples,int doNormalize);
void OpenExportFileNameForWriting();
void convertRAWToWAV(int doNormalize);

#endif
