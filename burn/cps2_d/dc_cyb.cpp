#include "../burnint.h"
// Cybots (Jpn) - Driver
static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"        , 2, &Region,     "dip"},
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
  if (n<=0x00) return 1;
  if (n<=0x3a) return 2;
  return 1;
}

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
  nRet=BurnLoadRom(pqs         ,20,1);
  nRet=BurnLoadRom(pqs+0x200000,21,1);
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
  if (Region<=5) { CpsRamFF[0x80a0^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="cybotsj.zip";  return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"cybjx.03",0x80000,0x867c9acd, 0x10}, // xor decryption tables
  {"cybjx.04",0x80000,0x57ed677f, 0x10},

  {"cybj.03" ,0x80000,0x6096eada, 0x10}, // 68000 code (encrypted)
  {"cybj.04" ,0x80000,0x7b0ffaa9, 0x10},
  {"cybj.05" ,0x80000,0xec40408e, 0x10},
  {"cybj.06" ,0x80000,0x1ad0bed2, 0x10},
  {"cybj.07" ,0x80000,0x6245a39a, 0x10},
  {"cybj.08" ,0x80000,0x4b48e223, 0x10},
  {"cybj.09" ,0x80000,0xe15238f6, 0x10},
  {"cybj.10" ,0x80000,0x75f4003b, 0x10},

  // graphics:
  {"cyb.13" ,0x400000,0xf0dce192,    1}, // 10
  {"cyb.15" ,0x400000,0x187aa39c,    1},
  {"cyb.17" ,0x400000,0x8a0e4b12,    1},
  {"cyb.19" ,0x400000,0x34b62612,    1},
  {"cyb.14" ,0x400000,0xc1537957,    1},
  {"cyb.16" ,0x400000,0x15349e86,    1},
  {"cyb.18" ,0x400000,0xd83e977d,    1},
  {"cyb.20" ,0x400000,0x77cdad5c,    1},

  // sound - z80 roms
  {"cyb.01" ,0x020000,0x9c0fb079,    2}, // 18
  {"cyb.02" ,0x020000,0x51cb0c4e,    2},
  // sound - samples
  {"cyb.11" ,0x200000,0x362ccab2,    2},
  {"cyb.12" ,0x200000,0x7066e9cc,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsCybotsj=
{
  {"cybotsj" ,"Cyberbots: Full Metal Madness (Jpn)",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
