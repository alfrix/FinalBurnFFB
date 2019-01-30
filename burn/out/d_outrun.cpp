#include "out.h"
// Out Run - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"Coin 1"     ,0, OutButton+6, "p1 coin"},
  {"Start"      ,0, OutButton+3, "p1 start"},
  {"Left/Right" ,1, OutAxis+0  , "p1 x-axis"},
  {"Accel"      ,1, OutAxis+1  , "p1 fire 1"},
  {"Change Gear",0, &OutGear,    "p1 fire 2"},
  {"Brake"      ,1, OutAxis+2  , "p1 fire 3"},
  {"Coin 2"     ,0, OutButton+7, "p2 coin"},
  {"Reset"      ,0, &OutReset,   "reset"},
  {"Diagnostic" ,0, OutButton+1, "diag"},
  {"Dip A"      ,2, OutDip+0,    "dip"},
  {"Dip B"      ,2, OutDip+1,    "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="outrun.zip"; return 0; }
  return 1;
}


// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"10380a",0x10000,0x434fadbc, 0x10}, //  0 cpu 1 code (even)
  {"10381a",0x10000,0xbe8c412b, 0x10}, //  1
  {"10382a",0x10000,0x1ddcc04e, 0x10}, //  2            (odd)
  {"10383a",0x10000,0xdcc586e7, 0x10}, //  3
  {"10327a",0x10000,0xe28a5baf, 0x10}, //  4 cpu 2 code (even)
  {"10328a",0x10000,0xd5ec5e5d, 0x10}, //  5
  {"10329a",0x10000,0xda131c81, 0x10}, //  6            (odd)
  {"10330a",0x10000,0xba9ec82a, 0x10}, //  7
  {"10268" ,0x08000,0x95344b04,    1}, //  8 tile data
  {"10267" ,0x08000,0xa85bb823,    1}, //  9
  {"10266" ,0x08000,0x9f6f1a74,    1}, // 10
  {"10232" ,0x08000,0x776ba1eb,    1}, // 11
  {"10231" ,0x08000,0x8908bcbf,    1}, // 12
  {"10230" ,0x08000,0x686f5e50,    1}, // 13
  {"10377" ,0x20000,0xc86daecb,    1}, // 14 sprite data
  {"10375" ,0x20000,0x62a472bd,    1}, // 15
  {"10373" ,0x20000,0x339f8e64,    1}, // 16
  {"10371" ,0x20000,0x0a1c98de,    1}, // 17
  {"10378" ,0x20000,0x544068fd,    1}, // 18
  {"10376" ,0x20000,0x8337ace7,    1}, // 19
  {"10374" ,0x20000,0x22744340,    1}, // 20
  {"10372" ,0x20000,0x1640ad1f,    1}, // 21
  {"10185" ,0x08000,0x22794426,    1}, // 22 ground data

  {"10187" ,0x08000,0xa10abaa9,    2}, // 23 z80 program
  {"10193" ,0x08000,0xbcd10dde,    2}, // 24 pcm
  {"10192" ,0x08000,0x770f1270,    2}, // 25
  {"10191" ,0x08000,0x20a284ab,    2}, // 26
  {"10190" ,0x08000,0x7cab70e2,    2}, // 27
  {"10189" ,0x08000,0x01366b54,    2}, // 28
  {"10188" ,0x08000,0xbad30ad9,    2}  // 29
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvOutrun=
{
  {"outrun","Out Run","Some gfx missing"},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  OutInit,OutExit,OutFrame,OutScan,NULL,320,224
};
