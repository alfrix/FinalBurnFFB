#include "aft.h"
// After Burner I - Driver

static char szDrvZip[][32]={"aburner2.zip","aburner.zip"};

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i>=sizeof(szDrvZip)/sizeof(szDrvZip[0])) return 1; // Check i is in range
  if (pszName!=NULL) { *pszName=szDrvZip[i]; return 0; }
  return 1;
}

static struct { int nLabel,nPos,nLen,bMpr; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {10940,  0, 0x20000, 0, 0x4d132c4e, 0x10}, //  0 cpu 1 code even
  {0    ,  0,       0, 0, 0         ,    0},
  {10941,  0, 0x20000, 0, 0x136ea264, 0x10}, //  2            odd
  {0    ,  0,       0, 0, 0         ,    0},
  {10927,  0, 0x20000, 0, 0x66d36757, 0x10}, //  4 cpu 2 code even
  {0    ,  0,       0, 0, 0         ,    0},
  {10928,  0, 0x20000, 0, 0x7c01d40b, 0x10}, //  6            odd
  {0    ,  0,       0, 0, 0         ,    0},
  {10926,  0, 0x10000, 0, 0xed8bd632,    1}, //  8 tile bitplanes
  {10925,  0, 0x10000, 0, 0x4ef048cc,    1}, //  9 
  {10924,  0, 0x10000, 0, 0x50c15a6d,    1}, // 10 
  {10932,125, 0x20000, 1, 0xcc0821d6,    1}, // 11 sprite bitmaps
  {10934,129, 0x20000, 1, 0x4a51b1fa,    1}, // 12
  {10936,133, 0x20000, 1, 0xada70d64,    1}, // 13
  {10938,102, 0x20000, 1, 0xe7675baf,    1}, // 14
  {10933,126, 0x20000, 1, 0xc8efb2c3,    1}, // 15
  {10935,130, 0x20000, 1, 0xc1e23521,    1}, // 16
  {10937,134, 0x20000, 1, 0xf0199658,    1}, // 17
  {10939,103, 0x20000, 1, 0xa0d49480,    1}, // 18
  {10942,  0, 0x20000, 0, 0x5ce10b8c,    1}, // 19
  {10943,  0, 0x20000, 0, 0xb98294dc,    1}, // 20
  {10944,  0, 0x20000, 0, 0x17be8f67,    1}, // 21
  {10945,  0, 0x20000, 0, 0xdf4d4c4f,    1}, // 22
  {10946,  0, 0x20000, 0, 0xd7d485f4,    1}, // 23
  {10947,  0, 0x20000, 0, 0x08838392,    1}, // 24
  {10948,  0, 0x20000, 0, 0x64284761,    1}, // 25
  {10949,  0, 0x20000, 0, 0xd8437d92,    1}, // 26
  {10922, 40, 0x10000, 0, 0xb49183d4,    1}, // 27 ground data
  {10923,  0, 0x10000, 0, 0x6888eb8f,    2}, // 28 z80 program
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

  // try to make a name
  if (nAka==0) sprintf(szName,"%s%d.%d",DrvRomDesc[i].bMpr?"mpr":"epr",nLabel,nPos);
  if (nAka==1) sprintf(szName,"%s%d.bin",DrvRomDesc[i].bMpr?"mpr":"epr",nLabel,nPos);
  if (nAka==2) sprintf(szName,"%d.%d",nLabel,nPos);
  if (nAka==3) sprintf(szName,"%d.ROM",nLabel);
  if (nAka==4)
  {
    // Some roms are also known by different names
    if (nLabel>=11103 && nLabel<=11106) { nLabel-=11103; nLabel+=11094; }
    if (nLabel==11102) nLabel=10929;
    sprintf(szName,"%d.ROM",nLabel);
  }

  if (szName[0]==0) return 1; // We couldn't make a name (unknown nAka)

  // Return information about the rom
  *pszName=szName;
  return 0;
}

static int DrvScode(unsigned int n) { if (n>=0x91 && n<=0x97) return 2;  return 1; }

static int DrvInit()
{
  AftGame=1; // AB1
  // Set dip switches
  AftInput[1]=0x00; // Dip A
  AftInput[2]=0x3e; // Dip B
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

struct BurnDriver BurnDrvAburner1=
{
  {"aburner1","After Burner I","Some gfx missing"},
  DrvZipName,DrvRomInfo,DrvRomName,Aft2InputInfo,
  DrvInit,DrvExit,AftFrame,AftScan,NULL,320,224
};
