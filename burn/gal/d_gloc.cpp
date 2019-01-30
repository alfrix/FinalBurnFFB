#include "gal.h"
// G-LOC - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="g-loc.zip"; return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"13030"     ,0x20000,0x81abcabf, 0x10}, //  0  cpu M code (even)
  {"13029"     ,0x20000,0xf3638efb, 0x10}, //  1             (odd)
  {"13032"     ,0x20000,0x7da09c4e, 0x10}, //  2  cpu X code (even)
  {"13031"     ,0x20000,0xf3c7e3f4, 0x10}, //  3             (odd)
  {"13170"     ,0x20000,0x45189229, 0x10}, //  4  cpu Y code (even)
  {"13028"     ,0x20000,0xb6aa2edf, 0x10},
  {"13169"     ,0x20000,0x1b47cd6e, 0x10}, //  6             (odd)
  {"13027"     ,0x20000,0x6463c87a, 0x10},

  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},

  {"13048"     ,0x80000,0xfe1eb0dd,    1}, // 12  sprite bitmaps
  {"13056"     ,0x80000,0x5904f8e6,    1},
  {"13044"     ,0x80000,0x4d931f89,    1},
  {"13052"     ,0x80000,0x0291f040,    1},
  {"13064"     ,0x80000,0x5f8e651b,    1},
  {"13072"     ,0x80000,0x6b85641a,    1},
  {"13060"     ,0x80000,0xee16ad97,    1},
  {"13068"     ,0x80000,0x64d52bbb,    1},

  {"13047"     ,0x80000,0x53340832,    1},
  {"13055"     ,0x80000,0x39b6b665,    1},
  {"13043"     ,0x80000,0x208f16fd,    1},
  {"13051"     ,0x80000,0xad62cbd4,    1},
  {"13063"     ,0x80000,0xc580bf6d,    1},
  {"13071"     ,0x80000,0xdf99ef99,    1},
  {"13059"     ,0x80000,0x4c982558,    1},
  {"13067"     ,0x80000,0xf97f6119,    1},

  {"13046"     ,0x80000,0xc75a86e9,    1},
  {"13054"     ,0x80000,0x2934549a,    1},
  {"13042"     ,0x80000,0x53ed97af,    1},
  {"13050"     ,0x80000,0x04429068,    1},
  {"13062"     ,0x80000,0x4fdb4ee3,    1},
  {"13070"     ,0x80000,0x52ea130e,    1},
  {"13058"     ,0x80000,0x19ff1626,    1},
  {"13066"     ,0x80000,0xbc70a250,    1},

  {"13045"     ,0x80000,0x54d5bc6d,    1},
  {"13053"     ,0x80000,0x9502af13,    1},
  {"13041"     ,0x80000,0xd0a7402c,    1},
  {"13049"     ,0x80000,0x5b9c0b6c,    1},
  {"13061"     ,0x80000,0x7b95ec3b,    1},
  {"13069"     ,0x80000,0xe1f538f0,    1},
  {"13057"     ,0x80000,0x73baefee,    1},
  {"13065"     ,0x80000,0x8937a655,    1},

  {"13033"     ,0x10000,0x6df5e827,    2}, // 44
  {"13034"     ,0x80000,0xcd22d95d,    2}, // 45
  {"13035"     ,0x80000,0x009fa13e,    2},
  {"13036"     ,0x80000,0x7890c26c,    2},
};

// Return 0 if a rom number is defined, and (if pri!=NULL) information about each rom
static int DrvRomInfo(struct BurnRomInfo *pri,unsigned int i)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  // Return information about the rom
  if (pri!=NULL)
  {
    pri->nLen =DrvRomDesc[i].nLen;
    pri->nCrc =DrvRomDesc[i].nCrc;
    pri->nType=DrvRomDesc[i].nType;
  }
  return 0;
}

// Return possible names for rom number 'i'
static int DrvRomName(char **pszName,unsigned int i,int nAka)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  if (nAka!=0) return 1; // Roms known by one name
  *pszName=DrvRomDesc[i].szName;
  return 0;
}

static struct BurnInputInfo DrvInp[]=
{
  {"Diagnostic" , 0, GalButton+1, "diag"},
  {"Service"    , 0, GalButton+2, "service"},
  {"Start"      , 0, GalButton+3, "p1 start"},
  {"Vulcan"     , 0, GalButton+4, "p1 fire 1"},
  {"Missile"    , 0, GalButton+5, "p1 fire 2"},
  {"Coin 1"     , 0, GalButton+6, "p1 coin"},
  {"Coin 2"     , 0, GalButton+7, "p2 coin"},

  {"Left/Right" , 1, GalAnalog+5, "p1 x-axis"},
  {"Up/Down"    , 1, GalAnalog+3, "p1 y-axis"},
  {"Throttle"   , 1, GalAnalog+4, "p1 z-axis"},

  {"Reset"      , 0, &GalReset  , "reset"},
  {"Dip A"      , 2, GalDip+0   , "dip"},
  {"Dip B"      , 2, GalDip+1   , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvInit()
{
  GalGame=0x40; // G-Loc
  GalDip[0]=0; GalDip[1]=0x85;
  return GalInit();
}

struct BurnDriver BurnDrvGLoc=
{
  {"g-loc","G-Loc","w.i.p."},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,GalExit,GalFrame,GalScan,
  NULL,320,224
};
