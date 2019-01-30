#include "../burnint.h"
// Quiz Nanairo Dreams - Driver

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
  {"P1 Shot 3"     , 0, CpsInp001+6, "p1 fire 3"},
  {"P1 Shot 4"     , 0, CpsInp001+7, "p1 fire 4"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Shot 1"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Shot 2"     , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Shot 3"     , 0, CpsInp000+6, "p2 fire 3"},
  {"P2 Shot 4"     , 0, CpsInp000+7, "p2 fire 4"},

  {"Reset"         , 0, &CpsReset,   "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nDrvCount=0;
  nDrvCount=sizeof(DrvInp)/sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n)
{
  if (n>=0x300) n-=0x300;
  if (n<0x30) return 2;
  return 1;
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;
  Cps=2;
  nCpsRomLen=  4*0x080000;
  nCpsCodeLen= 1*0x080000;
  nCpsGfxLen=    0x800000;
  nCpsZRomLen= 1*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<4;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,1+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  nRet=BurnXorRom(CpsCode,0,1); if (nRet!=0) return 1;

  // Load graphics roms
  Cps2LoadTiles(CpsGfx,5);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom         , 9,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,10,1);
  BurnLoadRom(pqs+0x200000,11,1);
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
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="qndream.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"tqzjx.03" ,0x80000,0x5804a8f8, 0x10}, // xor decryption table
  {"tqzj.03"  ,0x80000,0x7acf3e30, 0x10}, // 68000 code (encrypted)
  {"tqzj.04"  ,0x80000,0xf1044a87, 0x10},
  {"tqzj.05"  ,0x80000,0x4105ba0e, 0x10},
  {"tqzj.06"  ,0x80000,0xc371e8a5, 0x10}, 

  // graphics:
  {"tqz.14"  ,0x200000,0x98af88a2,    1}, //  5
  {"tqz.16"  ,0x200000,0xdf82d491,    1},
  {"tqz.18"  ,0x200000,0x42f132ff,    1},
  {"tqz.20"  ,0x200000,0xb2e128a3,    1},

  // sound - z80 roms
  {"tqz.01"  ,0x020000,0xe9ce9d0a,    2}, //  9
  // sound - samples
  {"tqz.11"  ,0x200000,0x78e7884f,    2}, // 10
  {"tqz.12"  ,0x200000,0x2e049b13,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsQndream=
{
  {"qndream","Quiz Nanairo Dreams",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// -------------------------------------------------------
