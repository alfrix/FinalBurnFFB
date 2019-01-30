#include "../burnint.h"
// U.N. Squadron - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Shot A"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Shot B"     , 0, CpsInp001+5, "p1 fire 2"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Shot A"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Shot B"     , 0, CpsInp000+5, "p2 fire 2"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp018+6, "diag"},
  {"Service"       , 0, CpsInp018+2, "service"},

  {"Dip A"         , 2, &Cpi01A    , "dip"},
  {"Dip B"         , 2, &Cpi01C    , "dip"},
  {"Dip C"         , 2, &Cpi01E    , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n)
{
  if (n<0x19) return 2; if (n<0x1a) return 1;
  if (n<0x20) return 2; if (n<0x42) return 1;
  if (n<0x5b) return 2; if (n<0x5c) return 1;
  if (n<0x62) return 2; if (n<0xf0) return 1;
  return 0;
}

static int DrvFind1(int t) { if (t==0x20) return -1; return t<<6; }
static int DrvFind3(int t) { if (t==0) return -1; return t<<9; }

static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x200000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen=    0x020000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  nRet=BurnLoadRom(CpsRom+0x000001,0,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x000000,1,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040001,2,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040000,3,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x080000,4,1); if (nRet!=0) return 1; // Already byteswapped

  // Load graphics roms
  CpsLoadTiles(CpsGfx,5);

  // Load Z80 Rom
  BurnLoadRom(CpsZRom,9,1);
  nPsndIrqPeriod=(60<<10)/500; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  BurnLoadRom(CpsAd,10,1);

  // Extras:
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  CpsLayEn[1]=0x20; CpsLayEn[2]=0x10; CpsLayEn[3]=0x08; // Layer enable

  // Custom tile finder:
  CpsFind1=DrvFind1; CpsFind3=DrvFind3;

  // Ready to go
  return CpsRunInit();
}

static int DrvExit()
{
  CpsRunExit(); CpsExit();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="unsquad.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"unsquad.30" ,0x20000,0x24d8f88d,0x10}, // 0 even 68000 code
  {"unsquad.35" ,0x20000,0x8b954b59,0x10}, // 1 odd
  {"unsquad.31" ,0x20000,0x33e9694b,0x10}, //
  {"unsquad.36" ,0x20000,0x7cc8fb9e,0x10}, //
  {"unsquad.32" ,0x80000,0xae1d7fb0,0x10}, // 4 both

  // graphics:
  {"unsquad.05" ,0x80000,0xbf4575d8,   1}, // 5
  {"unsquad.07" ,0x80000,0xa02945f4,   1},
  {"unsquad.01" ,0x80000,0x5965ca8d,   1},
  {"unsquad.03" ,0x80000,0xac6db17d,   1},

  // z80 rom
  {"unsquad.09" ,0x10000,0xf3dd1367,   2}, // 9
  // samples
  {"unsquad.18" ,0x20000,0x584b43a9,   2}
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

static int DrvFrame()
{
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

struct BurnDriver BurnDrvCpsUnsquad=
{
  {"unsquad","U.N. Squadron",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
