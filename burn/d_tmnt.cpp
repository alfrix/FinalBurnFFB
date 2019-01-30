#include "burnint.h"
// TMNT - Driver

// Count possible zip names and (if pszName!=NULL) return them
static int DrvZipName(char **pszName,unsigned int i)
{
  if (i==0) { if (pszName!=NULL) *pszName="tmnt.zip";  return 0; }
  return 1;
}

// Rom information
static struct { char szName[16]; int nLen; unsigned int nCrc; int nType; } DrvRomDesc[]=
{
  {"963-r23",0x20000,0           , 0x10}, //  0 68000 code (even)
  {"963-r24",0x20000,0           , 0x10}, //  1            (odd)
  {"963-r21",0x10000,0           , 0x10}, //  2            (even)
  {"963-r22",0x10000,0           , 0x10}, //  3            (odd)

  {"963-a28",0x80000,0           ,    1}, //  4 tile bitmaps
  {"963-a29",0x80000,0           ,    1}, //  5
  {"963-a15",0x80000,0           ,    1}, //  6
  {"963-a17",0x80000,0           ,    1}, //  7
  {"963-a16",0x80000,0           ,    1}, //  8
  {"963-a18",0x80000,0           ,    1}, //  9
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
  {"p1 coin"  , 0, TmntCoin+0, "p1 coin"},
  {"p2 coin"  , 0, TmntCoin+1, "p2 coin"},
  {"p3 coin"  , 0, TmntCoin+2, "p3 coin"},
  {"p4 coin"  , 0, TmntCoin+3, "p4 coin"},
  {"p1 start" , 0, TmntStart+0,"p1 start"},
  {"p2 start" , 0, TmntStart+1,"p2 start"},
  {"p3 start" , 0, TmntStart+2,"p3 start"},
  {"p4 start" , 0, TmntStart+3,"p4 start"},
};

// Return 0 if a input number is defined, and (if pii!=NULL) information about each input
static int DrvInputInfo(struct BurnInputInfo *pii,unsigned int i)
{
  if (i>=sizeof(DrvInp)/sizeof(DrvInp[0])) return 1;
  if (pii!=NULL) *pii=DrvInp[i];
  return 0;
}

struct BurnDriver BurnDrvTmnt=
{
  {"tmnt","Teenage Mutant Ninja Turtles","w.i.p."},
  DrvZipName,DrvRomInfo,DrvRomName,DrvInputInfo,
  TmntInit,TmntExit,TmntFrame,TmntScan,
  &bTmntRecalcPal,288,224,
};
