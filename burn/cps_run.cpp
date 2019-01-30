#include "burnint.h"
// CPS - Run

int CpsRunReset()
{
  // Reset machine
  CpsEepromReset();

  SekOpen(0); SekReset();
  SekClose();

  ZetReset(); // Reset z80
  return 0;
}

int CpsRunInit()
{
  int Ret=0;
  SekInit(1); // Allocate 68000

  // Memory init
  Ret=CpsMemInit(); if (Ret!=0) return 1;
  CpsEepromBlank();
  CpsRwInit();

  // Graphics init
  Ret=CpsPalInit(); if (Ret!=0) return 1;
  Ret=CpsObjInit(); if (Ret!=0) return 1;

  // Sound init
  if (Cps==1) { Ret=PsndInit(); if (Ret!=0) return 1; }
  if (Cps==2) { Ret=QsndInit(); if (Ret!=0) return 1; }

  CpsEepromReset();
  CpsRunReset();
  return 0;
}

int CpsRunExit()
{
  // Sound exit
  if (Cps==2) QsndExit();
  if (Cps==1) PsndExit();

  // Graphics exit
  CpsObjExit();
  CpsPalExit();

  // Memory exit
  CpsRwExit();
  CpsMemExit();

  SekExit(); // Deallocate 68000

  return 0;
}

// Start = 0-3, Count=1-4
void CpsGetPalette(int Start,int Count)
{
  int nPal; unsigned char *Find;
  // Update Palette
  nPal=*((unsigned short *)(CpsReg+0x0a)); nPal<<=8;

  // Ghouls points to the wrong place on boot up I think:
  nPal&=0xfff800; Find=CpsFindGfxRam(nPal,0x1000);
  if (Find==NULL) return;
  memcpy(CpsSavePal+(Start<<10), Find+(Start<<10), Count<<10);
}

int CpsFrame()
{
  int Done,nTotal,nNext,i;
  static int Count=0;
  if (CpsReset) CpsRunReset();

  CpsRwGetInp(); // Update the input port values
  CpsCalc[0]=CpsCalc[1]=0; // (reset multiply)

  Done=0; nTotal=nCpsCycles;
  SekOpen(0);

  SekInterrupt(2); // VBlank
  for (i=0; i<4; i++)
  {
    int Start,nEnd;
    nNext=((i+1)*nTotal) >> 2; // find out next cycle count to run to
    Done+=SekRun(nNext-Done); // run cpu

    CpsScodePoll(); // See which code has been passed
    // Run sound chips for this section
    Start=i<<8; nEnd=(i+1)<<8; 
         if (Cps==2) QsndSectRun(Start,nEnd);
    else if (Cps==1) PsndSectRun(Start,nEnd);

  }
  SekClose();

  CpsGetPalette(0,4); // Get palette
  CpsObjGet();        // Get objects
  memcpy(CpsSaveReg,CpsReg,0x100); // Registers correct now

  if (pBurnDraw==NULL) return 0; // Done

  // Draw the screen -----------------
  CpsDraw();
  return 0;
}
