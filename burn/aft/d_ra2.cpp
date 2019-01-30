#include "aft.h"
// Royal Ascot 2 - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"Coin 1"     , 0, AftButton+6, "p1 coin"},
  {"Start"      , 0, AftButton+3, "p1 start"},
  {"Left/Right" , 1, AftInput+3 , "p1 x-axis"},
  {"Up/Down"    , 1, AftInput+4 , "p1 y-axis"},
  {"Fire 1"     , 0, AftButton+4, "p1 fire 1"},
  {"Fire 2"     , 0, AftButton+5, "p1 fire 2"},
  {"Throttle"   , 1, AftInput+5 , "p1 z-axis"},
  {"Fast"       , 0, AftButton+8, "p1 fire 3"},
  {"Slow"       , 0, AftButton+9, "p1 fire 4"},

  {"Coin 2"     , 0, AftButton+7, "p2 coin"},

  {"Reset"      , 0, &AftReset,   "reset"},
  {"Service"    , 0, AftButton+2, "service"},
  {"Diagnostic" , 0, AftButton+1, "diag"},
  {"Dip A"      , 2, AftInput+1 , "dip"},
  {"Dip B"      , 2, AftInput+2 , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvInit()
{
  AftGame=0;
  AftInput[1]=0x00; // Dip A
  AftInput[2]=0x00; // Dip B
  return AftInit();
}

static int DrvExit()
{
  AftExit();
  AftGame=0;
  AftInput[1]=0;
  AftInput[2]=0;
  return 0;
}

static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="royalascot2.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  // cpu 1 code
  {"ep13965a"    ,0x20000,0x7eacdfb3, 0x10}, // even (rom name typo - 96->69?)
  {""            ,0      ,0         ,    0},
  {"ep13694a"    ,0x20000,0x15b86498, 0x10}, // odd
  {""            ,0      ,0         ,    0},

  // cpu 2 code
  {"epr13967"    ,0x20000,0x3b92e2b8, 0x10}, // even
  {""            ,0      ,0         ,    0},
  {"epr13966"    ,0x20000,0xeaa644e1, 0x10}, // odd
  {""            ,0      ,0         ,    0},

  // tile bitplanes
  {"epr13961"    ,0x10000,0x68038629,    1},
  {"epr13962"    ,0x10000,0x7d7605bc,    1},
  {"epr13963"    ,0x10000,0xf3376b65,    1},

  // sprite bitmaps
  {"epr13957"    ,0x20000,0x6d50fb54,    1},
  {"epr13958"    ,0x20000,0x7803a027,    1},
  {"epr13959"    ,0x20000,0xdb245b22,    1},
  {"epr13960"    ,0x20000,0xb974128d,    1},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},

  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},

  {""            ,0      ,0         ,    0}, // ground bitmap
  {"ep14221A"    ,0x10000,0x0d429ac4, 0x12}, // z80 program
  {""            ,0      ,0         ,    0}, // pcm data
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvRoyalAscot2=
{
  {"royalascot2","Royal Ascot 2","w.i.p."},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,AftFrame, AftScan,
  NULL,320,224
};
