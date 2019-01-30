#include "../burnint.h"
// Street Fighter 2 CE/Turbo - Driver

static unsigned char nCpuSpeed=7;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Low Punch"  , 0, CpsInp001+4, "p1 fire 4"},
  {"P1 Mid Punch"  , 0, CpsInp001+5, "p1 fire 5"},
  {"P1 High Punch" , 0, CpsInp001+6, "p1 fire 6"},
  {"P1 Low Kick"   , 0, CpsInp177+0, "p1 fire 1"},
  {"P1 Mid Kick"   , 0, CpsInp177+1, "p1 fire 2"},
  {"P1 High Kick"  , 0, CpsInp177+2, "p1 fire 3"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Low Punch"  , 0, CpsInp000+4, "p2 fire 4"},
  {"P2 Mid Punch"  , 0, CpsInp000+5, "p2 fire 5"},
  {"P2 High Punch" , 0, CpsInp000+6, "p2 fire 6"},
  {"P2 Low Kick"   , 0, CpsInp177+4, "p2 fire 1"},
  {"P2 Mid Kick"   , 0, CpsInp177+5, "p2 fire 2"},
  {"P2 High Kick"  , 0, CpsInp177+6, "p2 fire 3"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp018+6, "diag"},
  {"Service"       , 0, CpsInp018+2, "service"},

  {"Dip A"         , 2, &Cpi01A    , "dip"},
  {"Dip B"         , 2, &Cpi01C    , "dip"},
  {"Dip C"         , 2, &Cpi01E    , "dip"},

  {"Cpu Speed (Mhz)", 2, &nCpuSpeed    , "dip"},
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
  if (n>=0x01 && n<0x1f) return 2;
  if (n>=0x34 && n<0x35) return 2;
  if (n>=0x79 && n<0x84) return 2;
  if (n>=0x8c && n<0x8d) return 2;

  if (n>=0x8f && n<0xad) return 2;
  if (n>=0xc2 && n<0xc3) return 2;
  return 1;
}

static int DrvFind1(int t) { return (t+0x10000)<<6; }
static int DrvFind2(int t) { return (t+0x08000)<<7; }
static int DrvFind3(int t) { return (t+0x02000)<<9; }

static int DrvInit()
{
  int nRet=0; int i=0;
  Cps=1;
  nCpsRomLen =  0x180000;
  nCpsGfxLen =  0x600000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen  =  0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (they are already byteswapped)
  for (i=0;i<3;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,0+i,1); if (nRet!=0) return 1; }

  // Load up and interleve each set of 4 roms to make the 16x16 tiles
  CpsLoadTiles(CpsGfx+0x000000,3+0*4);
  CpsLoadTiles(CpsGfx+0x200000,3+1*4);
  CpsLoadTiles(CpsGfx+0x400000,3+2*4);

  // Load Z80 Rom
  nRet=BurnLoadRom(CpsZRom,15,1);
  BurnScode=DrvScode;

  // Load ADPCM data
  nRet=BurnLoadRom(CpsAd        ,16,1);
  nRet=BurnLoadRom(CpsAd+0x20000,17,1);
  nPsndIrqPeriod=(60<<10)/250; //OLDTST

  // Extras:
  CpsMult[0]=0x40; CpsMult[1]=0x42; CpsMult[2]=0x44; CpsMult[3]=0x46; // Multiply ports
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  // Custom tile finder:
  CpsFind1=DrvFind1; CpsFind2=DrvFind2; CpsFind3=DrvFind3;

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
  int nSpeed=0;

  nSpeed=nCpuSpeed; if (nSpeed>32) nSpeed=32;
  nCpsCycles=nSpeed*1000000/60;
  // Carry on to CPS frame
  CpsFrame();
  nCpsCycles=0;
  return 0;
}
// Street Fighter 2 CE ----------------------------------
static int Sf2ceZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sf2ce.zip"; return 0; }
  return 1;
}

static struct StdRomInfo Sf2ceRomDesc[]=
{
  {"sf2ce.23" ,0x80000,0x3f846b74, 0x10}, //  0 68000 code
  {"sf2ce.22" ,0x80000,0x99f1cca4, 0x10}, //  1
  {"sf2ce.21" ,0x80000,0x925a7877, 0x10}, //  2

// 3
  {"sf2.01"   ,0x80000,0x03b0d852,    1},
  {"sf2.03"   ,0x80000,0x840289ec,    1},
  {"sf2.02"   ,0x80000,0xcdb5f027,    1},
  {"sf2.04"   ,0x80000,0xe2799472,    1},

  {"sf2.05"   ,0x80000,0xba8a2761,    1},
  {"sf2.07"   ,0x80000,0xe584bfb5,    1},
  {"sf2.06"   ,0x80000,0x21e3f87d,    1},
  {"sf2.08"   ,0x80000,0xbefc47df,    1},

// 11
  {"sf2.10"   ,0x80000,0x960687d5,    1},
  {"sf2.12"   ,0x80000,0x978ecd18,    1},
  {"sf2.11"   ,0x80000,0xd6ec9a0a,    1},
  {"sf2.13"   ,0x80000,0xed2c67f6,    1},

// 15
// z80 program
  {"sf2.09"   ,0x10000,0x08f6b60e,    2},
// adpcm samples
  {"sf2.18"   ,0x20000,0x7f162009,    2},
  {"sf2.19"   ,0x20000,0xbeade53f,    2},
// 18
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Sf2ce) STD_ROM_FN(Sf2ce)

struct BurnDriver BurnDrvCpsSf2ce=
{
  {"sf2ce","Street Fighter 2 CE",""},
  Sf2ceZipName,Sf2ceRomInfo,Sf2ceRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// Street Fighter 2 Turbo ----------------------------------
static int Sf2tZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sf2ce.zip"; return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="sf2t.zip"; return 0; }
  return 1;
}

static struct StdRomInfo Sf2tRomDesc[]=
{
  {"sf2.23"   ,0x80000,0x89a1fc38, 0x10}, //  0 68000 code
  {"sf2.22"   ,0x80000,0xaea6e035, 0x10}, //  1
  {"sf2.21"   ,0x80000,0xfd200288, 0x10}, //  2

  {"sf2t.10"  ,0x80000,0x3c042686,    1},
  {"sf2t.12"  ,0x80000,0x8b7e7183,    1},
  {"sf2t.11"  ,0x80000,0x293c888c,    1},
  {"sf2t.13"  ,0x80000,0x842b35a4,    1},
};

// Make The RomInfo/Name functions for the game
static struct StdRomInfo *Sf2tPickRom(unsigned int i)
{
  if (i< 3) return Sf2tRomDesc +i; // 68000 program
  if (i<11) return Sf2ceRomDesc+i;
  if (i<15) return Sf2tRomDesc+i-8; // background tiles
  if (i<18) return Sf2ceRomDesc+i;
  return NULL;
}

STD_ROM_FN(Sf2t)

struct BurnDriver BurnDrvCpsSf2t=
{
  {"sf2t","Street Fighter 2 Turbo",""},
  Sf2tZipName,Sf2tRomInfo,Sf2tRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// ------------------------------------------------------
