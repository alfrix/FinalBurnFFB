#include "../burnint.h"
// X-Men:COTA - Driver
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

static int DrvScode(unsigned int n) { if (n<=0x1d) return 2;   return 1; }

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
  nRet=Cps2LoadTiles(CpsGfx           ,10);
  nRet=Cps2LoadTiles(CpsGfx+4*0x400000,14);

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
  if (Region<=5) { CpsRamFF[0x4bf7^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="xmcota.zip";  return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"xmnux.03e",0x80000,0x27636ac7, 0x10}, // xor decryption table
  {"xmnux.04e",0x80000,0x0aed300c, 0x10}, // xor decryption table

  {"xmnu.03e" ,0x80000,0x0bafeb0e, 0x10}, // 68000 code (encrypted)
  {"xmnu.04e" ,0x80000,0xc29bdae3, 0x10},
  {"xmnu.05a" ,0x80000,0xac0d7759, 0x10},
  {"xmnu.06a" ,0x80000,0x1b86a328, 0x10},
  {"xmnu.07a" ,0x80000,0x2c142a44, 0x10},
  {"xmnu.08a" ,0x80000,0xf712d44f, 0x10},
  {"xmnu.09a" ,0x80000,0x9241cae8, 0x10},
  {"xmnu.10a" ,0x80000,0x53c0eab9, 0x10},

// 10
  // graphics:
  {"xmn.13"  ,0x400000,0xbf4df073,    1},
  {"xmn.15"  ,0x400000,0x4d7e4cef,    1},
  {"xmn.17"  ,0x400000,0x513eea17,    1},
  {"xmn.19"  ,0x400000,0xd23897fc,    1},
  {"xmn.14"  ,0x400000,0x778237b7,    1},
  {"xmn.16"  ,0x400000,0x67b36948,    1},
  {"xmn.18"  ,0x400000,0x015a7c4c,    1},
  {"xmn.20"  ,0x400000,0x9dde2758,    1},

// 18
  // sound - z80 roms
  {"xmn.01a" ,0x020000,0x40f479ea,    2},
  {"xmn.02a" ,0x020000,0x39d9b5ad,    2},
  // sound - samples
  {"xmn.11"  ,0x200000,0xc848a6bc,    2},
  {"xmn.12"  ,0x200000,0x729c188f,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsXmcota=
{
  {"xmcota" ,"X-Men: Children of the Atom","v3.00"},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

// ------------------------------------------------------
static int XmcotajZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="xmcota.zip";  return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="xmcotaj.zip"; return 0; }
  return 1;
}

static struct StdRomInfo XmcotajRomDesc[]=
{
// In xmcotaj.zip
  {"xmnjx.03b",0x80000,0x523c9589, 0x10}, // xor decryption table
  {"xmnjx.04b",0x80000,0x673765ba, 0x10}, // xor decryption table

  {"xmnj.03b" ,0x80000,0xc8175fb3, 0x10}, // 68000 code (encrypted)
  {"xmnj.04b" ,0x80000,0x54b3fba3, 0x10},
  {"xmnj.05"  ,0x80000,0xc3ed62a2, 0x10},
  {"xmnj.06"  ,0x80000,0xf03c52e1, 0x10},
  {"xmnj.07"  ,0x80000,0x325626b1, 0x10},
  {"xmnj.08"  ,0x80000,0x7194ea10, 0x10},
  {"xmnj.09"  ,0x80000,0xae946df3, 0x10},
  {"xmnj.10"  ,0x80000,0x32a6be1d, 0x10},
};

static struct StdRomInfo *XmcotajPickRom(unsigned int i)
{
  if (i<10) return XmcotajRomDesc+i;
  if (i<22) return DrvRomDesc+i;
  return NULL;
}

STD_ROM_FN(Xmcotaj)

struct BurnDriver BurnDrvCpsXmcotaj=
{
  {"xmcotaj" ,"X-Men: Children of the Atom (Jpn)","v2.10"},
  XmcotajZipName, XmcotajRomInfo, XmcotajRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// ------------------------------------------------------
