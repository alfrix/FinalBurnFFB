#include "../burnint.h"
// Marvel Vs. Capcom - Driver

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int CpsCount=0;
  CpsCount=sizeof(CpsFsi)/sizeof(CpsFsi[0]);
  if (i<CpsCount) { if (pii!=NULL) *pii=CpsFsi[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n)
{
  if (n<0x30) return 2; // music
  return 1; // sound effect
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  8*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=  8*0x400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x400000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<8;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  nRet=BurnXorRom(CpsCode         ,0,1); if (nRet!=0) return 1;
  nRet=BurnXorRom(CpsCode+0x080000,1,1); if (nRet!=0) return 1;

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx           ,10);
  nRet=Cps2LoadTiles(CpsGfx+4*0x400000,14);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,18,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,19,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         ,20,1);
  nRet=BurnLoadRom(pqs+0x400000,21,1);
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
  if (i==0) { if (pszName!=NULL) *pszName="mvsc.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"mvcux.03d"   ,0x80000,0x86685cbf, 0x10}, // xor decryption table
  {"mvcux.04d"   ,0x80000,0x438ba92b, 0x10},

  {"mvcu.03d"    ,0x80000,0xc6007557, 0x10}, // 68000 code (encrypted)
  {"mvcu.04d"    ,0x80000,0x724b2b20, 0x10},
  {"mvc.05a"     ,0x80000,0x2d8c8e86, 0x10},
  {"mvc.06a"     ,0x80000,0x8528e1f5, 0x10},
  {"mvc.07"      ,0x80000,0xc3baa32b, 0x10},
  {"mvc.08"      ,0x80000,0xbc002fcd, 0x10},
  {"mvc.09"      ,0x80000,0xc67b26df, 0x10},
  {"mvc.10"      ,0x80000,0x0fdd1e26, 0x10},

  // graphics:
  {"mvc.13"     ,0x400000,0xfa5f74bc,    1}, // 10
  {"mvc.15"     ,0x400000,0x71938a8f,    1},
  {"mvc.17"     ,0x400000,0x92741d07,    1},
  {"mvc.19"     ,0x400000,0xbcb72fc6,    1},
  {"mvc.14"     ,0x400000,0x7f1df4e4,    1},
  {"mvc.16"     ,0x400000,0x90bd3203,    1},
  {"mvc.18"     ,0x400000,0x67aaf727,    1},
  {"mvc.20"     ,0x400000,0x8b0bade8,    1},

  // sound - z80 roms
  {"mvc.01"     ,0x020000,0x41629e95,    1},
  {"mvc.02"     ,0x020000,0x963abf6b,    1},
  // sound - samples
  {"mvc.11"     ,0x400000,0x850fe663,    2},
  {"mvc.12"     ,0x400000,0x7ccb1896,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsMvsc=
{
  {"mvsc" ,"Marvel Vs. Capcom",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame, CpsAreaScan,
  &CpsRecalcPal,384,224
};
