#include "../burnint.h"
// Mercs - Driver

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

  {"P3 Coin"       , 0, CpsInp177+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp177+7, "p3 start"},
  {"P3 Up"         , 0, CpsInp177+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp177+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp177+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp177+0, "p3 right"},
  {"P3 Shot A"     , 0, CpsInp177+4, "p3 fire 1"},
  {"P3 Shot B"     , 0, CpsInp177+5, "p3 fire 2"},

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
  if (n>=0xf0) return 0; // f0=stop
  if (n>=0x5a) n%=0x5a; // Mirror
  if (n<0x12) return 2; // Music
  return 1; // Sound
}

static int DrvFind3(int t)
{
  if (t>=0x4000) return -1;
  return t<<9;
}

static int DrvInit()
{
  int nRet=0;
  nRet=RotInit(); if (nRet!=0) return 1;

  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x300000;
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
  CpsLoadTiles    (CpsGfx         , 5);
  CpsLoadTilesByte(CpsGfx+0x200000, 9);

  // Load Z80 Rom
  BurnLoadRom(CpsZRom,17,1);
  nPsndIrqPeriod=(60<<10)/250; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  BurnLoadRom(CpsAd         ,18,1);
  BurnLoadRom(CpsAd+0x020000,19,1);

  // Extras:
  CpsId[0]=0x60; CpsId[1]=0x0402; // Board ID
  CpsLcReg=0x6c;
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  // Custom tile finder:
  CpsFind3=DrvFind3;

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
  if (i==0) { if (pszName!=NULL) *pszName="mercs.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"so2_30e.rom" ,0x20000,0xe17f9bf7,0x10}, // 0 even 68000 code
  {"so2_35e.rom" ,0x20000,0x78e63575,0x10}, // 1 odd
  {"so2_31e.rom" ,0x20000,0x51204d36,0x10}, //
  {"so2_36e.rom" ,0x20000,0x9cfba8b4,0x10}, //
  {"so2_32.rom"  ,0x80000,0x2eb5cf0c,0x10}, // 4 both

  // graphics:
  {"so2_gfx6.rom",0x80000,0xaa6102af,   1}, // 5
  {"so2_gfx8.rom",0x80000,0x839e6869,   1},
  {"so2_gfx2.rom",0x80000,0x597c2875,   1},
  {"so2_gfx4.rom",0x80000,0x912a9ca0,   1},
  {"so2_24.rom"  ,0x20000,0x3f254efe,   1},
  {"so2_14.rom"  ,0x20000,0xf5a8905e,   1},
  {"so2_26.rom"  ,0x20000,0xf3aa5a4a,   1},
  {"so2_16.rom"  ,0x20000,0xb43cd1a8,   1},
  {"so2_20.rom"  ,0x20000,0x8ca751a3,   1},
  {"so2_10.rom"  ,0x20000,0xe9f569fd,   1},
  {"so2_22.rom"  ,0x20000,0xfce9a377,   1},
  {"so2_12.rom"  ,0x20000,0xb7df8a06,   1},

  // z80 rom
  {"so2_09.rom"  ,0x10000,0xd09d7c7a,   2}, // 17
  // samples
  {"so2_18.rom"  ,0x20000,0xbbea1643,   2},
  {"so2_19.rom"  ,0x20000,0xac58aa71,   2}
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsMercs=
{
  {"mercs","Mercs",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,224,384
};
