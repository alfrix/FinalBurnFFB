#include "../burnint.h"
// X-Men vs Street Fighter - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"           , 2, &Region,           "dip"},
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

static int DrvScode(unsigned int n) { if (n<=0x2b) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  7*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=  8*0x400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<7;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  nRet=BurnXorRom(CpsCode         ,0,1); if (nRet!=0) return 1;
  nRet=BurnXorRom(CpsCode+0x080000,1,1); if (nRet!=0) return 1;

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx           , 9);
  nRet=Cps2LoadTiles(CpsGfx+4*0x400000,13);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,17,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,18,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         ,19,1);
  nRet=BurnLoadRom(pqs+0x200000,20,1);
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
  if (Region<=5) { CpsRamFF[0x5479^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="xmvsf.zip";  return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"xvsux.03h",0x80000,0x1539c639, 0x10}, // xor decryption tables
  {"xvsux.04h",0x80000,0x68916b3f, 0x10},

  {"xvsu.03h" ,0x80000,0x5481155a, 0x10}, // 68000 code (encrypted)
  {"xvsu.04h" ,0x80000,0x1e236388, 0x10},
  {"xvs.05a"  ,0x80000,0x7db6025d, 0x10},
  {"xvs.06a"  ,0x80000,0xe8e2c75c, 0x10},
  {"xvs.07"   ,0x80000,0x08f0abed, 0x10},
  {"xvs.08"   ,0x80000,0x81929675, 0x10},
  {"xvs.09"   ,0x80000,0x9641f36b, 0x10},

  // graphics:
  {"xvs.13"  ,0x400000,0xf6684efd,    1}, //  9
  {"xvs.15"  ,0x400000,0x29109221,    1},
  {"xvs.17"  ,0x400000,0x92db3474,    1},
  {"xvs.19"  ,0x400000,0x3733473c,    1},
  {"xvs.14"  ,0x400000,0xbcac2e41,    1},
  {"xvs.16"  ,0x400000,0xea04a272,    1},
  {"xvs.18"  ,0x400000,0xb0def86a,    1},
  {"xvs.20"  ,0x400000,0x4b40ff9f,    1},

  // sound - z80 roms
  {"xvs.01"  ,0x020000,0x3999e93a,    2}, // 17
  {"xvs.02"  ,0x020000,0x101bdee9,    2},
  // sound - samples
  {"xvs.11"  ,0x200000,0x9cadcdbc,    2},
  {"xvs.12"  ,0x200000,0x7b11e460,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsXmvsf=
{
  {"xmvsf" ,"X-Men Vs. Street Fighter",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
