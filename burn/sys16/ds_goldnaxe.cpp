#include "sys.h"
// System 16 - Golden Axe - Driver

static int DrvInit()
{
  int Ret=0,i=0;
  static unsigned char Bank[]=
  {
    0, 1, 4, 5,
    8, 9, 0, 0,
    2, 3, 6, 7,
   10,11, 0, 0
  };

  BsysRomLen     =0x0c0000;
  BsysTileDataLen=0x080000;
  BsysSprDataLen =0x180000;
  BsysSprBank=Bank;
  Ret=BsysInit(); if (Ret) return 1;

  // Custom banks:
  BsysTileBank=0x10;
  BsysObjBank =0x20;
  BsysPramBank=0x14;

  // Load program roms
  Ret=BurnLoadRom(BsysRom+0x000001,0,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x000000,1,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x080001,2,2); if (Ret) return 1;
  Ret=BurnLoadRom(BsysRom+0x080000,3,2); if (Ret) return 1;

  // Load tile data (three bitfields)
  BsysLoadTiles(BsysTileData,4);

  // Load the roms and interleve by 4
  for (i=0;i<6;i++) BurnLoadRom(BsysSprData+((i&6)<<18)+((i&1)^1),7+i,2);

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
  // Golden Axe protection read in routine at 0x3ca0
  *(unsigned short *)(BsysRam+0x2c1c)=0x0001;
  *(unsigned int   *)(BsysRam+0x2cd8)=0x159d048c;
  *(unsigned int   *)(BsysRam+0x2cdc)=0x37bf26ae;
  return BsysFrame();
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="goldnaxe.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"epr12523.a7",0x20000,0x8e6128d7,0x10}, //  0 even 68000 code at 0x000000
  {"epr12522.a5",0x20000,0xb6c35160,0x10}, //  1 odd
  {"epr12521.a8",0x20000,0x5001d713,0x10}, //  2 program even at 0x080000?
  {"epr12519.a6",0x20000,0x4438ca8e,0x10}, //  3 odd
  {"epr12385"   ,0x20000,0xb8a4e7e0,0x10}, //  4 tile data 
  {"epr12386"   ,0x20000,0x25d7d779,0x10}, //  5
  {"epr12387"   ,0x20000,0xc7fcadf3,0x10}, //  6

  {"mpr12378.b1",0x40000,0x119e5a82,0x10}, //  7 sprite data
  {"mpr12379.b4",0x40000,0x1a0e8c57,0x10}, //  8
  {"mpr12380.b2",0x40000,0xbb2c0853,0x10}, //  9
  {"mpr12381.b5",0x40000,0x81ba6ecc,0x10}, // 10
  {"mpr12382.b3",0x40000,0x81601c6f,0x10}, // 11
  {"mpr12383.b6",0x40000,0x5dbacf7a,0x10}, // 12
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvSysGoldnaxe=
{
  {"goldnaxe","Golden Axe","Incomplete"},
  DrvZipName,DrvRomInfo,DrvRomName,BsysInputInfo,
  DrvInit,DrvExit,DrvFrame,NULL,
  NULL,320,224
};
