#include "sys.h"
// System 16 - Altered Beast - Driver

static int DrvInit()
{
  int Ret=0,i=0;
  BsysRomLen     =0x040000;
  BsysTileDataLen=0x080000;
  BsysSprDataLen =0x100000;
  Ret=BsysInit(); if (Ret) return 1;

  // Load program roms
  Ret=BurnLoadRom(BsysRom+0x000001,0,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x000000,1,2); if (Ret) return 1;

  // Load tile data (three bitfields)
  BsysLoadTiles(BsysTileData,2);

  // Load the roms and interleve by 4
  for (i=0;i<8;i++) BurnLoadRom(BsysSprData+((i&6)<<17)+((i&1)^1),5+i,2);

  Ret=BsysRunInit(); if (Ret) return 1;
  return 0;
}

static int DrvExit()
{
  BsysRunExit();
  BsysExit();
  return 0;
}

static int DrvFrame()
{
  return BsysFrame();
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="altbeast.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"11705"      ,0x20000,0x57dc5c7a,0x10}, //  0 even 68000 code
  {"11704"      ,0x20000,0x33bbcf07,0x10}, //  1 odd
  {"11674"      ,0x20000,0xa57a66d5,   1}, //  2 tile data
  {"11675"      ,0x20000,0x2ef2f144,   1}, //  3
  {"11676"      ,0x20000,0x0c04acac,   1}, //  4

  {"epr11677.b1",0x20000,0xa01425cd,   1}, //  5 sprite data
  {"epr11681.b5",0x20000,0xd9e03363,   1}, //  6
  {"epr11678.b2",0x20000,0x17a9fc53,   1}, //  7
  {"epr11682.b6",0x20000,0xe3f77c5e,   1}, //  8
  {"epr11679.b3",0x20000,0x14dcc245,   1}, //  9
  {"epr11683.b7",0x20000,0xf9a60f06,   1}, // 10
  {"epr11680.b4",0x20000,0xf43dcdec,   1}, // 11
  {"epr11684.b8",0x20000,0xb20c0edb,   1}, // 12
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvSysAltbeast=
{
  {"altbeast","Altered Beast","Incomplete"},
  DrvZipName,DrvRomInfo,DrvRomName,BsysInputInfo,
  DrvInit,DrvExit,DrvFrame,NULL,
  NULL,320,224
};
