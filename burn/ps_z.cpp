#include "burnint.h"
// PSound - Z80

static unsigned char PsndZIn(unsigned short a) { (void)a; return 0; }
static void PsndZOut(unsigned short a,unsigned char d) { (void)a; (void)d; }

static int nTimerPeriod=0; // Period of the timer in cycles
static int nNextTimer=0; // The z80 cycle count when FM timer A will carry out
static unsigned char FmReg[0x100]; // Our copy of the FM registers
static unsigned char nFmSel=0; // The FM register selected
static int nPsndZBank=0;
static unsigned char *PsndZRam=NULL;

static INLINE int CalcTimerPeriod()
{
  nTimerPeriod=(FmReg[0x10]<<2)|(FmReg[0x11]&3);
  nTimerPeriod=64*(1024-nTimerPeriod);
  return 0;
}

// Map in the memory for the current 0x8000-0xc000 bank
static int PsndZBankMap()
{
  unsigned int nOff; unsigned char *Bank;
  nOff=nPsndZBank<<14;
  nOff+=0x8000;

  if (nOff+0x4000>nCpsZRomLen) nOff=0; // End of bank in out of range
  Bank=CpsZRom+nOff;

  // Read and fetch the bank
  ZetMapArea(0x8000,0xbfff,0,Bank);
  ZetMapArea(0x8000,0xbfff,2,Bank);
  return 0;
}

// PSound Z80 memory write
static void PsndZWrite(unsigned short a,unsigned char d)
{
  if (a==0xf000) { nFmSel=d; return; } // FM Register select
  if (a==0xf001)
  {
    // FM Register write
    FmReg[nFmSel]=d;
    // Write the register to the chip emu
    if (bPsmOkay) YM2151WriteReg(0,nFmSel,d);

    if (nFmSel==0x10 || nFmSel==0x11) { CalcTimerPeriod(); } // recalc timer period
    return;
  }
  if (a==0xf002)
  {
    PsaWrite(d);
    return;
  }
  if (a==0xf004)
  {
    int nNewBank; nNewBank=d&0x0f;
    if (nPsndZBank!=nNewBank)
    {
      nPsndZBank=nNewBank;
      PsndZBankMap();
    }
  }
}

// PSound Z80 memory read
static unsigned char PsndZRead(unsigned short a)
{
  unsigned char d=0;
       if (a==0xf008) d=PsndCode;
  else if (a==0xf00a) d=PsndFade;
  else if (a==0xf002) d=nPsaStatus; // adpcm status
  else if (a==0xf001) { static unsigned char Toggle=0; Toggle^=1; d=Toggle; }
  return d;
}

int PsndZInit()
{
  if (nCpsZRomLen<0x8000) return 1; // Not enough Z80 Data
  if (CpsZRom==NULL) return 1;
  PsndZRam=(unsigned char *)malloc(0x800); if (PsndZRam==NULL) return 1;

  ZetRead=PsndZRead; ZetWrite=PsndZWrite;
  ZetIn=PsndZIn; ZetOut=PsndZOut;
  ZetInit();

  // Read and fetch first 0x8000 of Rom
  ZetMapArea(0x0000,0x7fff,0,CpsZRom);
  ZetMapArea(0x0000,0x7fff,2,CpsZRom);

  // Map first Bank of Rom to 0x8000-0xc000
  nPsndZBank=0;
  PsndZBankMap();

  // Ram
  ZetMapArea(0xd000,0xd7ff,0,PsndZRam);
  ZetMapArea(0xd000,0xd7ff,1,PsndZRam);
  ZetMapArea(0xd000,0xd7ff,2,PsndZRam);

  // Sound chip interfaces
  ZetMemCallback(0xf000,0xffff,0);
  ZetMemCallback(0xf000,0xffff,1);

  // In case it tries to fetch other areas
  ZetMapArea(0xc000,0xcfff,2,CpsZRom);
  ZetMapArea(0xd800,0xffff,2,CpsZRom);
  ZetMemEnd();

  return 0;
}

int PsndZExit()
{
  if (PsndZRam!=NULL) free(PsndZRam);  PsndZRam=NULL;

  ZetExit();
  ZetIn=NULL; ZetOut=NULL;
  ZetRead=NULL; ZetWrite=NULL;
  return 0;
}

int PsndZRun(int nWant)
{
  int nDid;
  if (nWant<=0) return 0;
  nDid=ZetRun(nWant);
  nNextTimer-=nDid;
  return nDid;
}

// Scan the current PSound z80 state
int PsndZScan(int nAction)
{
  struct BurnArea ba; int i=0;
  ZetScan(nAction); // Scan Z80

  SCAN_VAR(nTimerPeriod)
  SCAN_VAR(nNextTimer)
  SCAN_VAR(FmReg)
  SCAN_VAR(nFmSel)
  SCAN_VAR(nPsndZBank)

  // Scan Ram
  memset(&ba,0,sizeof(ba));
  ba.szName="PsndZRam";
  ba.Data=PsndZRam;
  ba.nLen=0x800;
  BurnAcb(&ba);

  if (nAction&2)
  {
    PsndZBankMap(); // If write, bank could have changed
    CalcTimerPeriod(); // If write, timer registers could have changed
    // Resend all the registers to the ym2151
    if (bPsmOkay) { for (i=0;i<0x100;i++) { YM2151WriteReg(0,i,FmReg[i]); } }
  }
  return 0;
}
