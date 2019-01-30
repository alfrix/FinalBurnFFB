#include "../burnint.h"
// Saturday Night Slammasters II - Driver

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

static int DrvScode(unsigned int n) { if (n<=0x2f) return 2;  return 1; }

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  6*0x080000;
  nCpsCodeLen= 3*0x080000;
  nCpsGfxLen=   0x1200000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<6;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,3+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<3;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  nRet=Cps2LoadTiles(CpsGfx           , 9);
  nRet=Cps2LoadTiles(CpsGfx+ 0x0800000,13);
  nRet=Cps2LoadTiles(CpsGfx+ 0x1000000,17);

  // Load Z80 Roms
  nRet=BurnLoadRom(CpsZRom         ,21,1);
  nRet=BurnLoadRom(CpsZRom+0x020000,22,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  nRet=BurnLoadRom(pqs         ,23,1);
  nRet=BurnLoadRom(pqs+0x200000,24,1);
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
  if (Region<=0x5 || Region==0xa) CpsRamFF[0x7445^1]=Region;

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="slam2e.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"smbex.03b",0x80000,0x3b1457bd, 0x10}, // xor decryption tables
  {"smbex.04b",0x80000,0x6299eb4e, 0x10},
  {"smbex.05b",0x80000,0xbe4a84d1, 0x10},

  {"smbe.03b" ,0x80000,0xb8016278, 0x10}, // 68000 code (encrypted)
  {"smbe.04b" ,0x80000,0x18c4c447, 0x10},
  {"smbe.05b" ,0x80000,0x18ebda7f, 0x10},
  {"smbe.06b" ,0x80000,0x89c80007, 0x10},
  {"smb.07b"  ,0x80000,0xb9a11577, 0x10},
  {"smb.08b"  ,0x80000,0xf931b76b, 0x10},

  // graphics:
  {"smb.13"  ,0x200000,0xd9b2d1de,    1}, // 9
  {"smb.15"  ,0x200000,0x9a766d92,    1},
  {"smb.17"  ,0x200000,0x51800f0f,    1},
  {"smb.19"  ,0x200000,0x35757e96,    1},
  {"smb.14"  ,0x200000,0xe5bfd0e7,    1},
  {"smb.16"  ,0x200000,0xc56c0866,    1},
  {"smb.18"  ,0x200000,0x4ded3910,    1},
  {"smb.20"  ,0x200000,0x26ea1ec5,    1},
  {"smb.21"  ,0x080000,0x0a08c5fc,    1},
  {"smb.23"  ,0x080000,0x0911b6c4,    1},
  {"smb.25"  ,0x080000,0x82d6c4ec,    1},
  {"smb.27"  ,0x080000,0x9b48678b,    1},

  // sound - z80 roms
  {"smb.01"  ,0x020000,0x0abc229a,    2}, // 21
  {"smb.02"  ,0x020000,0xd051679a,    2},
  // sound - samples
  {"smb.11"  ,0x200000,0xc56935f9,    2},
  {"smb.12"  ,0x200000,0x955b0782,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSlam2e=
{
  {"slam2e" ,"Slammasters II - Ring Of Destruction (Euro)",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
