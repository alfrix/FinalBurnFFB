#include "gal.h"
// Power Drift - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="pwrdrift.zip"; return 0; }
  return 1;
}

static int DrvZipNameJpn(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="pwrdrift.zip"; return 0; }
  if (i==1) { if (pszName!=NULL) *pszName="pwrdrifj.zip";  return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"pd12019a.bin",0x20000,0x11188a30, 0x10}, //  0 cpu M code (even)
  {"pd12018a.bin",0x20000,0x1c582e1f, 0x10}, //  1            (odd)
  {"pd11905.bin" ,0x20000,0x1cf68109, 0x10}, //  2 cpu X code (even)
  {"pd11904.bin" ,0x20000,0xbb993681, 0x10}, //  3            (odd)
  {"pd12017.bin" ,0x20000,0x31190322, 0x10}, //  4 cpu Y code (even)
  {"pd11748.bin" ,0x20000,0x82a76cab, 0x10}, //  5             extra code?
  {"pd12016.bin" ,0x20000,0x499f64a6, 0x10}, //  6            (odd)
  {"pd11747.bin" ,0x20000,0x9796ece5, 0x10}, //  7             extra code?

  {"pd11789.bin" ,0x20000,0xb86f8d2b,    1}, //  8 line bitmaps
  {"pd11791.bin" ,0x20000,0x36b2910a,    1}, //  9
  {"pd11790.bin" ,0x20000,0x2a564e66,    1}, // 10
  {"pd11792.bin" ,0x20000,0xc85caf6e,    1}, // 11

  {"pd11757.bin" ,0x20000,0xe46dc478,    1}, // 12 sprite bitmaps
  {"pd11758.bin" ,0x20000,0x5b435c87,    1}, // 13
  {"pd11773.bin" ,0x20000,0x1b5d5758,    1}, // 14
  {"pd11774.bin" ,0x20000,0x2ca0c170,    1}, // 15
  {"pd11759.bin" ,0x20000,0xac8111f6,    1}, // 16
  {"pd11760.bin" ,0x20000,0x91282af9,    1}, // 17
  {"pd11775.bin" ,0x20000,0x48225793,    1}, // 18
  {"pd11776.bin" ,0x20000,0x78c46198,    1}, // 19

  {"pd11761.bin" ,0x20000,0xbaa5d065,    1}, // 20
  {"pd11762.bin" ,0x20000,0x1d1af7a5,    1}, // 21
  {"pd11777.bin" ,0x20000,0x9662dd32,    1}, // 22
  {"pd11778.bin" ,0x20000,0x2dfb7494,    1}, // 23
  {"pd11763.bin" ,0x20000,0x1ee23407,    1}, // 24
  {"pd11764.bin" ,0x20000,0xe859305e,    1}, // 25
  {"pd11779.bin" ,0x20000,0xa49cd793,    1}, // 26
  {"pd11780.bin" ,0x20000,0xd514ed81,    1}, // 27

  {"pd11765.bin" ,0x20000,0x649e2dff,    1}, // 28
  {"pd11766.bin" ,0x20000,0xd92fb7fc,    1}, // 29
  {"pd11781.bin" ,0x20000,0x9692d4cd,    1}, // 30
  {"pd11782.bin" ,0x20000,0xc913bb43,    1}, // 31
  {"pd11767.bin" ,0x20000,0x1f8ad054,    1}, // 32
  {"pd11768.bin" ,0x20000,0xdb2c4053,    1}, // 33
  {"pd11783.bin" ,0x20000,0x6d189007,    1}, // 34
  {"pd11784.bin" ,0x20000,0x57f5fd64,    1}, // 35

  {"pd11769.bin" ,0x20000,0x28f0ab51,    1}, // 36
  {"pd11770.bin" ,0x20000,0xd7557ea9,    1}, // 37
  {"pd11785.bin" ,0x20000,0xe6ef32c4,    1}, // 38
  {"pd11786.bin" ,0x20000,0x2066b49d,    1}, // 39
  {"pd11771.bin" ,0x20000,0x67635618,    1}, // 40
  {"pd11772.bin" ,0x20000,0x0f798d3a,    1}, // 41
  {"pd11787.bin" ,0x20000,0xe631dc12,    1}, // 42
  {"pd11788.bin" ,0x20000,0x8464c66e,    1}, // 43

  {"pd11899.bin" ,0x10000,0xed9fa889,    2}, // 44 z80 program
  {"pd11754.bin" ,0x80000,0xcad59f39,    2}, // 45 pcm data  (aka 11754.mpr)
  {"pd11756.bin" ,0x20000,0x12e43f8a,    2}, // 46
  {"pd11755.bin" ,0x20000,0xc2db1244,    2}  // 47
};

// Rom information for the different japanese roms
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDescJpn[]=
{
  {"11750b.epr"  ,0x20000,0xbc14ce30, 0x10}, //    cpu M code (even)
  {"11749b.epr"  ,0x20000,0x9e385568, 0x10}, //               (odd)
  {"11752.epr"   ,0x20000,0xb6bb8111, 0x10}, //    cpu X code (even)
  {"11751.epr"   ,0x20000,0x7f0d0311, 0x10}, //               (odd)
  {"11746a.epr"  ,0x20000,0xb0f1caf4, 0x10}, //    cpu Y code (even)
  {"pd11748.bin" ,0x20000,0x82a76cab, 0x10}, //                extra code?
  {"11745a.epr"  ,0x20000,0xa89720cd, 0x10}, //               (odd)
  {"pd11747.bin" ,0x20000,0x9796ece5, 0x10}  //                extra code?
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
  else             *pszName=DrvRomDesc[i].szName;
  return 0;
}


static struct BurnInputInfo DrvInp[]=
{
  {"Coin 1"     , 0, GalButton+6, "p1 coin"},
  {"Start"      , 0, GalButton+3, "p1 start"},
  {"Left/Right" , 1, GalAnalog+5, "p1 x-axis"},
  {"Accel"      , 1, GalAnalog+4, "p1 fire 1"},
  {"Change Gear", 0, &GalGear,    "p1 fire 2"},
  {"Brake"      , 1, GalAnalog+3, "p1 fire 3"},

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

static int DrvScode(unsigned int n) { if (n>=0x90 && n<=0x97) return 2;   return 1; }

static int DrvInit()
{
  GalGame=0x20; // PD
  GalDip[0]=0; GalDip[1]=0x15; GalButton[5]=1; // start in low gear
  BurnScode=DrvScode;
  return GalInit();
}

static int DrvjInit()
{
  GalGame=0x21; // PD (jpn)
  GalDip[0]=0; GalDip[1]=0x15; GalButton[5]=1; // start in low gear
  BurnScode=DrvScode;
  return GalInit();
}

static int DrvExit()
{
  GalExit();
  BurnScode=NULL;
  GalDip[0]=0; GalDip[1]=0; GalButton[5]=0;
  GalGame=0;
  return 0;
}

struct BurnDriver BurnDrvPwrDrift=
{
  {"pwrdrift","Power Drift","Glitches, no tilt"},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,GalFrame,GalScan,NULL,320,224
};

struct BurnDriver BurnDrvPwrDriftJpn=
{
  {"pwrdrifj","Power Drift (Jpn)","Glitches, no tilt"},
  DrvZipNameJpn,DrvRomInfoJpn,DrvRomNameJpn,DrvInputInfo,
  DrvjInit,DrvExit,GalFrame,GalScan,NULL,320,224
};
