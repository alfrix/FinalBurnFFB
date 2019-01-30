#include "../burnint.h"
// Street Fighter Alpha (CPS2) - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"    , 2, &Region,  "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nCpsCount=0,nDrvCount=0;
  // Inputs common to all CPS2 games
  nCpsCount=sizeof(CpsFsi)/sizeof(CpsFsi[0]);
  if (i<nCpsCount) { if (pii!=NULL) *pii=CpsFsi[i];  return 0; }
  i-=nCpsCount;

  // Driver specific
  nDrvCount=sizeof(DrvInp) /sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n) { if (n<=0x34) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;
  Cps=2;
  nCpsRomLen=  4*0x080000;
  nCpsCodeLen= 1*0x080000;
  nCpsGfxLen=    0x800000;
  nCpsZRomLen= 2*0x020000;
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
  nRet=BurnLoadRom(CpsZRom         , 9,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,10,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         ,11,1);
  nRet=BurnLoadRom(pqs+0x200000,12,1);
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
  if (Region<=5) { CpsRamFF[0x8191^1]=(unsigned char)(Region<<1); }

  CpsFrame(); // Carry on to CPS frame
  return 0;
}

// Street Fighter Alpha  ---------------------------------
static int SfaZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sfa.zip"; return 0; }
  return 1;
}

static struct StdRomInfo SfaRomDesc[]=
{
  {"sfzux.03a",0x80000,0x1a3160ed, 0x10}, // xor decryption table
  {"sfzu.03a" ,0x80000,0x49fc7db9, 0x10}, // 68000 code (encrypted)
  {"sfz.04a"  ,0x80000,0x5f99e9a5, 0x10},
  {"sfz.05a"  ,0x80000,0x0810544d, 0x10},
  {"sfz.06"   ,0x80000,0x806e8f38, 0x10}, 

  // graphics:
  {"sfz.14"  ,0x200000,0x90fefdb3,    1}, //  5
  {"sfz.16"  ,0x200000,0x5354c948,    1},
  {"sfz.18"  ,0x200000,0x41a1e790,    1},
  {"sfz.20"  ,0x200000,0xa549df98,    1},

  // sound - z80 roms
  {"sfz.01"  ,0x020000,0xffffec7d,    2}, //  9
  {"sfz.02"  ,0x020000,0x45f46a08,    2},
  // sound - samples
  {"sfz.11"  ,0x200000,0xc4b093cd,    2}, // 11
  {"sfz.12"  ,0x200000,0x8bdbc4b4,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Sfa) STD_ROM_FN(Sfa)

struct BurnDriver BurnDrvCpsSfa=
{
  {"sfa","Street Fighter Alpha",""},
  SfaZipName, SfaRomInfo, SfaRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// -------------------------------------------------------
