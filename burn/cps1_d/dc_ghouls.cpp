#include "../burnint.h"
// Ghouls 'N Ghosts - Driver

static unsigned char DiagonalsOk=1,SkipCheck=1;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Fire"       , 0, CpsInp001+4, "p1 fire 1"},
  {"P1 Jump"       , 0, CpsInp001+5, "p1 fire 2"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Fire"       , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},

  {"Reset"         , 0, &CpsReset  , "reset"},
  {"Diagnostic"    , 0, CpsInp018+6, "diag"},
  {"Service"       , 0, CpsInp018+2, "service"},

  {"Dip A"         , 2, &Cpi01A    , "dip"},
  {"Dip B"         , 2, &Cpi01C    , "dip"},
  {"Dip C"         , 2, &Cpi01E    , "dip"},
  {"P1 Diagonals Ok", 2, &DiagonalsOk, "dip"},
  {"Skip Check"    , 2, &SkipCheck , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n)
{
  if (n<=0x06) return 2; // music
  if (n<=0x08) return 1; // credit/die effect
  if (n<=0x1f) return 2; // music
  return 1; // effect
}

static int DrvFind0(int t)
{
  if (t&0x1000) t|=0x4000;
  t<<=7; return t;
}

static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=  0x100000;
  nCpsGfxLen=  0x300000;
  nCpsZRomLen= 0x010000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  nRet=BurnLoadRom(CpsRom+0x000001,0,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x000000,1,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040001,2,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040000,3,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x080000,4,1); if (nRet!=0) return 1;
  BurnByteswap(CpsRom+0x080000,0x080000);

  // Load graphics roms
  CpsLoadTiles    (CpsGfx         , 5);
  CpsLoadTilesByte(CpsGfx+0x200000, 9);
  CpsLoadTilesByte(CpsGfx+0x280000,17);

  // Load Z80 Rom
  nRet=BurnLoadRom(CpsZRom,25,1);
  nPsndIrqPeriod=(60<<10)/500; //OLDTST
  BurnScode=DrvScode;

  // Extras:
  // Custom tile finder:
  CpsFind0=DrvFind0; // Sprites

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
  static unsigned char Patch[8]={0xf8,0x4e,0x00,0x04,0,0,0,0};

  if (DiagonalsOk)
  {
    // Change diagonals so they allow Arthur to keep running
    if (DrvInp[2].pVal[0])
    {
      if (DrvInp[4].pVal[0]) DrvInp[2].pVal[0]=0; // Up/Left  ==> Left
      if (DrvInp[5].pVal[0]) DrvInp[2].pVal[0]=0; // Up/Right ==> Right
    }
  }

  // Patch to skip memory check
  if (SkipCheck) { memcpy(Patch+4,CpsRom+0x61964,4); memcpy(CpsRom+0x61964,Patch,4); }
  CpsFrame();
  if (SkipCheck) memcpy(CpsRom+0x61964,Patch+4,4);
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="ghouls.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"ghl29.bin" ,0x20000,0x166a58a2, 0x10}, //  0 even 68000 code
  {"ghl30.bin" ,0x20000,0x7ac8407a, 0x10}, //  1 odd
  {"ghl27.bin" ,0x20000,0xf734b2be, 0x10}, //  2 even
  {"ghl28.bin" ,0x20000,0x03d3e714, 0x10}, //  3 odd
  {"ghl17.bin" ,0x80000,0x3ea1b0f2, 0x10}, //  4 both

  // graphics:
  // 5
  {"ghl5.bin"  ,0x80000,0x0ba9c0b0,    1},
  {"ghl7.bin"  ,0x80000,0x5d760ab9,    1},
  {"ghl6.bin"  ,0x80000,0x4ba90b59,    1},
  {"ghl8.bin"  ,0x80000,0x4bdee9de,    1},

  // 9
  {"ghl09.bin" ,0x10000,0xae24bb19,    1},
  {"ghl18.bin" ,0x10000,0xd34e271a,    1},
  {"ghl13.bin" ,0x10000,0x3f70dd37,    1},
  {"ghl22.bin" ,0x10000,0x7e69e2e6,    1},
  {"ghl11.bin" ,0x10000,0x37c9b6c6,    1},
  {"ghl20.bin" ,0x10000,0x2f1345b4,    1},
  {"ghl15.bin" ,0x10000,0x3c2a212a,    1},
  {"ghl24.bin" ,0x10000,0x889aac05,    1},
  {"ghl10.bin" ,0x10000,0xbcc0f28c,    1},
  {"ghl19.bin" ,0x10000,0x2a40166a,    1},
  {"ghl14.bin" ,0x10000,0x20f85c03,    1},
  {"ghl23.bin" ,0x10000,0x8426144b,    1},
  {"ghl12.bin" ,0x10000,0xda088d61,    1},
  {"ghl21.bin" ,0x10000,0x17e11df0,    1},
  {"ghl16.bin" ,0x10000,0xf187ba1c,    1},
  {"ghl25.bin" ,0x10000,0x29f79c78,    1},

  // 25
  {"ghl26.bin" ,0x10000,0x3692f6e5,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsGhouls=
{
  {"ghouls","Ghouls 'N Ghosts",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
