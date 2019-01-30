#include "gal.h"
// Galaxy Force HW - Sound

void GalSndCode(unsigned char Code)
{
  // See if the code should be blocked
  if (BurnScodePass(Code)==0) return;

  BsysSndCode=Code;
  ZetNmi();
  
  ZetRun(300); // run the z80 to accept the code
}

static int GetBank(int Reg86)
{
  int b;
  b=(Reg86>>3)&0x1f;

  if ((GalGame&0xf0)==0x30)
  {
    // Rail Chase
    if (b==7) return 0; // credit sound is wrong
    // Before plane goes over may be wrong?
    if (b< 0x18) return b;
    return -1;
  }

  if (b==16) b=10;
  if (b==17) b=11;
  return b;
}

int GalSndInit()
{
  BsysZInit(); // z80 init
  PcmRom=GalPcm; PcmBankCount=GalPcmLen>>16; PcmBankSize=16; // bank length = 1<<16 = 0x10000
  PcmGetBank=GetBank;
  BsysSndInit(); // Init sound emulation
  return 0;
}

int GalSndExit()
{
  BsysSndExit(); // Exit sound emulation
  PcmRom=NULL; PcmBankCount=0; PcmBankSize=0;
  BsysZExit(); // z80 exit
  return 0;
}
