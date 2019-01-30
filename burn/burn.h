//---------------------------------------------------------------------------
// Burn - Arcade emulator library
// Copyright (c) 2002 Dave (www.finalburn.com), all rights reserved.

// This refers to all the code except where stated otherwise
// (e.g. 68000/ym2151)

// You can use, modify and redistribute this code freely as long as you
// don't do so commercially. This copyright notice must remain with the code.
// If your program uses this code, you must either distribute or link to the
// source code. If you modify or improve this code, you must distribute
// the source code improvements.

// Dave
// Homepage: www.finalburn.com
// E-mail:  dave@finalburn.com
//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

struct BurnRomInfo { int nLen; unsigned int nCrc; int nType; };
struct BurnInputInfo { char *szName; unsigned char nType; unsigned char *pVal;  char *szInfo;};
struct BurnArea { void *Data; int nLen; char *szName; };

extern int nBurnVer; // Version number of the library

// burn.cpp
extern int (__cdecl *dprintf) (char *szFormat,...);
extern unsigned int nBurnDrvCount; // Count of game drivers
extern unsigned int nBurnDrvSelect; // Which game driver is selected

extern unsigned char *pBurnDraw; // Pointer to correctly sized bitmap
extern int nBurnPitch; // Pitch between each line
extern int nBurnBpp;   // Bytes per pixel
extern int nBurnSoundChoice; // &1 play sound effects &2=play music
extern unsigned char nBurnLayer; // Can be used externally to select which layers to show

int BurnLibInit();
int BurnLibExit();
extern int (*BurnExtInput) (unsigned int i); // App-defined func to get the state of each input
char *BurnDrvText(unsigned int i);
int BurnDrvGetZipName(char **pszName,unsigned int i);
int BurnDrvGetRomInfo(struct BurnRomInfo *pri,unsigned int i);
int BurnDrvGetRomName(char **pszName,unsigned int i,int nAka);
int BurnDrvGetInputInfo(struct BurnInputInfo *pii,unsigned int i);
int BurnDrvGetScreen(int *pnWidth,int *pnHeight);
int BurnDrvInit();
int BurnDrvExit();
int BurnDrvFrame();
int BurnRecalcPal();
extern unsigned int (*BurnHighCol) (int r,int g,int b,int i);
int BurnAreaScan(int nAction,int *pnMin); // &1=for reading &2=for writing &4=Volatile &8=Non-Volatile
extern int (*BurnAcb) (struct BurnArea *pba); // Area callback

// load.cpp
// Application-defined rom loading function:
extern int (*BurnExtLoadRom)(unsigned char *Dest,int *pnWrote,int i);

// sound.cpp
extern int nBurnSoundRate;   // sample rate of sound
extern int nBurnSoundLen;    // length in samples per frame
extern short *pBurnSoundOut; // pointer to output buffer

#ifdef __cplusplus
} // End of extern "C"
#endif
