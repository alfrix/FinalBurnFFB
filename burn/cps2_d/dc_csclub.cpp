#include "../burnint.h"
// Capcom Sports Club - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp020+4, "p1 coin"},
  {"P1 Start"      , 0, CpsInp020+0, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Shot 1"     , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Shot 2"     , 0, CpsInp001+5, "p1 fire 2"},
  {"P1 Shot 3"     , 0, CpsInp001+6, "p1 fire 3"},
  
  {"P2 Coin"       , 0, CpsInp020+5, "p2 coin"},
  {"P2 Start"      , 0, CpsInp020+1, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Shot 1"     , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Shot 2"     , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Shot 3"     , 0, CpsInp000+6, "p2 fire 3"},

  {"Reset"         , 0, &CpsReset,   "reset"},
  {"Diagnostic"    , 0, CpsInp021+1, "diag"},
  {"Service"       , 0, CpsInp021+2, "service"},
  {"Region"        , 2, &Region,  "dip"},
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
  if (n>=0x278) n-=0x278;
  if (n<0x28) return 2;
  return 1;
}

static int DrvInit()
{
  int nRet=0; int i=0; unsigned char *pqs=NULL;
  Cps=2;
  nCpsRomLen=  5*0x080000;
  nCpsCodeLen= 4*0x080000;
  nCpsGfxLen=    0x800000;
  nCpsZRomLen= 1*0x020000;
  nCpsQSamLen= 2*0x200000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (as they are on the roms) for CpsRom
  for (i=0;i<5;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,4+i,1); if (nRet!=0) return 1; }

  // Make decrypted rom
  memcpy(CpsCode,CpsRom,nCpsCodeLen);
  for (i=0;i<4;i++)
  { nRet=BurnXorRom(CpsCode+0x080000*i,i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  Cps2LoadTiles(CpsGfx,9);

  // Load Z80 Roms
  BurnLoadRom(CpsZRom,13,1);

  // Load Q Sample Roms
  pqs=(unsigned char *)CpsQSam;
  BurnLoadRom(pqs         ,14,1);
  BurnLoadRom(pqs+0x200000,15,1);
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
  // Insert region code into RAM
  if (Region<=7) { CpsRamFF[0x8c0b^1]=Region; }
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="csclubj.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
  {"cscjx.03" ,0x80000,0x2de1d45d, 0x10}, // 0, xor decryption table
  {"cscjx.04" ,0x80000,0x81b25d76, 0x10},
  {"cscjx.05" ,0x80000,0x5adb1c93, 0x10},
  {"cscjx.06" ,0x80000,0xf5558f79, 0x10},

  {"cscj.03"  ,0x80000,0xec4ddaa2, 0x10}, // 4, 68000 code (encrypted)
  {"cscj.04"  ,0x80000,0x60c632bb, 0x10},
  {"cscj.05"  ,0x80000,0xad042003, 0x10},
  {"cscj.06"  ,0x80000,0x169e4d40, 0x10}, 
  {"csc.07"   ,0x80000,0x01b05caa, 0x10}, 

  // graphics:
  {"csc.14"  ,0x200000,0xe8904afa,    1}, // 9
  {"csc.16"  ,0x200000,0xc98c8079,    1},
  {"csc.18"  ,0x200000,0xc030df5a,    1},
  {"csc.20"  ,0x200000,0xb4e55863,    1},

  // sound - z80 roms
  {"csc.01"  ,0x020000,0xee162111,    2}, // 13
  // sound - samples
  {"csc.11"  ,0x200000,0xa027b827,    2},
  {"csc.12"  ,0x200000,0xcb7f6e55,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsCsclubj=
{
  {"csclubj","Capcom Sports Club (Jpn)",""},
  DrvZipName, DrvRomInfo, DrvRomName, DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
// -------------------------------------------------------
