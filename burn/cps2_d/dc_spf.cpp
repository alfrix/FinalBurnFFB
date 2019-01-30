#include "../burnint.h"
// Super Puzzle Fighter - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Shot 1"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Shot 2"     , 0, CpsInp001+5, "p1 fire 2"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Shot 1"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Shot 2"     , 0, CpsInp000+5, "p2 fire 2"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
  {"Region"        , 2, &Region,     "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nDrvCount=0;
  nDrvCount=sizeof(DrvInp)/sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n) { if (n<=0x1f) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  2*0x080000;
  nCpsCodeLen= 1*0x080000;
  nCpsGfxLen=   0x0400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<2;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,1+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<1;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx         ,3);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,7,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,8,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         , 9,1);
  nRet=BurnLoadRom(pqs+0x200000,10,1);
  BurnByteswap(pqs,nCpsQSamLen);
  BurnScode=DrvScode;

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
  // Insert region code into RAM
  if (Region<=4) { CpsRamFF[0x8091^1]=(unsigned char)(Region<<1); }
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="spf2t.zip";  return 0; } // Parent
  if (i==1) { if (pszName!=NULL) *pszName="spf2xj.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"pzfjx.03a",0x80000,0xc2e3f231, 0x10},

  {"pzfj.03a" ,0x80000,0x2070554a, 0x10},
  {"pzf.04a"  ,0x80000,0xb80649e2, 0x10},

  // graphics:
  {"pzf.14"  ,0x100000,0x2d4881cb,    1}, // 3
  {"pzf.16"  ,0x100000,0x4b0fd1be,    1},
  {"pzf.18"  ,0x100000,0xe43aac33,    1},
  {"pzf.20"  ,0x100000,0x7f536ff1,    1},

  // sound - z80 roms
  {"pzf.01"  ,0x020000,0x600fb2a3,    2}, // 7
  {"pzf.02"  ,0x020000,0x496076e0,    2},
  // sound - samples
  {"pzf.11"  ,0x200000,0x78442743,    2},
  {"pzf.12"  ,0x200000,0x399d2c7b,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSpf2xj=
{
  {"spf2xj" ,"Super Puzzle Fighter 2 X",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// -----------------------------------------------------------
