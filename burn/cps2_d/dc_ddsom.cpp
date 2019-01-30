#include "../burnint.h"
// Dungeons & Dragons: Shadow over Mystara - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Attack"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Jump"       , 0, CpsInp001+5, "p1 fire 2"},
  {"P1 Select"     , 0, CpsInp001+6, "p1 fire 3"},
  {"P1 Use"        , 0, CpsInp001+7, "p1 fire 4"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Attack"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Select"     , 0, CpsInp000+6, "p2 fire 3"},
  {"P2 Use"        , 0, CpsInp000+7, "p2 fire 4"},

  {"P3 Coin"       , 0, CpsInp020+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp020+2, "p3 start"},
  {"P3 Up"         , 0, CpsInp011+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp011+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp011+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp011+0, "p3 right"},
  {"P3 Attack"     , 0, CpsInp011+4, "p3 fire 1"},
  {"P3 Jump"       , 0, CpsInp011+5, "p3 fire 2"},
  {"P3 Select"     , 0, CpsInp011+6, "p3 fire 3"},
  {"P3 Use"        , 0, CpsInp011+7, "p3 fire 4"},

  {"P4 Coin"       , 0, CpsInp020+7, "p4 coin"},
  {"P4 Start"      , 0, CpsInp020+3, "p4 start"},
  {"P4 Up"         , 0, CpsInp010+3, "p4 up"},
  {"P4 Down"       , 0, CpsInp010+2, "p4 down"},
  {"P4 Left"       , 0, CpsInp010+1, "p4 left"},
  {"P4 Right"      , 0, CpsInp010+0, "p4 right"},
  {"P4 Attack"     , 0, CpsInp010+4, "p4 fire 1"},
  {"P4 Jump"       , 0, CpsInp010+5, "p4 fire 2"},
  {"P4 Select"     , 0, CpsInp010+6, "p4 fire 3"},
  {"P4 Use"        , 0, CpsInp010+7, "p4 fire 4"},

  {"Reset"         , 0, &CpsReset  , "reset"},
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
  if (n<0x3c) return 2; //music
  return 1; // Sound effect
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  8*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=   0x1800000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<8;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<2;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  Cps2LoadTiles(CpsGfx          ,10);
  Cps2LoadTiles(CpsGfx+0x1000000,14);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom         ,18,1);
  BurnLoadRom(CpsZRom+0x020000,19,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,20,1);
  BurnLoadRom(pqs+0x200000,21,1);
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
  // Stop opposite directions being pressed
  ONLY_ONE(001,0,1) ONLY_ONE(001,2,3) // Player 1
  ONLY_ONE(000,0,1) ONLY_ONE(000,2,3) // Player 2
  ONLY_ONE(011,0,1) ONLY_ONE(011,2,3) // Player 3
  ONLY_ONE(010,0,1) ONLY_ONE(010,2,3) // Player 4

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="ddsom.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"dd2ex.03b",0x80000,0xe4924c80, 0x10}, // 0,xor decryption tables
  {"dd2ex.04d",0x80000,0x13c8b16f, 0x10},

  {"dd2e.03b" ,0x80000,0xcd2deb66, 0x10}, // 2, 68000 code (encrypted)
  {"dd2e.04d" ,0x80000,0xbfee43cc, 0x10},
  {"dd2e.05b" ,0x80000,0x049ab19d, 0x10},
  {"dd2e.06d" ,0x80000,0x3994fb8b, 0x10},

  {"dd2e.07"  ,0x80000,0xbb777a02, 0x10},
  {"dd2e.08"  ,0x80000,0x30970890, 0x10},
  {"dd2e.09"  ,0x80000,0x99e2194d, 0x10},
  {"dd2e.10"  ,0x80000,0xe198805e, 0x10},
  // graphics:
  {"dd2.13"  ,0x400000,0xa46b4e6e,    1}, // 10
  {"dd2.15"  ,0x400000,0xd5fc50fc,    1},
  {"dd2.17"  ,0x400000,0x837c0867,    1},
  {"dd2.19"  ,0x400000,0xbb0ec21c,    1},
  {"dd2.14"  ,0x200000,0x6d824ce2,    1},
  {"dd2.16"  ,0x200000,0x79682ae5,    1},
  {"dd2.18"  ,0x200000,0xacddd149,    1},
  {"dd2.20"  ,0x200000,0x117fb0c0,    1},

  // sound - z80 roms
  {"dd2.01"  ,0x020000,0x99d657e5,    2}, // 18
  {"dd2.02"  ,0x020000,0x117a3824,    2},

  // sound - samples
  {"dd2.11"  ,0x200000,0x98d0c325,    2},
  {"dd2.12"  ,0x200000,0x5ea2e7fa,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsDdsom=
{
  {"ddsom" ,"Dungeons & Dragons: Shadow over Mystara",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// -----------------------------------------------------------

