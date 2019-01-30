#include "../burnint.h"
// Marvel Super Heroes - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Region",2,&Region,"dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nCpsCount=0,nDrvCount=0;
  // Inputs common to all CPS2 Fighter Style games
  nCpsCount=sizeof(CpsFsi)/sizeof(CpsFsi[0]);
  if (i<nCpsCount) { if (pii!=NULL) *pii=CpsFsi[i];  return 0; }
  i-=nCpsCount;
  // Driver specific
  nDrvCount=sizeof(DrvInp) /sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n) { if (n<=0x36) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  8*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=  8*0x400000;
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
  // Insert Region code into RAM
  if (Region<=5) { CpsRamFF[0x4c23^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="msh.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"mshux.03",0x80000,0x10bfc357, 0x10}, // xor decryption table
  {"mshux.04",0x80000,0x871f0863, 0x10},

  {"mshu.03" ,0x80000,0xd2805bdd, 0x10}, // 68000 code (encrypted)
  {"mshu.04" ,0x80000,0x743f96ff, 0x10},
  {"msh.05"  ,0x80000,0x6a091b9e, 0x10},
  {"msh.06"  ,0x80000,0x803e3fa4, 0x10},
  {"msh.07"  ,0x80000,0xc45f8e27, 0x10},
  {"msh.08"  ,0x80000,0x9ca6f12c, 0x10},
  {"msh.09"  ,0x80000,0x82ec27af, 0x10},
  {"msh.10"  ,0x80000,0x8d931196, 0x10},

  // graphics:
  {"msh.13"  ,0x400000,0x09d14566,    1}, // 10
  {"msh.15"  ,0x400000,0xee962057,    1},
  {"msh.17"  ,0x400000,0x604ece14,    1},
  {"msh.19"  ,0x400000,0x94a731e8,    1},
  {"msh.14"  ,0x400000,0x4197973e,    1},
  {"msh.16"  ,0x400000,0x438da4a0,    1},
  {"msh.18"  ,0x400000,0x4db92d94,    1},
  {"msh.20"  ,0x400000,0xa2b0c6c0,    1},

  // sound - z80 roms
  {"msh.01"  ,0x020000,0xc976e6f9,    2}, // 18
  {"msh.02"  ,0x020000,0xce67d0d9,    2},
  // sound - samples
  {"msh.11"  ,0x200000,0x37ac6d30,    2},
  {"msh.12"  ,0x200000,0xde092570,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsMsh=
{
  {"msh" ,"Marvel Super Heroes",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
