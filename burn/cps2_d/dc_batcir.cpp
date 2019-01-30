#include "../burnint.h"
// Battle Circuit

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Attack"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Jump"       , 0, CpsInp001+5, "p1 fire 2"},

  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Attack"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},

  {"P3 Coin"       , 0, CpsInp020+6, "p3 coin"},
  {"P3 Start"      , 0, CpsInp020+2, "p3 start"},
  {"P3 Up"         , 0, CpsInp011+3, "p3 up"},
  {"P3 Down"       , 0, CpsInp011+2, "p3 down"},
  {"P3 Left"       , 0, CpsInp011+1, "p3 left"},
  {"P3 Right"      , 0, CpsInp011+0, "p3 right"},
  {"P3 Attack"     , 0, CpsInp011+4, "p3 fire 1"},
  {"P3 Jump"       , 0, CpsInp011+5, "p3 fire 2"},

  {"P4 Coin"       , 0, CpsInp020+7, "p4 coin"},
  {"P4 Start"      , 0, CpsInp020+3, "p4 start"},
  {"P4 Up"         , 0, CpsInp010+3, "p4 up"},
  {"P4 Down"       , 0, CpsInp010+2, "p4 down"},
  {"P4 Left"       , 0, CpsInp010+1, "p4 left"},
  {"P4 Right"      , 0, CpsInp010+0, "p4 right"},
  {"P4 Attack"     , 0, CpsInp010+4, "p4 fire 1"},
  {"P4 Jump"       , 0, CpsInp010+5, "p4 fire 2"},
 
  {"Reset"         , 0, &CpsReset   , "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
  {"Region"        , 2, &Region,     "dip"},
};

static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  unsigned int DrvCount=0;
  DrvCount=sizeof(DrvInp)/sizeof(DrvInp[0]);
  if (i<DrvCount) { if (pii!=NULL) *pii=DrvInp[i];  return 0; }
  return 1; // Out of range
}

static int DrvScode(unsigned int n)
{
  if (n<=0x2f) return 2;
  return 1;
}

static int DrvInit()
{
  int Ret=0; int i=0; unsigned char *pqs=NULL;

  Cps=2;
  nCpsRomLen=  7*0x080000;
  nCpsCodeLen= 4*0x080000;
  nCpsGfxLen=  4*0x400000;
  nCpsZRomLen= 2*0x020000;
  nCpsQSamLen= 2*0x200000;
  Ret=CpsInit(); if (Ret!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<7;i++)
  { Ret=BurnLoadRom(CpsRom+0x080000*i,4+i,1); if (Ret!=0) return 1; }

  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<4;i++)
  { Ret=BurnXorRom(CpsCode+0x080000*i,0+i,1); if (Ret!=0) return 1; }

  // Load graphics roms
  Ret=Cps2LoadTiles(CpsGfx           ,11);

  // Load Z80 Roms
  Ret=BurnLoadRom(CpsZRom         ,15,1);
  Ret=BurnLoadRom(CpsZRom+ 0x20000,16,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  Ret=BurnLoadRom(pqs         ,17,1);
  Ret=BurnLoadRom(pqs+0x200000,18,1);
  BurnByteswap(pqs,nCpsQSamLen);
  BurnScode=DrvScode;

  Ret=CpsRunInit(); if (Ret!=0) return 1;
  // Ready to go
  return 0;
}

static int DrvExit()
{
  CpsRunExit(); CpsExit();
  return 0;
}

static int DrvFrame()
{
  // Insert Region code into RAM
  if (Region<=5) CpsRamFF[0x58c4^1]=Region;

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="batcirj.zip";  return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"btcjx.03",0x80000,0x01482d08, 0x10}, // xor decryption tables
  {"btcjx.04",0x80000,0x3d4c976b, 0x10},
  {"btcjx.05",0x80000,0x5bf819e1, 0x10},
  {"btcjx.06",0x80000,0x5d2fd190, 0x10},

  {"btcj.03" ,0x80000,0x6b7e168d, 0x10}, // 68000 code (encrypted)
  {"btcj.04" ,0x80000,0x46ba3467, 0x10},
  {"btcj.05" ,0x80000,0x0e23a859, 0x10},
  {"btcj.06" ,0x80000,0xa853b59c, 0x10},
  {"btc.07"  ,0x80000,0x7322d5db, 0x10},
  {"btc.08"  ,0x80000,0x6aac85ab, 0x10},
  {"btc.09"  ,0x80000,0x1203db08, 0x10},

  // graphics:
  {"btc.13" ,0x400000,0xdc705bad,    1}, // 10
  {"btc.15" ,0x400000,0xe5779a3c,    1},
  {"btc.17" ,0x400000,0xb33f4112,    1},
  {"btc.19" ,0x400000,0xa6fcdb7e,    1},

  // sound - z80 roms
  {"btc.01" ,0x020000,0x1e194310,    2}, // 18
  {"btc.02" ,0x020000,0x01aeb8e6,    2},
  // sound - samples
  {"btc.11" ,0x200000,0xc27f2229,    2},
  {"btc.12" ,0x200000,0x418a2e33,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsBatcirj=
{
  {"batcirj" ,"Battle Circuit (Japan)",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
