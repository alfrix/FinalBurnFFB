#include "../burnint.h"
// Strider - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Attack"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Jump"       , 0, CpsInp001+5, "p1 fire 2"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Attack"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},

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
  if (n<=0x2f) return 2; // music
  if (n<=0x77) return 1; // effects
  if (n<=0x7f) return 2; // music mirror
  if (n<=0xef) return 1; // speech
  return 0; // control codes
}

static int DrvFind1(int t) { if (t==0x0020) return -1; return (t+0x8000)<<6; }
static int DrvFind3(int t) { return (t+0x1000)<<9; }

static int Rom8=0;
static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x400000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen  =  0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  if (Rom8)
  {
    // 0x80000 length rom
    nRet=BurnLoadRom(CpsRom,0,1); if (nRet!=0) return 1; // Already byteswapped
  }
  else
  {
    // 4*0x20000 length roms
    nRet=BurnLoadRom(CpsRom+0x000001,0,2); if (nRet!=0) return 1;
    nRet=BurnLoadRom(CpsRom+0x000000,1,2); if (nRet!=0) return 1;
    nRet=BurnLoadRom(CpsRom+0x040001,2,2); if (nRet!=0) return 1;
    nRet=BurnLoadRom(CpsRom+0x040000,3,2); if (nRet!=0) return 1;
  }
  nRet=BurnLoadRom(CpsRom+0x080000,4,1); if (nRet!=0) return 1; // Already byteswapped

  // Load graphics roms
  CpsLoadTiles(CpsGfx         ,5);
  CpsLoadTiles(CpsGfx+0x200000,9);

  // Load Z80 Rom
  nRet=BurnLoadRom(CpsZRom,13,1);
  nPsndIrqPeriod=(60<<10)/500; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  nRet=BurnLoadRom(CpsAd        ,14,1);
  nRet=BurnLoadRom(CpsAd+0x20000,15,1);

  // Extras:  
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

static int DrvFrame()
{
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="strider.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"strider.30"  ,0x20000,0xda997474,0x10}, // 0 even 68000 code
  {"strider.35"  ,0x20000,0x5463aaa3,0x10}, // 1 odd
  {"strider.31"  ,0x20000,0xd20786db,0x10}, // 2 even
  {"strider.36"  ,0x20000,0x21aa2863,0x10}, // 3 odd
  {"strider.32"  ,0x80000,0x9b3cfc08,0x10}, // 4 both

  // graphics:
  {"strider.06"  ,0x80000,0x4eee9aea,   1}, // 5
  {"strider.08"  ,0x80000,0x2d7f21e4,   1},
  {"strider.02"  ,0x80000,0x7705aa46,   1},
  {"strider.04"  ,0x80000,0x5b18b722,   1},
  {"strider.05"  ,0x80000,0x005f000b,   1}, // 9
  {"strider.07"  ,0x80000,0xb9441519,   1},
  {"strider.01"  ,0x80000,0xb7d04e8b,   1},
  {"strider.03"  ,0x80000,0x6b4713b4,   1},

  // z80 rom
  {"strider.09"  ,0x10000,0x08d63519,   2}, // 13
  // samples
  {"strider.18"  ,0x20000,0x4386bc80,   2},
  {"strider.19"  ,0x20000,0x444536d7,   2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsStrider=
{
  {"strider","Strider",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// --------------------------------- Japanese Version -----------------------------------
// Rom information
static struct StdRomInfo JpnRomDesc[]=
{
  {"sthj23.bin"  ,0x80000,0x046e7b12,0x10}, // 0 both
  {""            ,0      ,0         ,   0},
  {""            ,0      ,0         ,   0},
  {""            ,0      ,0         ,   0}
};

// Make The RomInfo/Name functions for the game
static struct StdRomInfo *JpnPickRom(unsigned int i)
{
  if (i<4) return JpnRomDesc+i;
  return DrvPickRom(i);
}

STD_ROM_FN(Jpn)

static int JpnInit()
{
  int Ret=0;
  Rom8=1; Ret=DrvInit(); Rom8=0;
  return Ret;
}

struct BurnDriver BurnDrvCpsStriderj=
{
  {"striderj","Strider Hiryu (Jpn)",""},
  DrvZipName,JpnRomInfo,JpnRomName,DrvInputInfo,
  JpnInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// --------------------------------------------------------------------------------------
