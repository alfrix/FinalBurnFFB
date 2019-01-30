#include "burnint.h"
// PSound (CPS1 sound)

static int nZTotal=3579540/60; // 3.5mhz Z80 (same as chip maybe)
int nPsndIrqPeriod=(60<<10)/250;

static int nZExtra=0; // Extra Z80 cycles we did last time
static int nIrqNext=0;

unsigned char PsndIrq=0,PsndCode=0,PsndFade=0; // Irq trigger/Sound code/fade sent to the z80 program

int PsndInit()
{
  int nRet=0;

  if (nPsndIrqPeriod<=0) nPsndIrqPeriod=(60<<10)/250;

  // Init PSound z80
  nRet=PsndZInit(); if (nRet!=0) return 1;

  // Init PSound mixing (not critical if it fails)
  nRet=PsmInit();

  nZExtra=0; nIrqNext=0;
  PsndIrq=0; PsndCode=0; PsndFade=0;
  return 0;
}

int PsndExit()
{
  nPsndIrqPeriod=0;
  PsmExit();
  PsaExit();
  PsndZExit();
  return 0;
}

int PsndScan(int nAction)
{
  SCAN_VAR(nZExtra) SCAN_VAR(nIrqNext)
  PsndZScan(nAction); // Scan z80
  SCAN_VAR(PsndIrq) SCAN_VAR(PsndCode) SCAN_VAR(PsndFade) // scan sound info
  return 0;
}

// Execute the z80 for a section
static INLINE void SectRun(int nStart,int nEnd)
{
  int nAtEnd,nAtStart,nWant,nDid=0;
  if (nEnd<=nStart) return;
  nAtStart=(nZTotal*nStart)>>10;
  nAtEnd  =(nZTotal*nEnd  )>>10;
  // Cycles would we like to do this time to get to end point
  nWant=nAtEnd-nAtStart-nZExtra;
  if (nWant>0) nDid=PsndZRun(nWant);
  nZExtra=nDid-nWant;
  PsmSect(nStart,nEnd); // create sound
}

int PsndSectRun(int nStart,int nEnd)
{
  int nTo;
  // See if an irq occurs before the end
  while (nStart+nIrqNext<nEnd)
  {
    nTo=nStart+nIrqNext;
    SectRun(nStart,nTo);
    ZetRaiseIrq(0xff); ZetRun(0); ZetLowerIrq();

    nIrqNext=nPsndIrqPeriod; // Next irq happens after period
    nStart=nTo; // Done this part now
  }

  nTo=nEnd;
  SectRun(nStart,nTo);
  nIrqNext-=nTo-nStart; // Next irq happens sooner
  return 0;
}
