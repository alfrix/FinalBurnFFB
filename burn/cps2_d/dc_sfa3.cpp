#include "../burnint.h"
// Street Fighter Alpha 3 - Driver

static unsigned char TimeRelease=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"TimeRelease"    , 2, &TimeRelease,  "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nCpsCount=0,nDrvCount=0;
  nCpsCount=sizeof(CpsFsi)/sizeof(CpsFsi[0]);
  if (i<nCpsCount) { if (pii!=NULL) *pii=CpsFsi[i];  return 0; }
  i-=nCpsCount;
  nDrvCount=sizeof(DrvInp) /sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n)
{
  n%=0xc00; // mirrored at 0xc00 intervals
  if (n<=0x02f) return 2;
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
  // Time release
  if (TimeRelease<=4) { CpsRamFF[0x80ee^1]=TimeRelease; }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sfa3.zip";   return 0; } 
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"sz3ux.03c"   ,0x80000,0x7091276b, 0x10}, 
  {"sz3ux.04c"   ,0x80000,0x83b213b1, 0x10},

  {"sz3u.03c"    ,0x80000,0xe007da2e, 0x10}, 
  {"sz3u.04c"    ,0x80000,0x5f78f0e7, 0x10},
  {"sz3.05c"     ,0x80000,0x57fd0a40, 0x10},
  {"sz3.06c"     ,0x80000,0xf6305f8b, 0x10},
  {"sz3.07c"     ,0x80000,0x6eab0f6f, 0x10},
  {"sz3.08c"     ,0x80000,0x910c4a3b, 0x10},
  {"sz3.09c"     ,0x80000,0xb29e5199, 0x10},
  {"sz3.10b"     ,0x80000,0xdeb2ff52, 0x10},
 
  {"sz3.13"     ,0x400000,0x0f7a60d9,    1}, 
  {"sz3.15"     ,0x400000,0x8e933741,    1},
  {"sz3.17"     ,0x400000,0xd6e98147,    1},
  {"sz3.19"     ,0x400000,0xf31a728a,    1},
  {"sz3.14"     ,0x400000,0x5ff98297,    1},
  {"sz3.16"     ,0x400000,0x52b5bdee,    1},
  {"sz3.18"     ,0x400000,0x40631ed5,    1},
  {"sz3.20"     ,0x400000,0x763409b4,    1},
 
  {"sz3.01"     ,0x020000,0xde810084,    1},
  {"sz3.02"     ,0x020000,0x72445dc4,    1},
  
  {"sz3.11"     ,0x400000,0x1c89eed1,    2},
  {"sz3.12"     ,0x400000,0xf392b13a,    2},
};

STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSfa3=
{
  {"sfa3" ,"Street Fighter Alpha 3",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
