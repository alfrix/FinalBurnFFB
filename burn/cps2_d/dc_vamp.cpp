#include "../burnint.h"
// Vampire (Jpn) - Driver

static unsigned char Region=0xff;
static unsigned char FreePlayOption=0;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"          , 2, &Region,         "dip"},
  {"FreePlayOption"  , 2, &FreePlayOption, "dip"},
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
  if (n<=0x2f) return 2;  return 1;
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
  nRet=Cps2LoadTiles(CpsGfx           ,10);
  nRet=Cps2LoadTiles(CpsGfx+ 0x1000000,14);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,18,1);
  nRet=BurnLoadRom(CpsZRom+ 0x20000,19,1);

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
  if (Region<=4)
  {
    CpsRamFF[0x8205^1]=(unsigned char)(Region<<1);
    if (Region>=1) CpsRamFF[0x8214^1]=1; // not japan -> title=darkstalkers
  }

  if (FreePlayOption<=1) CpsRamFF[0x8239^1]=FreePlayOption;

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// Darkstalkers (Parent Romset)
static struct StdRomInfo DstlkRom[]=
{
  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },

  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },

  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },
  {""         ,0      ,0         , 0   },

  // graphics:
  {"vam.13"  ,0x400000,0xc51baf99,    1}, // 10
  {"vam.15"  ,0x400000,0x3ce83c77,    1},
  {"vam.17"  ,0x400000,0x4f2408e0,    1},
  {"vam.19"  ,0x400000,0x9ff60250,    1},
  {"vam.14"  ,0x100000,0xbd87243c,    1},
  {"vam.16"  ,0x100000,0xafec855f,    1},
  {"vam.18"  ,0x100000,0x3a033625,    1},
  {"vam.20"  ,0x100000,0x2bff6a89,    1},

  // sound - z80 roms
  {"vam.01"  ,0x020000,0x64b685d5,    2}, // 18
  {"vam.02"  ,0x020000,0xcf7c97c7,    2},
  // sound - samples
  {"vam.11"  ,0x200000,0x4a39deb2,    2},
  {"vam.12"  ,0x200000,0x1a3e5c03,    2},
   // 22
};

// Vampire differences (Japan Romset)
static struct StdRomInfo VampRom[]=
{
  {"vamjx.03a",0x80000,0x2549f7bc, 0x10}, // xor decryption table
  {"vamjx.04b",0x80000,0xbb5a30a5, 0x10}, // xor decryption table

  {"vamj.03a" ,0x80000,0xf55d3722, 0x10}, // 68000 code (encrypted)
  {"vamj.04b" ,0x80000,0x4d9c43c4, 0x10},
  {"vamj.05a" ,0x80000,0x6c497e92, 0x10},
  {"vamj.06a" ,0x80000,0xf1bbecb6, 0x10}, 

  {"vamj.07a" ,0x80000,0x1067ad84, 0x10},
  {"vamj.08a" ,0x80000,0x4b89f41f, 0x10}, 
  {"vamj.09a" ,0x80000,0xfc0a4aac, 0x10}, 
  {"vamj.10a" ,0x80000,0x9270c26b, 0x10}, 
};

// --------------------- Vampire (Jpn) ------------------
static int VampjZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="dstlk.zip"; return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="vampj.zip"; return 0; }
  return 1;
}

static int VampjRomInfo(struct BurnRomInfo *pri,unsigned int i)
{
  struct StdRomInfo *por;
  if (i<10) por=VampRom+i; else if (i<22) por=DstlkRom+i; else return 1;
  if (pri==NULL) return 0;
  pri->nLen =por->nLen; pri->nCrc =por->nCrc; pri->nType=por->nType;
  return 0;
}
 
static int VampjRomName(char **pszName,unsigned int i,int nAka)
{
  struct StdRomInfo *por;
  if (i<10) por=VampRom+i; else if (i<22) por=DstlkRom+i; else return 1;
  if (nAka!=0) return 1;
  *pszName=por->szName;
  return 0;
}

struct BurnDriver BurnDrvCpsVampj=
{
  {"vampj" ,"Vampire (Jpn)",""},
  VampjZipName, VampjRomInfo, VampjRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// -----------------------------------------------------------
