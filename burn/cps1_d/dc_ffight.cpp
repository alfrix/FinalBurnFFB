#include "../burnint.h"
// Final Fight - Driver

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

  {"Reset"         , 0, &CpsReset,   "reset"},
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
  if (n>=0x40 && n<0x58) return 2;
  if (n>=0xa0 && n<0xb8) return 2;
  return 1;
}

static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x200000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen=    0x040000;
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
  nRet=BurnLoadRom(CpsZRom,9,1);
  nPsndIrqPeriod=(60<<10)/250;
  BurnScode=DrvScode;

  // Load ADPCM data
  nRet=BurnLoadRom(CpsAd        ,10,1);
  nRet=BurnLoadRom(CpsAd+0x20000,11,1);

  // Extras:
  CpsId[0]=0x60; CpsId[1]=0x0004; // Board ID
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  CpsLcReg=0x6e; // Layer control register is at 0x6e
  CpsPmReg[0]=0x66; CpsPmReg[1]=0x70; CpsPmReg[2]=0x68; CpsPmReg[3]=0x72;
  
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
  if (i==0) { if (pszName!=NULL) *pszName="ffight.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"ff30-36.bin" ,0x20000,0xf9a5ce83,0x10}, // 0 even 68000 code
  {"ff35-42.bin" ,0x20000,0x65f11215,0x10}, // 1 odd
  {"ff31-37.bin" ,0x20000,0xe1033784,0x10}, // 2 even
  {"ff36-43.bin" ,0x20000,0x995e968a,0x10}, // 3 odd
  {"ff32-32m.bin",0x80000,0xc747696e,0x10}, // 4 both

  // graphics:
  {"ff05-05m.bin",0x80000,0x9c284108,   1}, // 5
  {"ff07-07m.bin",0x80000,0xa7584dfb,   1},
  {"ff01-01m.bin",0x80000,0x0b605e44,   1},
  {"ff03-03m.bin",0x80000,0x52291cd2,   1},

  // z80 rom
  {"ff09-09.bin" ,0x10000,0xb8367eb5,   2}, // 9
  // samples
  {"ff18-18.bin" ,0x20000,0x375c66e7,   2},
  {"ff19-19.bin" ,0x20000,0x1ef137f9,   2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsFfight=
{
  {"ffight","Final Fight",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,CpsFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
