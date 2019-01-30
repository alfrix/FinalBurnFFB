#include "sys.h"
// System 16 - Shinobi - Driver

static int DrvInit()
{
  int Ret=0,i=0;
  static unsigned char Bank[]=
  {
    0, 0, 0, 0,
    0, 0, 0, 3,
    0, 0, 0, 2,
    0, 1, 0, 0
  };

  BsysRomLen     =0x040000;
  BsysTileDataLen=0x040000;
  BsysSprDataLen =0x080000;
  BsysSprBank=Bank;
  Ret=BsysInit(); if (Ret) return 1;

  // ------------  Tiles  -------------------------------------------
  // Load program roms
  Ret=BurnLoadRom(BsysRom+0x000001,0,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x000000,1,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x020001,2,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x020000,3,2); if (Ret) return 1;

  // Load tile data (three bitfields)
  BsysLoadTiles(BsysTileData,4);

  // Load the roms and interleve by 4
  for (i=0;i<8;i++) BurnLoadRom(BsysSprData+((i&6)<<16)+((i&1)^1),7+i,2);

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
  if (i==0) { if (pName!=NULL) *pName="shinobi.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"shinobi.a4" ,0x10000,0xb930399d,0x10}, //  0 even 68000
  {"shinobi.a1" ,0x10000,0x343f4c46,0x10}, //  1 odd
  {"epr11283"   ,0x10000,0x9d46e707,0x10}, //  2 even 68000
  {"epr11281"   ,0x10000,0x7961d07e,0x10}, //  3 odd
  {"shinobi.b9" ,0x10000,0x5f62e163,   1}, //  4 tile data
  {"shinobi.b10",0x10000,0x75f8fbc9,   1}, //  5
  {"shinobi.b11",0x10000,0x06508bb9,   1}, //  6

  {"epr11290.10",0x10000,0x611f413a,   1}, //  7 sprite data
  {"epr11294.11",0x10000,0x5eb00fc1,   1}, //  8
  {"epr11291.17",0x10000,0x3c0797c0,   1}, //  9
  {"epr11295.18",0x10000,0x25307ef8,   1}, // 10
  {"epr11292.23",0x10000,0xc29ac34e,   1}, // 11
  {"epr11296.24",0x10000,0x04a437f8,   1}, // 12
  {"epr11293.29",0x10000,0x41f41063,   1}, // 13
  {"epr11297.30",0x10000,0xb6e1fd72,   1}, // 14
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvSysShinobi=
{
  {"shinobi","Shinobi","Incomplete"},
  DrvZipName,DrvRomInfo,DrvRomName,BsysInputInfo,
  DrvInit,DrvExit,DrvFrame,NULL,
  NULL,320,224
};
