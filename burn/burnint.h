// Burn - Arcade emulator library - internal code

// Standard headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "burn.h"

// Make the INLINE macro
#undef INLINE
#define INLINE inline
#pragma warning (disable:4514) //disable "unreferenced inline function removed" warning message
#pragma warning (disable:4710) //disable "inline not inlined" warning message

// Chip emulators ------------------------------------------------

// sek.cpp
#include "sek/sek.h"

// zet.cpp
#include "zet.h"

// ym2151 interface:
extern "C" {
int YM2151Init(int num,int clock,int rate);
void YM2151Shutdown();
void YM2151UpdateOne(int num,short **buffers,int length);
void YM2151WriteReg(int n,int r,int v);
}

// --------------------------------------

struct BurnDriver
{
  char *szText[3];
  // [0] = Short name for the driver
  // [1] = Full name of the game the driver is for
  // [2] = Notes
  int (*GetZipName)(char **pszName,unsigned int i);  // Function to get possible zip names
  int (*GetRomInfo)(struct BurnRomInfo *pri,unsigned int i);  // Function to get the length and crc of each rom
  int (*GetRomName)(char **pszName,unsigned int i,int nAka);  // Function to get the possible names for each rom
  int (*GetInputInfo)(struct BurnInputInfo *pii,unsigned int i); // Function to get the input info for the game
  int (*Init)(); int (*Exit)(); int (*Frame)(); int (*AreaScan)(int nAction,int *pnMin);
  unsigned char *pRecalcPal; int nWidth,nHeight; // Screen width and height
};

// burn.cpp
// Scan a small variable or structure
static INLINE void ScanVar(void *pv,int nSize,char *szName)
{
  struct BurnArea ba;
  memset(&ba,0,sizeof(ba));
  ba.Data=pv; ba.nLen=nSize; ba.szName=szName;
  BurnAcb(&ba);
}

#define SCAN_VAR(x) ScanVar(&x,sizeof(x),#x);

int BurnClearSize(int w,int h);
int BurnClearScreen();
static INLINE void PutPix(unsigned char *pPix,unsigned int c)
{
       if (nBurnBpp>=4) *((unsigned int *)pPix)=c;
  else if (nBurnBpp>=3)
  {
    pPix[0]=(unsigned char) c;
    pPix[1]=(unsigned char)(c>>8);
    pPix[2]=(unsigned char)(c>>16);
  }
  else if (nBurnBpp>=2) *((unsigned short *)pPix)=(unsigned short)c;
  else *pPix=(unsigned char)c;
}

// Fill a line with a color
static void INLINE BurnFillLine(unsigned char *pPix,unsigned int c)
{
  unsigned char *pEnd; // Pointer to the end of the line
  pEnd=pPix+(nBurnBpp<<8)+(nBurnBpp<<6); // 256+64=320

  // Fill in the correct color depth:  
       if (nBurnBpp==2) { do { *((unsigned short *)pPix)=(unsigned short)c; pPix+=2; } while (pPix<pEnd); }
  else if (nBurnBpp==3)
  {
    do
    {
      *pPix++=(unsigned char) c;
      *pPix++=(unsigned char)(c>>8);
      *pPix++=(unsigned char)(c>>16);
    }
    while (pPix<pEnd);
  }
  else if (nBurnBpp==4) { do { *((unsigned int *)pPix)=c; pPix+=4; } while (pPix<pEnd); }
  else { do { *pPix++=(unsigned char)c; } while (pPix<pEnd); }
}

int BurnByteswap(unsigned char *pm,int nLen);
extern int (*BurnScode) (unsigned int nCode);
int BurnScodePass(unsigned int n);

// Standard rom functions
#include "stdrom.h"

// load.cpp
int BurnLoadRom(unsigned char *Dest,int i,int nGap);
int BurnXorRom(unsigned char *Dest,int i,int nGap);
int BurnLoadBitField(unsigned char *pDest,unsigned char *pSrc,int nField,int nSrcLen);

// CPS -------------------------------------------------
#include "cps.h"

// TMNT ------------------------------------------------
// tmnt.cpp
extern unsigned char bTmntRecalcPal;
extern unsigned char *TmntRam08,*TmntRam10,*TmntTile;
int TmntInit();
int TmntExit();
int TmntFrame();
int TmntScan(int nAction,int *pnMin);
// tmnt_pal.cpp
extern unsigned int TmntPal[0x400];
int TmntPalInit();
int TmntPalExit();
void TmntPalWrite(unsigned int a,unsigned char d);
int TmntPalUpdate(int bRecalc);
// tmnt_til.cpp
int TmntTileDraw();
// tmnt_inp.cpp
extern unsigned char TmntAoo[0x10]; // values to return from A0000
extern unsigned char TmntCoin[4],TmntStart[4];
int TmntInpMake();
