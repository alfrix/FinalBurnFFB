#include "sys.h"
// After Burner II - z80

unsigned char *BsysZ80=NULL;
unsigned char BsysSndCode=0; // The sound code to pass to the z80 program
static int TimerPeriod=0; // Period of the timer in cycles

int BsysFmTimer=0; // The z80 cycle count when FM timer A will carry out
static unsigned char FmReg[0x100]; // Our copy of the FM registers
static unsigned char FmSel=0; // The FM register selected

static INLINE int CalcTimerPeriod()
{
  TimerPeriod=(FmReg[0x10]<<2)|(FmReg[0x11]&3);
  TimerPeriod=64*(1024-TimerPeriod);
  return 0;
}

// Z80 accesses
static unsigned char BsysZIn(unsigned short a)
{
  a&=0xff; // 8-bit ports
  if (a==1)
  {
    if (BsysFmTimer<=ZetMidCycles()) return 1; // timer happened
    return 0;
  }
  if (a==0x40)
  {
    return BsysSndCode;
  }
  return 0;
}

static void BsysZOut(unsigned short a,unsigned char d)
{
  a&=0xff;

  if (a==0) { FmSel=d; return; } // FM Register select
  if (a==1)
  {
    // FM Register write
    FmReg[FmSel]=d;
    // Write the register to the chip emu
    if (BsysSndOkay) YM2151WriteReg(0,FmSel,d);

    if (FmSel==0x10 || FmSel==0x11) { CalcTimerPeriod(); } // recalc timer period
    if (FmSel==0x14 && (d&0x10))
    {
      int nNow=0; nNow=ZetMidCycles();
      // reset timer A - project next timer
      if (TimerPeriod>0)
      {
        while (BsysFmTimer<=nNow) BsysFmTimer+=TimerPeriod;
      }
    }
    return;
  }
}

int BsysZInit()
{
  memset(FmReg,0,sizeof(FmReg));
  // Init the z80
  ZetIn=BsysZIn; ZetOut=BsysZOut;
  ZetRead=PcmRead; ZetWrite=PcmWrite;

  ZetInit();
  ZetMapArea    (0x0000,0xefff,0,BsysZ80+0x0000); // Direct Read from ROM
  ZetMapArea    (0x0000,0xefff,2,BsysZ80+0x0000); // Direct Fetch from ROM
  ZetMemCallback(0xf000,0xf0ff,0); // Pcm Read
  ZetMemCallback(0xf000,0xf0ff,1); // Pcm Write
  ZetMapArea    (0xf100,0xffff,0,BsysZ80+0xf100); // Direct Read from RAM
  ZetMapArea    (0xf100,0xffff,1,BsysZ80+0xf100); // Direct Write to RAM
  ZetMapArea    (0xf100,0xffff,2,BsysZ80+0xf100); // Direct Fetch from RAM
  ZetMemEnd();


  // remember to do ZetReset() in main driver
  return 0;
}

int BsysZExit()
{
  ZetExit();
  ZetRead=NULL; ZetWrite=NULL;
  ZetIn=NULL; ZetOut=NULL;
  memset(FmReg,0,sizeof(FmReg));
  return 0;
}

int BsysZScan(int nAction)
{
  int i=0;
  SCAN_VAR(BsysFmTimer)
  SCAN_VAR(BsysSndCode)
  SCAN_VAR(FmSel)
  SCAN_VAR(FmReg)
  ZetScan(nAction); // Scan Z80

  if (nAction&2)
  {
    CalcTimerPeriod(); // If write, timer registers could have changed
    // Resend all the registers to the ym2151
    if (BsysSndOkay) for (i=0;i<0x100;i++) { YM2151WriteReg(0,i,FmReg[i]); }
  }
  return 0;
}
