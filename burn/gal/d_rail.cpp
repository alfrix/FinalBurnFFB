#include "gal.h"
// Rail Chase - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="railchaj.zip"; return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"ic54.bin"  ,0x20000,0x18eb23c5, 0x10}, //  0  cpu M code (even)
  {"ic53.bin"  ,0x20000,0x8f4f824e, 0x10}, //  1             (odd)
  {"ic81.bin"  ,0x20000,0xc5d525b6, 0x10}, //  2  cpu X code (even)
  {"ic80.bin"  ,0x20000,0x299e3c7c, 0x10}, //  3             (odd)
  {"ic25.bin"  ,0x20000,0x388b2365, 0x10}, //  4  cpu Y code (even)
  {"ic27.bin"  ,0x20000,0xdc1cd5a4, 0x10}, //  5
  {"ic24.bin"  ,0x20000,0x14dba5d4, 0x10}, //  6             (odd)
  {"ic26.bin"  ,0x20000,0x969fdb3a, 0x10}, //  7

  {"vic16.bin" ,0x40000,0x9a1dd53c,    1}, //  8  line bitmaps
  {"vic14.bin" ,0x40000,0x1fdf1b87,    1}, //  9
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},

  {"vic67.bin" ,0x80000,0x9fa88781,    1}, // 12  sprite bitmaps
  {"vic75.bin" ,0x80000,0x49e824bb,    1}, // 13
  {"vic63.bin" ,0x80000,0x35b5187e,    1}, // 14
  {"vic71.bin" ,0x80000,0x9a538b9b,    1}, // 15
  {"vic86.bin" ,0x80000,0xe11c6c67,    1}, // 16
  {"vic114.bin",0x80000,0x16344535,    1}, // 17
  {"vic82.bin" ,0x80000,0x78e9983b,    1}, // 18
  {"vic110.bin",0x80000,0xe9daa1a4,    1}, // 19

  {"vic66.bin" ,0x80000,0xb83df159,    1}, // 20
  {"vic74.bin" ,0x80000,0x76dbe9ce,    1}, // 21
  {"vic62.bin" ,0x80000,0x9e998209,    1}, // 22
  {"vic70.bin" ,0x80000,0x2caddf1a,    1}, // 23
  {"vic85.bin" ,0x80000,0xb15e19ff,    1}, // 24
  {"vic113.bin",0x80000,0x84c7008f,    1}, // 25
  {"vic81.bin" ,0x80000,0xc3cf5faa,    1}, // 26
  {"vic109.bin",0x80000,0x7e91beb2,    1}, // 27

  {"vic65.bin" ,0x80000,0x31dbb2c3,    1}, // 28
  {"vic73.bin" ,0x80000,0x7e68257d,    1}, // 29
  {"vic61.bin" ,0x80000,0x71031ad0,    1}, // 30
  {"vic69.bin" ,0x80000,0x27e70a5e,    1}, // 31
  {"vic84.bin" ,0x80000,0x7540bf85,    1}, // 32
  {"vic112.bin",0x80000,0x7d87b94d,    1}, // 33 (512k version was dumped badly)
  {"vic80.bin" ,0x80000,0x87725d74,    1}, // 34
  {"vic108.bin",0x80000,0x73477291,    1}, // 35

  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},
  {""          ,0      ,0         ,    0},

  {"ic102.bin" ,0x10000,0x7cc3b543,    2}, // 44
  {"ic105.bin" ,0x80000,0x76095538,    2}, // 45
  {"ic106.bin" ,0x80000,0xf604c270,    2}, // 46
  {"ic107.bin" ,0x80000,0x345f5a41,    2}, // 47
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
  {"P1 Coin"     , 0, GalButton+3, "p1 coin"},
  {"P1 Fire"      ,0, GalButton+0, "p1 fire 1"},
  {"P1 Left/Right",1, GalAnalog+2, "p1 x-axis"},
  {"P1 Up/Down"   ,1, GalAnalog+3, "p1 y-axis"},

  {"Reset"       , 0, &GalReset  , "reset"},
  {"Diagnostic"  , 0, GalButton+2, "diag"},
  {"Dip 0"       , 2, GalDip+0   , "dip"},
  {"Dip 1"       , 2, GalDip+1   , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n) { if (n>=0x81 && n<=0x8A) return 2;   return 1; }

static int DrvInit()
{
  GalGame=0x30; // Rail Chase
  GalDip[0]=0; GalDip[1]=0;
  BurnScode=DrvScode;
  return GalInit();
}

static int DrvExit()
{
  GalExit();
  BurnScode=NULL;
  GalDip[0]=0; GalDip[1]=0;
  GalGame=0;
  return 0;
}

struct BurnDriver BurnDrvRailCha=
{
  {"railchaj","Rail Chase (Jpn)","Some glitches"},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,GalFrame,GalScan,NULL,320,224
};
