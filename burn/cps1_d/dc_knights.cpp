#include "../burnint.h"
// Knights of the Round - Driver

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

  {"P3 Coin"       , 0, CpsInp177+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp177+7, "p3 start"},
  {"P3 Up"         , 0, CpsInp177+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp177+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp177+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp177+0, "p3 right"},
  {"P3 Attack"     , 0, CpsInp177+4, "p3 fire 1"},
  {"P3 Jump"       , 0, CpsInp177+5, "p3 fire 2"},

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
  if (n>=0xf0) return 0; // To cancel music on startup
  if (n>=0x75) n%=0x75;
  if (n<=0x1e) return 2; // Music
  return 1; // Sound
}

static int DrvFind1(int t)
{
  if (t==0xf020) return -1;
  t<<=6; return t;
}

static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x400000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen=    0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  nRet=BurnLoadRom(CpsRom+0x00000,0,1); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x80000,1,1); if (nRet!=0) return 1;
  // (file already byteswapped)

  // Load graphics roms
  CpsLoadTiles(CpsGfx,2);
  CpsLoadTiles(CpsGfx+0x200000,6);


  // Load Z80 Rom
  BurnLoadRom(CpsZRom,10,1);
  nPsndIrqPeriod=(60<<10)/250; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  BurnLoadRom(CpsAd        ,11,1);
  BurnLoadRom(CpsAd+0x20000,12,1);

  // Extras:
  CpsMult[0]=0x46; CpsMult[1]=0x44; CpsMult[2]=0x42; CpsMult[3]=0x40; // Multiply ports
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  CpsLcReg=0x68; // Layer control register
  CpsPmReg[0]=0x66; CpsPmReg[1]=0x64; CpsPmReg[2]=0x62; CpsPmReg[3]=0x60;
  CpsLayEn[1]=0x20; CpsLayEn[2]=0x10; CpsLayEn[3]=0x02; // Layer enable is different
  // Custom tile finder:
  CpsFind1=DrvFind1;

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
  if (i==0) { if (pszName!=NULL) *pszName="knights.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"kr_23e.rom" ,0x80000,0x1b3997eb,0x10}, // 0
  {"kr_22.rom"  ,0x80000,0xd0b671a9,0x10},

  // graphics:
  {"kr_gfx1.rom",0x80000,0x9e36c1a4,   1}, // 2
  {"kr_gfx3.rom",0x80000,0xc5832cae,   1},
  {"kr_gfx2.rom",0x80000,0xf095be2d,   1},
  {"kr_gfx4.rom",0x80000,0x179dfd96,   1},
  {"kr_gfx5.rom",0x80000,0x1f4298d2,   1},
  {"kr_gfx7.rom",0x80000,0x37fa8751,   1},
  {"kr_gfx6.rom",0x80000,0x0200bc3d,   1},
  {"kr_gfx8.rom",0x80000,0x0bb2b4e7,   1},

  // z80 rom
  {"kr_09.rom"  ,0x10000,0x5e44d9ee,   2}, // 10
  // samples
  {"kr_18.rom"  ,0x20000,0xda69d15f,   2},
  {"kr_19.rom"  ,0x20000,0xbfc654e9,   2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsKnights=
{
  {"knights","Knights of the Round",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,CpsFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
