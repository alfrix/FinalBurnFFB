#include "../burnint.h"
// Super Street Fighter 2 Turbo - Driver

static unsigned char nCpuSpeed=12;
static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Cpu Speed (Mhz)", 2, &nCpuSpeed    ,"dip"},
  {"Region"         , 2, &Region       ,"dip"},
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
  if (n>=0xd1 && n<=0xd6) return 2; // music
  return 1; // sound effect
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  7*0x080000;
  nCpsCodeLen= 6*0x080000;
  nCpsGfxLen=   0x1000000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<7;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,6+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<6;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx         ,13);
  nRet=Cps2LoadTiles(CpsGfx+0x800000,17);
  nRet=Cps2LoadTiles(CpsGfx+0xc00000,21);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,25,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,26,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         ,27,1);
  nRet=BurnLoadRom(pqs+0x200000,28,1);
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
  int nSpeed=0;
  // Insert region code into RAM
  if (Region<=5) { CpsRamFF[0x82cf^1]=(unsigned char)(Region<<1); }

  nSpeed=nCpuSpeed; if (nSpeed>32) nSpeed=32;
  nCpsCycles=nSpeed*1000000/60;
  CpsFrame(); // Carry on to CPS frame
  nCpsCycles=0;
  return 0;
}

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="ssf2.zip";  return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="ssf2t.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"sfxex.03c",0x80000,0xa181b207, 0x10}, // 0, xor decryption table
  {"sfxex.04a",0x80000,0xdf28bd00, 0x10},
  {"sfxex.05" ,0x80000,0x29b7bda4, 0x10},
  {"sfxex.06a",0x80000,0x6c8719b3, 0x10},
  {"sfxex.07" ,0x80000,0xdfc3b3cd, 0x10},
  {"sfxex.08" ,0x80000,0xd7436ae9, 0x10},

  {"sfxe.03c" ,0x80000,0x2fa1f396, 0x10}, // 6, 68000 code (encrypted)
  {"sfxe.04a" ,0x80000,0xd0bc29c6, 0x10},
  {"sfxe.05"  ,0x80000,0x65222964, 0x10},
  {"sfxe.06a" ,0x80000,0x8fe9f531, 0x10},
  {"sfxe.07"  ,0x80000,0x8a7d0cb6, 0x10},
  {"sfxe.08"  ,0x80000,0x74c24062, 0x10},
  {"sfx.09"   ,0x80000,0x642fae3f, 0x10},

  // graphics:
  {"ssf.13"  ,0x200000,0xcf94d275,    1}, // 13
  {"ssf.15"  ,0x200000,0x5eb703af,    1},
  {"ssf.17"  ,0x200000,0xffa60e0f,    1},
  {"ssf.19"  ,0x200000,0x34e825c5,    1},
  {"ssf.14"  ,0x100000,0xb7cc32e7,    1}, // 17
  {"ssf.16"  ,0x100000,0x8376ad18,    1},
  {"ssf.18"  ,0x100000,0xf5b1b336,    1},
  {"ssf.20"  ,0x100000,0x459d5c6b,    1},
  {"sfx.21"  ,0x100000,0xe32854af,    1}, // 21
  {"sfx.23"  ,0x100000,0x760f2927,    1},
  {"sfx.25"  ,0x100000,0x1ee90208,    1},
  {"sfx.27"  ,0x100000,0xf814400f,    1},

  // sound - z80 roms
  {"sfx.01"  ,0x020000,0xb47b8835,    2}, // 25
  {"sfx.02"  ,0x020000,0x0022633f,    2},
  // sound - samples
  {"sfx.11"  ,0x200000,0x9bdbd476,    2},
  {"sfx.12"  ,0x200000,0xa05e3aab,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSsf2t=
{
  {"ssf2t" ,"Super Street Fighter 2 Turbo (ETC)","Some glitches"},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// -----------------------------------------------------------
