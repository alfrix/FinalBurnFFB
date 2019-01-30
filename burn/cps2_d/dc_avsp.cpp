#include "../burnint.h"
// AvsP - Driver
static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Shot"       , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Attack"     , 0, CpsInp001+5, "p1 fire 2"},
  {"P1 Jump"       , 0, CpsInp001+6, "p1 fire 3"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Shot"       , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Attack"     , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Jump"       , 0, CpsInp000+6, "p2 fire 3"},

  {"P3 Coin"       , 0, CpsInp020+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp020+2, "p3 start"},
  {"P3 Up"         , 0, CpsInp011+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp011+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp011+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp011+0, "p3 right"},
  {"P3 Shot"       , 0, CpsInp011+4, "p3 fire 1"},
  {"P3 Attack"     , 0, CpsInp011+5, "p3 fire 2"},
  {"P3 Jump"       , 0, CpsInp011+6, "p3 fire 3"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
  {"Region"        , 2, &Region,     "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int nDrvCount=0;
  nDrvCount=sizeof(DrvInp)/sizeof(DrvInp[0]);
  if (i<nDrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n)
{
  if (n<=0x21) return 2; // music
  return 1; // sound effect
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  4*0x080000;
  nCpsCodeLen= 2*0x080000;
  nCpsGfxLen=   0x0c00000;
  nCpsZRomLen= 1*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<4;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,2+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<2;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,0+i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  Cps2LoadTiles(CpsGfx,10);
  // Just need first half of .14-.20, second half is overdumped
  memcpy(CpsGfx+0x0800000,CpsGfx,0x0400000);
  memset(CpsGfx,0,0x0800000); Cps2LoadTiles(CpsGfx,6);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom,14,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,15,1);
  BurnLoadRom(pqs+0x200000,16,1);
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
  if (Region<=4) { CpsRamFF[0x81c9^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// --------------------- Different versions ------------------
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="avsp.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
// 0
  {"avpux.03d",0x80000,0xd5b01046, 0x10}, // xor decryption table
  {"avpux.04d",0x80000,0x94bd7603, 0x10}, // xor decryption table

// 2
  {"avpu.03d" ,0x80000,0x42757950, 0x10}, // 68000 code (encrypted)
  {"avpu.04d" ,0x80000,0x5abcdee6, 0x10},
  {"avp.05d"  ,0x80000,0xfbfb5d7a, 0x10},
  {"avp.06"   ,0x80000,0x190b817f, 0x10},

// 6
  // graphics:
  {"avp.13"  ,0x200000,0x8f8b5ae4,    1}, // 10
  {"avp.15"  ,0x200000,0xb00280df,    1},
  {"avp.17"  ,0x200000,0x94403195,    1},
  {"avp.19"  ,0x200000,0xe1981245,    1},
  {"avp.14"  ,0x200000,0xebba093e,    1},
  {"avp.16"  ,0x200000,0xfb228297,    1},
  {"avp.18"  ,0x200000,0x34fb7232,    1},
  {"avp.20"  ,0x200000,0xf90baa21,    1},

// 14
  // sound - z80 roms
  {"avp.01"  ,0x020000,0x2d3b4220,    2}, // 18
// 15
  // sound - samples
  {"avp.11"  ,0x200000,0x83499817,    2},
  {"avp.12"  ,0x200000,0xf4110d49,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsAvsp=
{
  {"avsp" ,"Alien Vs. Predator",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};

