#include "../burnint.h"
// Mega Man - Driver

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
  {"P1 Fire 3"     , 0, CpsInp001+6, "p1 fire 3"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Fire"       , 0, CpsInp000+4, "p2 fire 1"},
  {"P2 Jump"       , 0, CpsInp000+5, "p2 fire 2"},
  {"P2 Fire 3"     , 0, CpsInp000+6, "p2 fire 3"},

  {"Reset"         , 0, &CpsReset,   "reset"},
  {"Diagnostic"    , 0, CpsInp018+6, "diag"},
  {"Service"       , 0, CpsInp018+2, "service"},

  {"Dip A"         , 2, &Cpi01A    , "dip"},
  {"Dip B"         , 2, &Cpi01C    , "dip"},
  {"Dip C"         , 2, &Cpi01E    , "dip"},
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
  if (n>=0x196) n=n%0x196; // Sound codes repeated every 0x196
  if (n>=0x0001 && n<0x001f) return 2;
  return 1;
}

static int DrvInit()
{
  int nRet=0; int i=0;
  Cps=1;
  nCpsRomLen= 3*0x080000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x800000;
  nCpsZRomLen=  0x020000;
  nCpsAdLen  =2*0x020000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms (they are already byteswapped)
  for (i=0;i<3;i++)
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

  Cpi01C=0x05; // Dip switches

  // Ready to go
  return CpsRunInit();
}

static int DrvExit()
{
  CpsRunExit(); CpsExit();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="megaman.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"rcma_23b.rom",0x80000,0x61e4a397, 0x10}, //  0 68000 code
  {"rcma_22b.rom",0x80000,0x708268c4, 0x10}, //  1
  {"rcma_21a.rom",0x80000,0x4376ea95, 0x10}, //  2
  {""            ,0      ,0         , 0   },

//4
  // graphics:
  {"rcm_01.rom",0x80000,0x6ecdf13f,    1},
  {"rcm_02.rom",0x80000,0x944d4f0f,    1},
  {"rcm_03.rom",0x80000,0x36f3073c,    1},
  {"rcm_04.rom",0x80000,0x54e622ff,    1},

  {"rcm_05.rom",0x80000,0x5dd131fd,    1},
  {"rcm_06.rom",0x80000,0xf0faf813,    1},
  {"rcm_07.rom",0x80000,0x826de013,    1},
  {"rcm_08.rom",0x80000,0xfbff64cf,    1},

  {"rcm_10.rom",0x80000,0x4dc8ada9,    1},
  {"rcm_11.rom",0x80000,0xf2b9ee06,    1},
  {"rcm_12.rom",0x80000,0xfed5f203,    1},
  {"rcm_13.rom",0x80000,0x5069d4a9,    1},

  {"rcm_14.rom",0x80000,0x303be3bd,    1},
  {"rcm_15.rom",0x80000,0x4f2d372f,    1},
  {"rcm_16.rom",0x80000,0x93d97fde,    1},
  {"rcm_17.rom",0x80000,0x92371042,    1},

//20
  {"rcm_09.rom",0x20000,0x9632d6ef,    2},
  {"rcm_18.rom",0x20000,0x80f1f8aa,    2},
  {"rcm_19.rom",0x20000,0xf257dbe1,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsMegaman=
{
  {"megaman","Mega Man - The Power Battle (Asia) ",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,CpsFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
