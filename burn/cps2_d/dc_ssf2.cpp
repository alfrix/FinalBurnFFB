#include "../burnint.h"
// SSF2 - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"    , 2, &Region,  "dip"},
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

static int DrvScode(unsigned int n)
{
  n&=0x1ff;
  if (n<=0x3f) return 2; // music
  if (n==0xd1) return 2; // music
  return 1; // sound effect
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  5*0x080000;
  nCpsCodeLen= 5*0x080000;
  nCpsGfxLen=    0xc00000;
  nCpsZRomLen= 1*0x020000;
  nCpsQSamLen= 8*0x080000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<5;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,5+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<5;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,0+i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx           ,10);
  nRet=Cps2LoadTiles(CpsGfx+  0x800000,14);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom,18,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  for (i=0;i<8;i++)
  {
    nRet=BurnLoadRom(pqs+0x080000*i,19+i,1);
  }
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
  if (Region<=5) { CpsRamFF[0x82cf^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="ssf2.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
// 0
  {"ssfux.03a",0x80000,0xec278279, 0x10}, // xor decryption tables
  {"ssfux.04a",0x80000,0x6194d896, 0x10},
  {"ssfux.05" ,0x80000,0xaa846b9f, 0x10},
  {"ssfux.06" ,0x80000,0x235268c4, 0x10},
  {"ssfux.07" ,0x80000,0xe46e033c, 0x10},

// 5
  {"ssfu.03a" ,0x80000,0x72f29c33, 0x10}, // 68000 code (encrypted)
  {"ssfu.04a" ,0x80000,0x935cea44, 0x10},
  {"ssfu.05"  ,0x80000,0xa0acb28a, 0x10},
  {"ssfu.06"  ,0x80000,0x47413dcf, 0x10},
  {"ssfu.07"  ,0x80000,0xe6066077, 0x10},

// 10
  // graphics:
  {"ssf.13"  ,0x200000,0xcf94d275,    1},
  {"ssf.15"  ,0x200000,0x5eb703af,    1},
  {"ssf.17"  ,0x200000,0xffa60e0f,    1},
  {"ssf.19"  ,0x200000,0x34e825c5,    1},
  {"ssf.14"  ,0x100000,0xb7cc32e7,    1},
  {"ssf.16"  ,0x100000,0x8376ad18,    1},
  {"ssf.18"  ,0x100000,0xf5b1b336,    1},
  {"ssf.20"  ,0x100000,0x459d5c6b,    1},

// 18
	// sound - z80 rom
  {"ssf.01"  ,0x020000,0xeb247e8c,    2},
// 19
  // sound - samples
  {"ssf.q01" ,0x080000,0xa6f9da5c,    2},
  {"ssf.q02" ,0x080000,0x8c66ae26,    2},
  {"ssf.q03" ,0x080000,0x695cc2ca,    2},
  {"ssf.q04" ,0x080000,0x9d9ebe32,    2},
  {"ssf.q05" ,0x080000,0x4770e7b7,    2},
  {"ssf.q06" ,0x080000,0x4e79c951,    2},
  {"ssf.q07" ,0x080000,0xcdd14313,    2},
  {"ssf.q08" ,0x080000,0x6f5a088c,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSsf2=
{
  {"ssf2", "Super Street Fighter 2","Some glitches"},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// ------------------------------------------------------
