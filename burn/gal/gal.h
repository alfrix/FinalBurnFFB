// Galaxy Force ----------------------------------
#include "../sys16/sys.h"

// gal_rw.cpp
extern unsigned char GalGame;
extern unsigned char GalDip[2];
extern unsigned char GalReset;
extern unsigned int GalCounter;
int GalFrame();
int GalInit();
int GalExit();

// gal_line.cpp
extern unsigned char *GalLineSrc; // Original line sprite table
extern unsigned char *GalLineData; // Line Sprite bitmap data
extern int GalLineDataLen; // Length of above
int GalLineDrawSprite(int i,unsigned char *ps);
int GalLineDraw();

// gal_mem.cpp
extern unsigned char *GalRomM, *GalRomX, *GalRomY;
extern unsigned char *GalRamMFF,*GalRamXFF,*GalRamYFF;
extern unsigned char *GalRam0C, *GalRamM180, *GalRamM188;
extern unsigned char *GalRamM19, *GalRamX18;
extern unsigned char *GalPcm; extern int GalPcmLen;
int GalMemInit();
int GalMemExit();
int GalScan(int nAction,int *pnMin);

// gal_rw.cpp
extern unsigned char GalButton[8];
extern unsigned char GalInput,GalGear,GalLastGear;
#define GAL_ANALOG_COUNT (6)
extern unsigned char GalAnaVal[GAL_ANALOG_COUNT];
extern unsigned char GalAnalog[GAL_ANALOG_COUNT];
void GalInputPrepare();
void GalInputMemory();
int GalCpuInit();

// gal_snd.cpp
void GalSndCode(unsigned char Code);
int GalSndInit();
int GalSndExit();

// gal_spr.cpp
extern unsigned char *GalSprSrc; // Original sprite table
extern unsigned char *GalSprData; // Normal Sprite bitmap data
extern int GalSprDataLen; // Length of above
int GalSprDraw();
