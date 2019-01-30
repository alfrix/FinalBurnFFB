#include "burnint.h"
// QSound

static int nZTotal=8000000/60; // 8mhz Z80
static int nZExtra=0; // Extra Z80 cycles we did last time
static int nIrqPeriod=(60<<10)/253; // period of IRQ in frames<<10
// (because of the 0-0x400 frame ticks)
static int nIrqNext=0;

// QSound
int QsndInit()
{
  int nRet=0;
  // Init QSound z80
  nRet=QsndZInit(); if (nRet!=0) return 1;
  nZExtra=0; nIrqNext=nIrqPeriod;

  nQscRate=nBurnSoundRate;
  QscReset(); // Init QSound chip
  return 0;
}

int QsndExit()
{
  QscReset(); // Exit QSound chip
  nQscRate=0;
  QsndZExit();
  return 0;
}

int QsndScan(int nAction)
{
  SCAN_VAR(nZExtra)   // scan extra cycles
  SCAN_VAR(nIrqNext)  // scan next irq time
  QsndZScan(nAction); // Scan Z80
  QscScan(nAction);   // Scan QSound Chip
  return 0;
}

// Render sound for a section of one frame,
// e.g. 0x200 to 0x400 for second half
static INLINE void SectSound(int nStart,int nEnd)
{
  int nAtEnd,nAtStart;
  short *Dest; int nSam;
  if (pBurnSoundOut==NULL) return;

  nAtStart=(nBurnSoundLen*nStart)>>10;
  nAtEnd  =(nBurnSoundLen*nEnd  )>>10;

  Dest=pBurnSoundOut+(nAtStart<<1); nSam=nAtEnd-nAtStart;
  QscUpdate(Dest,nSam);
}

// Run a section of one frame,
// e.g. 0x200 to 0x400 for second half
static INLINE void SectRun(int nStart,int nEnd)
{
  int nAtEnd,nAtStart,nWant,nDid=0;

  if (nEnd<=nStart) return;

  nAtStart=(nZTotal*nStart)>>10;
  nAtEnd  =(nZTotal*nEnd  )>>10;

  // Cycles would we like to do this time to get to end point
  nWant=nAtEnd-nAtStart-nZExtra;

  if (nWant>0) nDid=ZetRun(nWant);
  nZExtra=nDid-nWant;

  SectSound(nStart,nEnd);
}

int QsndSectRun(int nStart,int nEnd)
{
  int nTo;
  // See if an irq occurs before the end
  while (nStart+nIrqNext<nEnd)
  {
    nTo=nStart+nIrqNext;
    SectRun(nStart,nTo);
    ZetRaiseIrq(0xff); ZetRun(0); ZetLowerIrq();

    nIrqNext=nIrqPeriod; // Next irq happens after period
    nStart=nTo; // Done this part now
  }

  nTo=nEnd;
  SectRun(nStart,nTo);
  nIrqNext-=nTo-nStart; // Next irq happens sooner
  return 0;
}
