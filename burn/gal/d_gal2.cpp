#include "gal.h"
// Galaxy Force II - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="gforce2.zip"; return 0; }
  return 1;
}

static int DrvZipNameJpn(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="gforce2.zip"; return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="gforce2j.zip"; return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"gf11816.bin" ,0x20000,0x317dd0c2, 0x10}, //    cpu M code (even)
  {"gf11815.bin" ,0x20000,0xf1fb22f1, 0x10}, //               (odd)
  {"gf11875.bin" ,0x20000,0xc81701c6, 0x10}, //    cpu X code (even)
  {"gf11874.bin" ,0x20000,0x5301fd79, 0x10}, //               (odd)
  {"gf11688.bin" ,0x20000,0xc845f2df, 0x10}, //    cpu Y code (even)
  {""            ,0      ,0         ,    0},
  {"gf11687.bin" ,0x20000,0x1cbefbbf, 0x10}, //               (odd)
  {""            ,0      ,0         ,    0},

  {"gf11467.bin" ,0x20000,0x6e60e736,    1}, //  8 line bitmaps
  {"gf11468.bin" ,0x20000,0x74ca9ca5,    1}, //  9
  {"gf11694.bin" ,0x20000,0x7e297b84,    1}, // 10
  {"gf11695.bin" ,0x20000,0x38a864be,    1}, // 11

  {"gf11469.bin" ,0x20000,0xed7a2299,    1}, // 12 sprite bitmaps
  {"gf11470.bin" ,0x20000,0x34dea550,    1}, // 13
  {"gf11477.bin" ,0x20000,0xa2784653,    1}, // 14
  {"gf11478.bin" ,0x20000,0x8b778993,    1}, // 15
  {"gf11471.bin" ,0x20000,0xf1974069,    1}, // 16
  {"gf11472.bin" ,0x20000,0x0d24409a,    1}, // 17
  {"gf11479.bin" ,0x20000,0xecd6138a,    1}, // 18
  {"gf11480.bin" ,0x20000,0x64ad66c5,    1}, // 19

  {"gf11473.bin" ,0x20000,0x0538c6ec,    1}, // 20
  {"gf11474.bin" ,0x20000,0xeb923c50,    1}, // 21
  {"gf11481.bin" ,0x20000,0x78e652b6,    1}, // 22
  {"gf11482.bin" ,0x20000,0x2f879766,    1}, // 23
  {"gf11475.bin" ,0x20000,0x69cfec89,    1}, // 24
  {"gf11476.bin" ,0x20000,0xa60b9b79,    1}, // 25
  {"gf11483.bin" ,0x20000,0xd5d3a505,    1}, // 26
  {"gf11484.bin" ,0x20000,0xb8a56a50,    1}, // 27

  {"gf11696.bin" ,0x20000,0x99e8e49e,    1}, // 28
  {"gf11697.bin" ,0x20000,0x7545c52e,    1}, // 29
  {"gf11700.bin" ,0x20000,0xe13839c1,    1}, // 30
  {"gf11701.bin" ,0x20000,0x9fb3d365,    1}, // 31
  {"gf11698.bin" ,0x20000,0xcfeba3e2,    1}, // 32
  {"gf11699.bin" ,0x20000,0x4a00534a,    1}, // 33
  {"gf11702.bin" ,0x20000,0x2a09c627,    1}, // 34
  {"gf11703.bin" ,0x20000,0x43bb7d9f,    1}, // 35

  {"gf11524.bin" ,0x20000,0x5d35849f,    1}, // 36
  {"gf11525.bin" ,0x20000,0x9ae47552,    1}, // 37
  {"gf11532.bin" ,0x20000,0xb3565ddb,    1}, // 38
  {"gf11533.bin" ,0x20000,0xf5d16e8a,    1}, // 39
  {"gf11526.bin" ,0x20000,0x094cb3f0,    1}, // 40
  {"gf11527.bin" ,0x20000,0xe821a144,    1}, // 41
  {"gf11534.bin" ,0x20000,0xb7f0ad7c,    1}, // 42
  {"gf11535.bin" ,0x20000,0x95da7a46,    1}, // 43

  {"gf11693.bin" ,0x10000,0x0497785c,    2}, // 44 z80 program
  {"gf11465.bin" ,0x80000,0xe1436dab,    2}, // 45
  {"gf11516.bin" ,0x20000,0x19d0e17f,    2}, // 46
  {"gf11814.bin" ,0x20000,0x0b05d376,    2}  // 47
};

