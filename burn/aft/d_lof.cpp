#include "aft.h"
// Line of Fire - Driver

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
  AftGame=4; // LOF
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
  if (i==0) { if (pName!=NULL) *pName="loffire.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  // cpu 1 code (encrypted unfortunately)
  {"epr12849.rom",0x20000,0         , 0x10}, // even
  {""            ,0      ,0         ,    0},
  {"epr12850.rom",0x20000,0         , 0x10}, // odd
  {""            ,0      ,0         ,    0},
  // cpu 2 code
  {"epr12804.rom",0x20000,0         , 0x10}, // even
  {"epr12802.rom",0x20000,0         , 0x10},
  {"epr12805.rom",0x20000,0         , 0x10}, // odd
  {"epr12803.rom",0x20000,0         , 0x10},

  // tile bitplanes
  {"opr12791.rom",0x10000,0         ,    1},
  {"opr12792.rom",0x10000,0         ,    1},
  {"opr12793.rom",0x10000,0         ,    1},

  // sprite bitmaps
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
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},
  {""            ,0      ,0         ,    0},

  {""            ,0      ,0         ,    0}, // ground bitmap
  {"epr12798.rom",0x10000,0         , 0x12}, // z80 program
  {"epr12800.rom",0x20000,0         ,    2}, // pcm data
  {"epr12799.rom",0x20000,0         ,    2},
  {"epr12801.rom",0x20000,0         ,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvLoffire=
{
  {"loffire" ,"Line of Fire","w.i.p."},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,AftFrame, AftScan,
  NULL,320,224
};
