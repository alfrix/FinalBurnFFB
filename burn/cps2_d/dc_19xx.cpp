#include "../burnint.h"
// 19XX - Driver
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
  nRet=RotInit(); if (nRet!=0) return 1;

  Cps=2;
  nCpsRomLen=  5*0x080000;
  nCpsCodeLen= 4*0x080000;
  nCpsGfxLen=   0x1000000;
  nCpsZRomLen= 1*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<5;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,4+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<4;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,0+i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  Cps2LoadTiles(CpsGfx          , 9);
  Cps2LoadTiles(CpsGfx+0x0800000,13);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom,17,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,18,1);
  BurnLoadRom(pqs+0x200000,19,1);
  BurnByteswap(pqs,nCpsQSamLen);
  BurnScode=DrvScode;

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

  // Insert Region code into RAM
  if (Region<=5) CpsRamFF[0x4d6e^1]=Region;

  // Carry on to CPS frame
  CpsFrame();

  RotStop();
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="19xx.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
// 0
  {"19xux.03" ,0x80000,0x239a08ae, 0x10}, // xor decryption table
  {"19xux.04" ,0x80000,0xc13a1072, 0x10},
  {"19xux.05" ,0x80000,0x8c066ec3, 0x10},
  {"19xux.06" ,0x80000,0x4b1caeb9, 0x10},
	
// 4
  {"19xu.03"  ,0x80000,0x05955268, 0x10}, // 68000 code (encrypted)
  {"19xu.04"  ,0x80000,0x3111ab7f, 0x10},
  {"19xu.05"  ,0x80000,0x38df4a63, 0x10},
  {"19xu.06"  ,0x80000,0x5c7e60d3, 0x10},
  {"19x.07"   ,0x80000,0x61c0296c, 0x10},

// 9
  // graphics:
  {"19x.13"  ,0x080000,0x427aeb18,    1},
  {"19x.15"  ,0x080000,0x63bdbf54,    1},
  {"19x.17"  ,0x080000,0x2dfe18b5,    1},
  {"19x.19"  ,0x080000,0xcbef9579,    1},
  {"19x.14"  ,0x200000,0xe916967c,    1},
  {"19x.16"  ,0x200000,0x6e75f3db,    1},
  {"19x.18"  ,0x200000,0x2213e798,    1},
  {"19x.20"  ,0x200000,0xab9d5b96,    1},

// 17
  // sound - z80 roms
  {"19x.01"  ,0x020000,0xef55195e,    2},
// 18
  // sound - samples
  {"19x.11"  ,0x200000,0xd38beef3,    2},
  {"19x.12"  ,0x200000,0xd47c96e2,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCps19xx=
{
  {"19xx" ,"19XX: The War Against Destiny",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,224,384
};
