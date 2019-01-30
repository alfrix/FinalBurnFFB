#include "../burnint.h"
// Street Fighter Zero (CPS Changer) - Driver

static unsigned char Region=0xff;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Pause"      , 0, CpsInp018+2, ""},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Low Punch"  , 0, CpsInp001+4, "p1 fire 4"},
  {"P1 Mid Punch"  , 0, CpsInp001+5, "p1 fire 5"},
  {"P1 High Punch" , 0, CpsInp001+6, "p1 fire 6"},
  {"P1 Low Kick"   , 0, CpsInp001+7, "p1 fire 1"},
  {"P1 Mid Kick"   , 0, CpsInp018+0, "p1 fire 2"},
  {"P1 High Kick"  , 0, CpsInp018+6, "p1 fire 3"},

  {"P2 Pause"      , 0, CpsInp018+3, ""},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Low Punch"  , 0, CpsInp000+4, "p2 fire 4"},
  {"P2 Mid Punch"  , 0, CpsInp000+5, "p2 fire 5"},
  {"P2 High Punch" , 0, CpsInp000+6, "p2 fire 6"},
  {"P2 Low Kick"   , 0, CpsInp000+7, "p2 fire 1"},
  {"P2 Mid Kick"   , 0, CpsInp018+1, "p2 fire 2"},
  {"P2 High Kick"  , 0, CpsInp018+7, "p2 fire 3"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Service"       , 0, CpsInp018+2, "service"},
  {"Region"        , 2, &Region,     "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n) { (void)n; return 0; }

static int DrvInit()
{
  int nRet=0; int i=0;
  Cps=1;
  nCpsRomLen=   0x200000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x800000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen  =  0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (they are already byteswapped)
  for (i=0;i<4;i++)
  { nRet=BurnLoadRom(CpsRom+0x080000*i,0+i,1); if (nRet!=0) return 1; }

  // Load graphics roms
  for (i=0;i<4;i++)
  {
    // Load up and interleve each set of 4 roms to make the 16x16 tiles
    CpsLoadTiles(CpsGfx+0x200000*i,4+i*4);
  }

  // Load Z80 Rom
  nRet=BurnLoadRom(CpsZRom,20,1);
  nPsndIrqPeriod=(60<<10)/500; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  nRet=BurnLoadRom(CpsAd        ,21,1);
  nRet=BurnLoadRom(CpsAd+0x20000,22,1);

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
  if (Region<=5) { CpsRamFF[0x8191^1]=(unsigned char)(Region<<1); }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="sfzch.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"sfzch23"  ,0x80000,0x1140743f, 0x10}, //  0 68000 code
  {"sfza22"   ,0x80000,0x8d9b2480, 0x10}, //  1
  {"sfzch21"  ,0x80000,0x5435225d, 0x10}, //  2
  {"sfza20"   ,0x80000,0x806e8f38, 0x10}, //  3

  // graphics:
  {"sfz01"    ,0x80000,0x0dd53e62,    1},
  {"sfz02"    ,0x80000,0x94c31e3f,    1},
  {"sfz03"    ,0x80000,0x9584ac85,    1},
  {"sfz04"    ,0x80000,0xb983624c,    1},

  {"sfz05"    ,0x80000,0x2b47b645,    1},
  {"sfz06"    ,0x80000,0x74fd9fb1,    1},
  {"sfz07"    ,0x80000,0xbb2c734d,    1},
  {"sfz08"    ,0x80000,0x454f7868,    1},

  {"sfz10"    ,0x80000,0x2a7d675e,    1},
  {"sfz11"    ,0x80000,0xe35546c8,    1},
  {"sfz12"    ,0x80000,0xf122693a,    1},
  {"sfz13"    ,0x80000,0x7cf942c8,    1},

  {"sfz14"    ,0x80000,0x09038c81,    1},
  {"sfz15"    ,0x80000,0x1aa17391,    1},
  {"sfz16"    ,0x80000,0x19a5abd6,    1},
  {"sfz17"    ,0x80000,0x248b3b73,    1},
//20
  {"sfz09"    ,0x10000,0xc772628b,    2},
  {"sfz18"    ,0x20000,0x61022b2d,    2},
  {"sfz19"    ,0x20000,0x3b5886d5,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSfzch=
{
  {"sfzch","Street Fighter Zero (CPS Changer)",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
