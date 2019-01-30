#include "../burnint.h"
// Vampire Savior (Jpn) - Driver

static unsigned char Region=0xff;
static unsigned char Title =0xff;
static unsigned char bStopMusicTimeout=1;

static struct BurnInputInfo DrvInp[]=
{
  {"Region"           , 2, &Region,           "dip"},
  {"Title"            , 2, &Title,            "dip"},
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

static int DrvScode(unsigned int n) { if (n<=0x57) return 2;  return 1; }

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
  unsigned int *TimeOut=NULL; unsigned int OldTimeOut=0;

  // Insert Region code into RAM
  if (Region<=6) { CpsRamFF[0x8091^1]=(unsigned char)(Region<<1); }
  if (Title<=1)  { CpsRamFF[0x8089^1]=Title; }

  // Patch to stop music timeout:
  TimeOut=(unsigned int *)(CpsCode+0xe48a); // Point to decrement opcode
  OldTimeOut=TimeOut[0]; // Preserve old code
  if (bStopMusicTimeout) TimeOut[0]=0x4e714e71; // Change to NOP for frame

  // Carry on to CPS frame
  CpsFrame();

  TimeOut[0]=OldTimeOut; // Restore old code
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="vsav.zip";  return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="vsavj.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
// In vsavj.zip
  {"vm3jx.03d",0x80000,0xa9ab54df, 0x10}, // xor decryption table
  {"vm3jx.04d",0x80000,0x20c4aa2d, 0x10}, // xor decryption table

  {"vm3j.03d" ,0x80000,0x2a2e74a4, 0x10}, // 68000 code (encrypted)
  {"vm3j.04d" ,0x80000,0x1c2427bc, 0x10},
  {"vm3j.05a" ,0x80000,0x95ce88d5, 0x10},
  {"vm3j.06b" ,0x80000,0x2c4297e0, 0x10},
  {"vm3j.07b" ,0x80000,0xa38aaae7, 0x10},
  {"vm3j.08a" ,0x80000,0x5773e5c9, 0x10},
  {"vm3j.09b" ,0x80000,0xd064f8b9, 0x10},
  {"vm3j.10b" ,0x80000,0x434518e9, 0x10},

// In vsav.zip
  // graphics:
  {"vm3.13"  ,0x400000,0xfd8a11eb,    1}, // 10
  {"vm3.15"  ,0x400000,0xdd1e7d4e,    1},
  {"vm3.17"  ,0x400000,0x6b89445e,    1},
  {"vm3.19"  ,0x400000,0x3830fdc7,    1},
  {"vm3.14"  ,0x400000,0xc1a28e6c,    1},
  {"vm3.16"  ,0x400000,0x194a7304,    1},
  {"vm3.18"  ,0x400000,0xdf9a9f47,    1},
  {"vm3.20"  ,0x400000,0xc22fc3d9,    1},

  // sound - z80 roms
  {"vm3.01"  ,0x020000,0xf778769b,    2}, // 18
  {"vm3.02"  ,0x020000,0xcc09faa1,    2},
  // sound - samples
  {"vm3.11"  ,0x400000,0xe80e956e,    2},
  {"vm3.12"  ,0x400000,0x9cd71557,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsVsavj=
{
  {"vsavj" ,"Vampire Savior (Jpn)",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
