#include "gal.h"
// Strike Fighter (Jpn) - Driver

// Count possible zip names and (if pName!=NULL) return them
static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="strkfgtr.zip"; return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"epr13828.54" ,0x20000,0x2470cf5f, 0x10}, //  0  cpu M code (even)
  {"epr13827.53" ,0x20000,0xa9d0cf7d, 0x10}, //  1             (odd)
  {"epr13830.81" ,0x20000,0xf9adc9d1, 0x10}, //  2  cpu X code (even)
  {"epr13829.80" ,0x20000,0xc5cd85dd, 0x10}, //  3             (odd)
  {"epr13824.25" ,0x20000,0x2cf2610c, 0x10}, //  4  cpu Y code (even)
  {"epr13826.27" ,0x20000,0x3d34ea55, 0x10}, //  5
  {"epr13823.24" ,0x20000,0x2c98242f, 0x10}, //  6             (odd)
  {"epr13825.26" ,0x20000,0xfe218d83, 0x10}, //  7

  {"epr13833.16" ,0x80000,0x6148e11a,    1},  //  8  line bitmaps
  {"epr13832.14" ,0x80000,0x41679754,    1},  //  9
  {"epr13040.17" ,0x80000,0x4aeb3a85,    1},
  {"epr13038.15" ,0x80000,0x0b2edb6d,    1},

  {"mpr13048.67" ,0x80000,0xfe1eb0dd,    1}, // 12  sprite bitmaps
  {"mpr13056.75" ,0x80000,0x5904f8e6,    1}, // 13
  {"mpr13044.63" ,0x80000,0x4d931f89,    1}, // 14
  {"mpr13052.71" ,0x80000,0x0291f040,    1}, // 15
  {"mpr13064.86" ,0x80000,0x5f8e651b,    1}, // 16
  {"mpr13072.114",0x80000,0x6b85641a,    1}, // 17
  {"mpr13060.82" ,0x80000,0xee16ad97,    1}, // 18
  {"mpr13068.110",0x80000,0x64d52bbb,    1}, // 19

  {"mpr13047.66" ,0x80000,0x53340832,    1}, // 20
  {"mpr13055.74" ,0x80000,0x39b6b665,    1}, // 21
  {"mpr13043.62" ,0x80000,0x208f16fd,    1}, // 22
  {"mpr13051.70" ,0x80000,0xad62cbd4,    1}, // 23
  {"mpr13063.85" ,0x80000,0xc580bf6d,    1}, // 24
  {"mpr13071.113",0x80000,0xdf99ef99,    1}, // 25
  {"mpr13059.81" ,0x80000,0x4c982558,    1}, // 26
  {"mpr13067.109",0x80000,0xf97f6119,    1}, // 27

  {"mpr13046.65" ,0x80000,0xc75a86e9,    1}, // 28
  {"mpr13054.73" ,0x80000,0x2934549a,    1}, // 29
  {"mpr13042.61" ,0x80000,0x53ed97af,    1}, // 30
  {"mpr13050.69" ,0x80000,0x04429068,    1}, // 31
  {"mpr13062.84" ,0x80000,0x4fdb4ee3,    1}, // 32
  {"mpr13070.112",0x80000,0x52ea130e,    1}, // 33
  {"mpr13058.80" ,0x80000,0x19ff1626,    1}, // 34
  {"mpr13066.108",0x80000,0xbc70a250,    1}, // 35

  {"mpr13045.64" ,0x80000,0x54d5bc6d,    1}, // 28
  {"mpr13053.72" ,0x80000,0x9502af13,    1}, // 29
  {"mpr13041.60" ,0x80000,0xd0a7402c,    1}, // 30
  {"mpr13049.68" ,0x80000,0x5b9c0b6c,    1}, // 31
  {"mpr13061.83" ,0x80000,0x7b95ec3b,    1}, // 32
  {"mpr13069.111",0x80000,0xe1f538f0,    1}, // 33
  {"mpr13057.79" ,0x80000,0x73baefee,    1}, // 34
  {"mpr13065.107",0x80000,0x8937a655,    1}, // 35

  {"epr13831.102",0x10000,0xdabbcea1,    2}, // 44
  {"mpr13034.105",0x80000,0xcd22d95d,    2}, // 45
  {"mpr13035.106",0x80000,0x009fa13e,    2}, // 46
  {"mpr13036.107",0x80000,0x7890c26c,    2}, // 47
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
static int DrvRomName(char **pName,unsigned int i,int nAka)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  if (nAka!=0) return 1; // Roms known by one name
  *pName=DrvRomDesc[i].szName;
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
  GalGame=0x50;
  GalDip[0]=0; GalDip[1]=0xa4;
  return GalInit();
}

struct BurnDriver BurnDrvStrkfgtr=
{
  {"strkfgtr","Strike Fighter (Jpn)","w.i.p."},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,GalExit,GalFrame,GalScan,
  NULL,320,224
};
