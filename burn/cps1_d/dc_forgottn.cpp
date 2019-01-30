#include "../burnint.h"
// Forgotten Worlds - Driver

static unsigned char Ana[2]={0x80,0x80};
struct Dig { int Val; unsigned char Left,Right; };
struct Dig Dig[2];
static unsigned char Diag=0;

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Dial Left"  , 0, &Dig[0].Left,"p1 fire 1"},
  {"P1 Shot"       , 0, CpsInp001+4, "p1 fire 2"},
  {"P1 Dial Right" , 0, &Dig[0].Right,"p1 fire 3"},
  {"P1 Dial Analog", 1, &Ana[0],     "p1 z-axis"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Dial Left"  , 0, &Dig[1].Left,"p2 fire 1"},
  {"P2 Shot"       , 0, CpsInp000+4, "p2 fire 2"},
  {"P2 Dial Right" , 0, &Dig[1].Right,"p2 fire 3"},
  {"P2 Dial Analog", 1, &Ana[1],     "p2 z-axis"},
 
  {"Reset"         , 0, &CpsReset,   "reset"},
  {"Diagnostic"    , 0, &Diag,       "diag"},
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
  if (n<0x20) return 2; // Music
  if (n<0x60) return 1; // Sound
  if (n<0x80) return 2; // Music
  if (n<0x81) return 0; // 0x80=Stop
  if (n<0xf0) return 1; // Speech
  return 0; // (0xf0=stop)
}

static int DrvFind0(int t) { t|=0x4000; t<<=7; return t; }

static int DrvInit()
{
  int nRet=0;
  Cps=1;
  nCpsRomLen=   0x100000;
  nCpsCodeLen=0; // not encrypted
  nCpsGfxLen=   0x400000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen=    0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  nRet=BurnLoadRom(CpsRom+0x000001,0,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x000000,1,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040001,2,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x040000,3,2); if (nRet!=0) return 1;
  nRet=BurnLoadRom(CpsRom+0x080000,4,1); if (nRet!=0) return 1; // Already byteswapped

  // Load graphics roms
  CpsLoadTiles(CpsGfx         ,5);
  CpsLoadTiles(CpsGfx+0x200000,9);

  // Load Z80 Rom
  BurnLoadRom(CpsZRom,13,1);
  nPsndIrqPeriod=(60<<10)/500; //OLDTST
  BurnScode=DrvScode;

  // Load ADPCM data
  BurnLoadRom(CpsAd         ,14,1);
  BurnLoadRom(CpsAd+0x020000,15,1);

  // Extras:
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  nPsaSpeed=6061;
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
  int DigSpeed=0x18;
  int i=0;

  if (Diag) Cpi01C|=0x40; // Enabled diagnostic mode on Dip switch

  for (i=0;i<2;i++)
  {
    int Val=0;

    Val=Ana[i]<<3; Val-=0x400;

    // Digital controls
    if (Dig[i].Left ) Dig[i].Val-=DigSpeed;
    if (Dig[i].Right) Dig[i].Val+=DigSpeed;
    Val+=Dig[i].Val;

    // Clip to within -180 to +180
    Val+=0x400; Val&=0x7ff; Val-=0x400; CpsDial[i]=Val;
  }

  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="forgottn.zip"; return 0; }
  return 1;
}

// Rom information
static struct StdRomInfo DrvRomDesc[]=
{
  {"lwu11a"     ,0x20000,0xddf78831,0x10}, // 0 even 68000 code
  {"lwu15a"     ,0x20000,0xf7ce2097,0x10}, // 1 odd
  {"lwu10a"     ,0x20000,0x8cb38c81,0x10}, //
  {"lwu14a"     ,0x20000,0xd70ef9fd,0x10}, //
  {"lw-07"      ,0x80000,0xfd252a26,0x10}, // 4 both

  // graphics:
  {"lw-02"      ,0x80000,0x43e6c5c8,   1}, // 5
  {"lw-09"      ,0x80000,0x899cb4ad,   1},
  {"lw-06"      ,0x80000,0x5b9edffc,   1},
  {"lw-13"      ,0x80000,0x8e058ef5,   1},
  {"lw-01"      ,0x80000,0x0318f298,   1},
  {"lw-08"      ,0x80000,0x25a8e43c,   1},
  {"lw-05"      ,0x80000,0xe4552fd7,   1},
  {"lw-12"      ,0x80000,0x8e6a832b,   1},

  // z80 rom
  {"lwu00"      ,0x10000,0x59df2a63,   2}, //13
  // samples
  {"lw-03u"     ,0x20000,0x807d051f,   2},
  {"lw-04u"     ,0x20000,0xe6cd098e,   2}
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsForgottn=
{
  {"forgottn","Forgotten Worlds",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