// Rom information for the different japanese roms
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDescJpn[]=
{
  {"11513.epr" ,0x20000,0xe18bc177,    1}, //  0
  {"11512.epr" ,0x20000,0x6010e63e,    1}, //  1
  {"11515.epr" ,0x20000,0xd85875cf,    1}, //  2
  {"11514.epr" ,0x20000,0x3dcc6919,    1}, //  3
  {"11511.epr" ,0x20000,0xd80a86d6,    1}, //  4
  {""          ,0      ,0         ,    0},
  {"11510.epr" ,0x20000,0xd2b1bef4,    1}, //  6
  {""          ,0      ,0         ,    0},
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

static int DrvRomInfoJpn(struct BurnRomInfo *pri,unsigned int i)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  // Return information about the rom
  if (pri!=NULL)
  {
    if (i<8)
    {
      // Pick japanese roms
      pri->nLen =DrvRomDescJpn[i].nLen;
      pri->nCrc =DrvRomDescJpn[i].nCrc;
      pri->nType=DrvRomDescJpn[i].nType;
    }
    else
    {
      pri->nLen =DrvRomDesc[i].nLen;
      pri->nCrc =DrvRomDesc[i].nCrc;
      pri->nType=DrvRomDesc[i].nType;
    }
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

static int DrvRomNameJpn(char **pszName,unsigned int i,int nAka)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  if (nAka!=0) return 1; // Roms known by one name
  if (i<8) *pszName=DrvRomDescJpn[i].szName; // Pick japanese roms
  else     *pszName=DrvRomDesc[i].szName;
  return 0;
}

static struct BurnInputInfo DrvInp[]=
{
  {"Coin 1"     , 0, GalButton+6, "p1 coin"},
  {"Start"      , 0, GalButton+3, "p1 start"},
  {"Left/Right" , 1, GalAnalog+0, "p1 x-axis"},
  {"Up/Down"    , 1, GalAnalog+1, "p1 y-axis"},
  {"Shot"       , 0, GalButton+4, "p1 fire 1"},
  {"Missile"    , 0, GalButton+5, "p1 fire 2"},
  {"Throttle"   , 1, GalAnalog+2, "p1 z-axis"},

  {"Coin 2"     , 0, GalButton+7, "p2 coin"},

  {"Reset"      , 0, &GalReset  , "reset"},
  {"Diagnostic" , 0, GalButton+1, "diag"},
  {"Service"    , 0, GalButton+2, "service"},
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

static int DrvScode(unsigned int n) { if (n>=0xb5 && n<=0xbf) return 2;  return 1; }

static int DrvInit()
{
  GalGame=0x10; // GF
  GalDip[0]=0; GalDip[1]=0x89;
  BurnScode=DrvScode;
  return GalInit();
}

static int DrvjInit()
{
  GalGame=0x11; // GF (jpn)
  GalDip[0]=0; GalDip[1]=0x89;
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

struct BurnDriver BurnDrvGforce2=
{
  {"gforce2","Galaxy Force II","Glitches, no tilt"},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit ,DrvExit,GalFrame,GalScan,NULL,320,224
};

struct BurnDriver BurnDrvGforce2Jpn=
{
  {"gforce2j","Galaxy Force II (Jpn)","Glitches, no tilt"},
  DrvZipNameJpn,DrvRomInfoJpn,DrvRomNameJpn,DrvInputInfo,
  DrvjInit,DrvExit,GalFrame,GalScan,NULL,320,224
};
