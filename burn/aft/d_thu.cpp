#include "aft.h"
// Thunder Blade (Jpn) - Driver

static char szDrvZip[][32]={"thbladej.zip"};

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i>=sizeof(szDrvZip)/sizeof(szDrvZip[0])) return 1; // Check i is in range
  if (pszName!=NULL) { *pszName=szDrvZip[i]; return 0; }
  return 1;
}

static struct { int nLabel,nPos,nLen,bMpr; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {11304, 58, 0x20000, 0, 0xa90630ef, 0x10}, //  0 cpu 1 code (even)
  {11306, 57, 0x20000, 0, 0x4b95f2b4, 0x10}, //  1
  {11305, 63, 0x20000, 0, 0x9ba3ef61, 0x10}, //  2            (odd)
  {11307, 62, 0x20000, 0, 0x2d6833e4, 0x10}, //  3
  {11308, 20, 0x20000, 0, 0x7956c238, 0x10}, //  4 cpu 2 code (even)
  {11310, 21, 0x20000, 0, 0x5d9fa02c, 0x10}, //  6
  {11309, 29, 0x20000, 0, 0xc887f620, 0x10}, //  5            (odd)
  {11311, 30, 0x20000, 0, 0x483de21b, 0x10}, //  7
  {11314,154, 0x10000, 1, 0xd4f954a9,    1}, //  8 tile bitplanes
  {11315,153, 0x10000, 1, 0x35813088,    1}, //  9 
  {11316,152, 0x10000, 1, 0x84290dff,    1}, // 10 
  {11323, 90, 0x20000, 1, 0x27e40735,    1}, // 11 sprites
  {11322, 94, 0x20000, 1, 0x10364d74,    1}, // 12
  {11321, 98, 0x20000, 1, 0x8e738f58,    1}, // 13
  {11320,102, 0x20000, 1, 0xa95c76b8,    1}, // 14
  {11327, 91, 0x20000, 1, 0xdeae90f1,    1}, // 15
  {11326, 95, 0x20000, 1, 0x29198403,    1}, // 16
  {11325, 99, 0x20000, 1, 0xb9e98ae9,    1}, // 17
  {11324,103, 0x20000, 1, 0x9742b552,    1}, // 18
  {11331, 92, 0x20000, 1, 0x3a2c042e,    1}, // 19
  {11330, 96, 0x20000, 1, 0xaa7c70c5,    1}, // 20
  {11329,100, 0x20000, 1, 0x31b20257,    1}, // 21
  {11328,104, 0x20000, 1, 0xda39e89c,    1}, // 22
  {11335, 93, 0x20000, 1, 0xf19b3e86,    1}, // 23
  {11334, 97, 0x20000, 1, 0x348f91c7,    1}, // 24
  {11333,101, 0x20000, 1, 0x05a2333f,    1}, // 25
  {11332,105, 0x20000, 1, 0xdc089ec6,    1}, // 26
  {11313, 40, 0x10000, 0, 0x6a56c4c3,    1}, // 27 ground data
  {11312, 17, 0x10000, 0, 0         ,    2}, // 28 z80 program
  {11317, 11, 0x20000, 1, 0         ,    2}, // 29 pcm data
  {11318, 12, 0x20000, 1, 0         ,    2}, // 30
  {11319, 13, 0x20000, 1, 0         ,    2}  // 31
};

// Return 0 if a rom number is defined, and (if pri!=NULL) information about each rom
static int DrvRomInfo(struct BurnRomInfo *pri,unsigned int i)
{
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range
  // Return information about the rom
  if (pri!=NULL)
  {
    pri->nLen=DrvRomDesc[i].nLen;
    pri->nCrc=DrvRomDesc[i].nCrc;
    pri->nType=DrvRomDesc[i].nType;
  }
  return 0;
}

// Return possible names for rom number 'i'
static int DrvRomName(char **pszName,unsigned int i,int nAka)
{
  static char szName[32]=""; int nLabel=0,nPos=0;
  if (pszName==NULL) return 1;
  if (i>=sizeof(DrvRomDesc)/sizeof(DrvRomDesc[0])) return 1; // Check i is in range

  nLabel=DrvRomDesc[i].nLabel;
  nPos=  DrvRomDesc[i].nPos;
  szName[0]=0; // no name yet

  // try to make an name
  if (nAka==0) sprintf(szName,"%d.epr",nLabel);

  if (szName[0]==0) return 1; // We couldn't make a name (unknown nAka)

  // Return information about the rom
  *pszName=szName;
  return 0;
}

static struct BurnInputInfo DrvInp[]=
{
  {"Coin 1"     , 0, AftButton+6, "p1 coin"},
  {"Start"      , 0, AftButton+3, "p1 start"},
  {"Left/Right" , 1, AftInput+3 , "p1 x-axis"},
  {"Up/Down"    , 1, AftInput+4 , "p1 y-axis"},
  {"Fire 1"     , 0, AftButton+4, "p1 fire 1"},
  {"Fire 2"     , 0, AftButton+5, "p1 fire 2"},
  {"Throttle"   , 1, AftInput+5 , "p1 z-axis"},
  {"Fast"       , 0, AftButton+8, "p1 fire 3"},
  {"Slow"       , 0, AftButton+9, "p1 fire 4"},

  {"Coin 2"     , 0, AftButton+7, "p2 coin"},

  {"Reset"      , 0, &AftReset,   "reset"},
  {"Service"    , 0, AftButton+2, "service"},
  {"Diagnostic" , 0, AftButton+1, "diag"},
  {"Dip A"      , 2, AftInput+1 , "dip"},
  {"Dip B"      , 2, AftInput+2 , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n) { if (n>=0x98 && n<=0x9f) return 2;   return 1; }

static int DrvInit()
{
  AftGame=3; // TB
  AftInput[1]=0x00; // Dip A
  AftInput[2]=0x03; // Dip B
  BurnScode=DrvScode;
  return AftInit();
}

static int DrvExit()
{
  AftExit();
  BurnScode=NULL;
  AftGame=0;
  return 0;
}

struct BurnDriver BurnDrvThbladej=
{
  {"thbladej","Thunder Blade (Jpn)","Background missing"},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,AftFrame,AftScan,
  NULL,320,224
};
