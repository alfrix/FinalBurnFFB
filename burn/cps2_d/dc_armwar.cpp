#include "../burnint.h"
// Armored Warriors - Driver

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
  {"P1 Fire"       , 0, CpsInp001+6, "p1 fire 3"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Attack"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Fire"       , 0, CpsInp000+6, "p2 fire 3"},

  {"P3 Coin"       , 0, CpsInp020+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp020+2, "p3 start"},
  {"P3 Up"         , 0, CpsInp011+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp011+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp011+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp011+0, "p3 right"},
  {"P3 Attack"     , 0, CpsInp011+4, "p3 fire 1"},
  {"P3 Jump"       , 0, CpsInp011+5, "p3 fire 2"},
  {"P3 Fire"       , 0, CpsInp011+6, "p3 fire 3"},

  {"Reset"         , 0, &CpsReset   ,"reset"},
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
  if (n<0x01) return 1; // sound
  if (n<0x30) return 2; // music
  if (n<0x49) return 1; // sound
  if (n<0x4a) return 2; // music
  if (n<0x4d) return 1; // sound
  if (n<0x4e) return 2; // music
  return 1; // sound
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  8*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=   0x1400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<8;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  nRet=BurnXorRom(CpsCode         ,0,1); if (nRet!=0) return 1;
  nRet=BurnXorRom(CpsCode+0x080000,1,1); if (nRet!=0) return 1;

  // Load graphics roms
  Cps2LoadTiles(CpsGfx           ,10);
  Cps2LoadTiles(CpsGfx+4*0x400000,14);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom         ,18,1);
  BurnLoadRom(CpsZRom+ 0x20000,19,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,20,1);
  BurnLoadRom(pqs+0x200000,21,1);
  BurnByteswap(pqs,nCpsQSamLen);
  BurnScode=DrvScode;

  // Extras:
  CpsMult[0]=0x40; CpsMult[1]=0x42; CpsMult[2]=0x44; CpsMult[3]=0x46; // Multiply ports

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

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="armwar.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"pwgux.03b",0x80000,0x5d41ddde, 0x10}, // 0, xor decryption table
  {"pwgux.04b",0x80000,0x4d0619f3, 0x10},

  {"pwgu.03b" ,0x80000,0x8b95497a, 0x10}, // 2, 68000 code (encrypted)
  {"pwgu.04b" ,0x80000,0x29eb5661, 0x10},
  {"pwgu.05b" ,0x80000,0xa54e9e44, 0x10},
  {"pwg.06"   ,0x80000,0x87a60ce8, 0x10},
  {"pwg.07"   ,0x80000,0xf7b148df, 0x10},
  {"pwg.08"   ,0x80000,0xcc62823e, 0x10},
  {"pwg.09a"  ,0x80000,0x4c26baee, 0x10},
  {"pwg.10"   ,0x80000,0x07c4fb28, 0x10},

  // graphics:
  {"pwg.13"  ,0x400000,0xae8fe08e,    1}, // 10
  {"pwg.15"  ,0x400000,0xdb560f58,    1},
  {"pwg.17"  ,0x400000,0xbc475b94,    1},
  {"pwg.19"  ,0x400000,0x07439ff7,    1},
  {"pwg.14"  ,0x100000,0xc3f9ba63,    1},
  {"pwg.16"  ,0x100000,0x815b0e7b,    1},
  {"pwg.18"  ,0x100000,0x0109c71b,    1},
  {"pwg.20"  ,0x100000,0xeb75ffbe,    1},

  // sound - z80 roms
  {"pwg.01"  ,0x020000,0x18a5c0e4,    2}, // 18
  {"pwg.02"  ,0x020000,0xc9dfffa6,    2},
  // sound - samples
  {"pwg.11"  ,0x200000,0xa78f7433,    2},
  {"pwg.12"  ,0x200000,0x77438ed0,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsArmwar=
{
  {"armwar" ,"Armored Warriors",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
