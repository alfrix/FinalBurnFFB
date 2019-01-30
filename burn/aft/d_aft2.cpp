#include "aft.h"
// After Burner II - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="aburner2.zip";  return 0; }
  return 1;
}

static struct { int nLabel,nPos,nLen,bMpr; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {11107, 58, 0x20000, 0, 0x6d87bab7, 0x10}, //  0 cpu 1 code even
  {0    ,  0,       0, 0, 0         ,    0},
  {11108,104, 0x20000, 0, 0x202a3e1d, 0x10}, //  2            odd
  {0    ,  0,       0, 0, 0         ,    0},

  {11109, 20, 0x20000, 0, 0x85a0fe07, 0x10}, //  4 cpu 2 code even
  {0    ,  0,       0, 0, 0         ,    0},
  {11110, 29, 0x20000, 0, 0xf3d6797c, 0x10}, //  6            odd
  {0    ,  0,       0, 0, 0         ,    0},

  {11115,154, 0x10000, 0, 0xe8e32921,    1}, //  8 tile bitplanes
  {11114,153, 0x10000, 0, 0x2e97f633,    1}, //  9 
  {11113,152, 0x10000, 0, 0x36058c8c,    1}, // 10 
  {10932,125, 0x20000, 1, 0xcc0821d6,    1}, // 11 sprite bitmaps
  {10934,129, 0x20000, 1, 0x4a51b1fa,    1}, // 12
  {10936,133, 0x20000, 1, 0xada70d64,    1}, // 13
  {10938,102, 0x20000, 1, 0xe7675baf,    1}, // 14
  {10933,126, 0x20000, 1, 0xc8efb2c3,    1}, // 15
  {10935,130, 0x20000, 1, 0xc1e23521,    1}, // 16
  {10937,134, 0x20000, 1, 0xf0199658,    1}, // 17
  {10939,103, 0x20000, 1, 0xa0d49480,    1}, // 18
  {11103,127, 0x20000, 1, 0xbdd60da2,    1}, // 19
  {11104,131, 0x20000, 1, 0x06a35fce,    1}, // 20
  {11105,135, 0x20000, 1, 0x027b0689,    1}, // 21
  {11106,104, 0x20000, 1, 0x9e1fec09,    1}, // 22
  {11116,128, 0x20000, 0, 0x49b4c1ba,    1}, // 23
  {11117,132, 0x20000, 0, 0x821fbb71,    1}, // 24
  {11118,136, 0x20000, 0, 0x8f38540b,    1}, // 25
  {11119,105, 0x20000, 0, 0xd0343a8e,    1}, // 26
  {10922, 40, 0x10000, 0, 0xb49183d4,    1}, // 27 ground data
  {11112, 17, 0x10000, 0, 0xd777fc6d,    2}, // 28 z80 program
  {10930, 11, 0x20000, 1, 0x9209068f,    2}, // 29 pcm data
  {10931, 12, 0x20000, 1, 0x6493368b,    2}, // 30
  {11102, 13, 0x20000, 1, 0x6c07c78d,    2}  // 31
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
  if (nAka==0) sprintf(szName,"%s%d.%d",DrvRomDesc[i].bMpr?"mpr":"epr",nLabel,nPos);
  if (nAka==1) sprintf(szName,"%d.%d",nLabel,nPos);
  if (nAka==2) sprintf(szName,"%d.rom",nLabel);
  if (nAka==3)
  {
    // Some roms are also known by different names
    if (nLabel>=11103 && nLabel<=11106) { nLabel-=11103; nLabel+=11094; }
    if (nLabel==11102) nLabel=10929;
    sprintf(szName,"%d.ROM",nLabel);
  }
  if (nAka==4) sprintf(szName,"%s%d.bin",DrvRomDesc[i].bMpr?"mpr":"epr",nLabel);

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
  {"Vulcan"     , 0, AftButton+4, "p1 fire 1"},
  {"Missile"    , 0, AftButton+5, "p1 fire 2"},
  {"Throttle"   , 1, AftInput+5 , "p1 z-axis"},
  {"Throttle To Max", 0, AftButton+8, "p1 fire 3"},
  {"Throttle To Min", 0, AftButton+9, "p1 fire 4"},

  {"Coin 2"     , 0, AftButton+7, "p2 coin"},
  {"Reset"      , 0, &AftReset  , "reset"},
  {"Service"    , 0, AftButton+2, "service"},

  {"Diagnostic" , 0, AftButton+1, "diag"},

  {"Dip A"      , 2, AftInput+1 , "dip"},
  {"Dip B"      , 2, AftInput+2 , "dip"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
int Aft2InputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

static int DrvScode(unsigned int n) { if (n>=0x91 && n<=0x97) return 2;  return 1; }

static int DrvInit()
{
  AftGame=2; // AB2
  AftInput[1]=0x00; // Dip A
  // 0x40=no/no/easy +0x30=yes/yes/normal
  AftInput[2]=0x22; // Dip B
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

struct BurnDriver BurnDrvAburner2=
{
  {"aburner2","After Burner II","Some gfx missing"},
  DrvZipName,DrvRomInfo,DrvRomName,Aft2InputInfo,
  DrvInit,DrvExit,AftFrame, AftScan,
  NULL,320,224
};
