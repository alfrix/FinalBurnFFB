#include "../burnint.h"
// Street Fighter 2 - Driver

static struct BurnInputInfo DrvInp[]=
{
  {"P1 Coin"       , 0, CpsInp018+0, "p1 coin"},
  {"P1 Start"      , 0, CpsInp018+4, "p1 start"},
  {"P1 Up"         , 0, CpsInp001+3, "p1 up"},
  {"P1 Down"       , 0, CpsInp001+2, "p1 down"},
  {"P1 Left"       , 0, CpsInp001+1, "p1 left"},
  {"P1 Right"      , 0, CpsInp001+0, "p1 right"},
  {"P1 Low Punch"  , 0, CpsInp001+4, "p1 fire 4"},
  {"P1 Mid Punch"  , 0, CpsInp001+5, "p1 fire 5"},
  {"P1 High Punch" , 0, CpsInp001+6, "p1 fire 6"},
  {"P1 Low Kick"   , 0, CpsInp177+0, "p1 fire 1"},
  {"P1 Mid Kick"   , 0, CpsInp177+1, "p1 fire 2"},
  {"P1 High Kick"  , 0, CpsInp177+2, "p1 fire 3"},

  {"P2 Coin"       , 0, CpsInp018+1, "p2 coin"},
  {"P2 Start"      , 0, CpsInp018+5, "p2 start"},
  {"P2 Up"         , 0, CpsInp000+3, "p2 up"},
  {"P2 Down"       , 0, CpsInp000+2, "p2 down"},
  {"P2 Left"       , 0, CpsInp000+1, "p2 left"},
  {"P2 Right"      , 0, CpsInp000+0, "p2 right"},
  {"P2 Low Punch"  , 0, CpsInp000+4, "p2 fire 4"},
  {"P2 Mid Punch"  , 0, CpsInp000+5, "p2 fire 5"},
  {"P2 High Punch" , 0, CpsInp000+6, "p2 fire 6"},
  {"P2 Low Kick"   , 0, CpsInp177+4, "p2 fire 1"},
  {"P2 Mid Kick"   , 0, CpsInp177+5, "p2 fire 2"},
  {"P2 High Kick"  , 0, CpsInp177+6, "p2 fire 3"},

  {"Reset"         , 0, &CpsReset  , "reset"},
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
  if (n>=0x01 && n<0x1f) return 2;
  if (n>=0x34 && n<0x35) return 2;
  if (n>=0x79 && n<0x84) return 2;
  if (n>=0x8c && n<0x8d) return 2;

  if (n>=0x8f && n<0xad) return 2;
  if (n>=0xc2 && n<0xc3) return 2;
  return 1;
}

static int DrvFind1(int t) { return (t+0x10000)<<6; }
static int DrvFind2(int t) { return (t+0x08000)<<7; }
static int DrvFind3(int t) { return (t+0x02000)<<9; }

static int DrvInit()
{
  int nRet=0; int i=0;
  Cps=1;
  nCpsRomLen =  0x100000;
  nCpsGfxLen =  0x600000;
  nCpsZRomLen=  0x010000;
  nCpsAdLen  =  0x040000;
  nRet=CpsInit(); if (nRet!=0) return 1;

  // Load program roms
  for (i=0;i<8;i++)
  {
    int a; a=(i>>1)*0x40000; a+=(i&1); a^=1;
    nRet=BurnLoadRom(CpsRom+a,i,2); if (nRet!=0) return 1;
  }

  // Load up and interleve each set of 4 roms to make the 16x16 tiles
  for (i=0;i<3;i++) CpsLoadTiles(CpsGfx+i*0x200000,8+i*4);

  // Load Z80 Rom
  nRet=BurnLoadRom(CpsZRom,20,1);
  BurnScode=DrvScode;

  // Load ADPCM data
  nRet=BurnLoadRom(CpsAd        ,21,1);
  nRet=BurnLoadRom(CpsAd+0x20000,22,1);

  nPsndIrqPeriod=(60<<10)/250; //OLDTST

  // Extras:
  CpsId[0]=0x48; CpsId[1]=0x0407; // Board ID
  Cpi01C=0x03; Cpi01E=0x60; // Dip switches
  CpsLcReg=0x54; // Layer control register is at 0x54
  CpsPmReg[0]=0x52; CpsPmReg[1]=0x50; CpsPmReg[2]=0x4e; CpsPmReg[3]=0x4c;
  CpsLayEn[1]=0x08; CpsLayEn[2]=0x10; CpsLayEn[3]=0x02; // Layer enable is different
  // Custom tile finder:
  CpsFind1=DrvFind1; CpsFind2=DrvFind2; CpsFind3=DrvFind3;

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
  // Carry on to CPS frame
  CpsFrame();
  return 0;
}

static int DrvZipName(char **pName,unsigned int i)
{
  if (i==0) { if (pName!=NULL) *pName="sf2.zip"; return 0; }
  return 1;
}

static struct StdRomInfo DrvRomDesc[]=
{
// 0 - 68000 code
  {"sf2_30a.bin" ,0x20000,0x57bd7051, 0x10},
  {"sf2e_37b.rom",0x20000,0x62691cdd, 0x10},
  {"sf2_31a.bin" ,0x20000,0xa673143d, 0x10},
  {"sf2_38a.bin" ,0x20000,0x4c2ccef7, 0x10},
  {"sf2_28a.bin" ,0x20000,0x4009955e, 0x10},
  {"sf2_35a.bin" ,0x20000,0x8c1f3994, 0x10},
  {"sf2_29a.bin" ,0x20000,0xbb4af315, 0x10},
  {"sf2_36a.bin" ,0x20000,0xc02a13eb, 0x10},

// 8
  {"sf2_06.bin",0x80000,0x22c9cc8e,    1},
  {"sf2_08.bin",0x80000,0x57213be8,    1},
  {"sf2_05.bin",0x80000,0xba529b4f,    1},
  {"sf2_07.bin",0x80000,0x4b1b33a8,    1},
  {"sf2_15.bin",0x80000,0x2c7e2229,    1},
  {"sf2_17.bin",0x80000,0xb5548f17,    1},
  {"sf2_14.bin",0x80000,0x14b84312,    1},
  {"sf2_16.bin",0x80000,0x5e9cd89a,    1},
  {"sf2_25.bin",0x80000,0x994bfa58,    1},
  {"sf2_27.bin",0x80000,0x3e66ad9d,    1},
  {"sf2_24.bin",0x80000,0xc1befaa8,    1},
  {"sf2_26.bin",0x80000,0x0627c831,    1},

// 20
// z80 program
  {"sf2_09.bin",0x10000,0xa4823a1b,    2},
// adpcm samples
  {"sf2_18.bin",0x20000,0x7f162009,    2},
  {"sf2_19.bin",0x20000,0xbeade53f,    2},
};

// Make The RomInfo/Name functions for the game
STD_ROM_PICK(Drv) STD_ROM_FN(Drv)

struct BurnDriver BurnDrvCpsSf2=
{
  {"sf2","Street Fighter 2",""},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  DrvInit,DrvExit,DrvFrame,CpsAreaScan,
  &CpsRecalcPal,384,224
};
