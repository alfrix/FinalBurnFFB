#include "../burnint.h"
// Street Fighter Alpha 2 - Driver
static unsigned char bStopMusicTimeout=1;

static struct BurnInputInfo DrvInp[]=
{
  {"bStopMusicTimeout", 2,&bStopMusicTimeout, "dip"},
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

static int DrvScode(unsigned int n) { if (n<=0x3f) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  6*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=   0x1400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<6;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  nRet=BurnXorRom(CpsCode         ,0,1); if (nRet!=0) return 1;
  nRet=BurnXorRom(CpsCode+0x080000,1,1); if (nRet!=0) return 1;

  // Load graphics roms
  Cps2LoadTiles(CpsGfx           , 8);
  Cps2LoadTiles(CpsGfx+4*0x400000,12);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom         ,16,1);
  BurnLoadRom(CpsZRom+ 0x20000,17,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,18,1);
  BurnLoadRom(pqs+0x200000,19,1);
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
  unsigned int *TimeOut=NULL; unsigned int OldTimeOut=0;

  // Patch to stop music timeout:
  TimeOut=(unsigned int *)(CpsCode+0x8cbda); // Point to decrement opcode
  OldTimeOut=TimeOut[0]; // Preserve old code
  if (bStopMusicTimeout) TimeOut[0]=0x4e714e71; // Change to NOP for frame

  // Carry on to CPS frame
  CpsFrame();

  TimeOut[0]=OldTimeOut; // Restore old code
  return 0;
}

// -----------------------------------------------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sfa2.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"sz2ux.03",0x80000,0x6bb6005f, 0x10}, // xor decryption table
  {"sz2ux.04",0x80000,0x74308a4b, 0x10}, // xor decryption table

  {"sz2u.03" ,0x80000,0x84a09006, 0x10}, // 68000 code (encrypted)
  {"sz2u.04" ,0x80000,0xac46e5ed, 0x10},
  {"sz2u.05" ,0x80000,0x6c0c79d3, 0x10},
  {"sz2u.06" ,0x80000,0xc5c8eb63, 0x10},
  {"sz2u.07" ,0x80000,0x5de01cc5, 0x10},
  {"sz2u.08" ,0x80000,0xbea11d56, 0x10},

  // graphics:
  {"sz2.13"  ,0x400000,0x4d1f1f22,    1}, // 8
  {"sz2.15"  ,0x400000,0x19cea680,    1},
  {"sz2.17"  ,0x400000,0xe01b4588,    1},
  {"sz2.19"  ,0x400000,0x0feeda64,    1},
  {"sz2.14"  ,0x100000,0x0560c6aa,    1},
  {"sz2.16"  ,0x100000,0xae940f87,    1},
  {"sz2.18"  ,0x100000,0x4bc3c8bc,    1},
  {"sz2.20"  ,0x100000,0x39e674c0,    1},

  // sound - z80 roms
  {"sz2.01"  ,0x020000,0x1bc323cf,    2}, // 16
  {"sz2.02"  ,0x020000,0xba6a5013,    2},
  // sound - samples
  {"sz2.11"  ,0x200000,0xaa47a601,    2},
  {"sz2.12"  ,0x200000,0x2237bc53,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSfa2=
{
  {"sfa2" ,"Street Fighter Alpha 2",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
