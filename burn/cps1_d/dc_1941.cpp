#include "../burnint.h"
// 1941 - Driver

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
  if (n<0x40) return 1; // Sound
  if (n<0x60) return 2; // Music
  if (n<0xac) return 1; // Sound
  if (n<0xcc) return 2; // Music
  if (n<0xf0) return 1; // Sound
  return 0; //  0xf0=stop
}

static int DrvInit()
{
  int nRet=0;
  nRet=RotInit(); if (nRet!=0) return 1;

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
  BurnLoadRom(CpsZRom,9,1);
  nPsndIrqPeriod=(60<<10)/250; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  BurnLoadRom(CpsAd         ,10,1);
  BurnLoadRom(CpsAd+0x020000,11,1);

  // Extras:
  CpsId[0]=0x60; CpsId[1]=0x0005; // Board ID
  CpsPmReg[0]=0x6a; CpsPmReg[1]=0x6c; CpsPmReg[2]=0x6e; CpsPmReg[3]=0x70;
  CpsLcReg=0x68; CpsLayEn[1]=0x02; CpsLayEn[2]=0x08; CpsLayEn[3]=0x20;
  Cpi01C=0x2b; Cpi01E=0x60;
  // Ready to go
  return CpsRunInit();
}

static int DrvExit()
{
  CpsRunExit(); CpsExit();
  RotExit();
  return 0;
}

static int DrvFrame()
{
  RotStart();
  CpsFrame();
  RotStop();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="1941.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"41e_30.rom" ,0x20000,0x9deb1e75,0x10}, // 0 even 68000 code
  {"41e_35.rom" ,0x20000,0xd63942b3,0x10}, // 1 odd
  {"41e_31.rom" ,0x20000,0xdf201112,0x10}, //
  {"41e_36.rom" ,0x20000,0x816a818f,0x10}, //
  {"41_32.rom"  ,0x80000,0x4e9648ca,0x10}, // 4 both

  // graphics:
  {"41_gfx5.rom",0x80000,0x01d1cb11,   1}, // 5
  {"41_gfx7.rom",0x80000,0xaeaa3509,   1},
  {"41_gfx1.rom",0x80000,0xff77985a,   1},
  {"41_gfx3.rom",0x80000,0x983be58f,   1},

  // z80 rom
  {"41_09.rom"  ,0x10000,0x0f9d8527,   2}, // 9
  // samples
  {"41_18.rom"  ,0x20000,0xd1f15aeb,   2},
  {"41_19.rom"  ,0x20000,0x15aec3a6,   2}
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCps1941=
{
  {"1941","1941 - Counter Attack",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,224,384
};
