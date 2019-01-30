#include "aft.h"
// After Burner HW - Run module

unsigned char AftGame=0; // 1=AB1, 2=AB2, 3=TB
unsigned char AftButton[10]={0,0,0,0,0,0,0,0,0,0};
unsigned char AftInput[6]={0,0,0,0x80,0x80,0x80};
unsigned char AftReset=0;

static int DrvSndGetBank (int Reg86) { return (Reg86>>4)&7; }

static int DrvReset()
{
  int i=0;
  for (i=0;i<2;i++) { SekOpen(i); SekReset(); SekClose(); } // Reset cpus
  ZetReset(); // reset z80

  return 0;
}

int AftInit()
{
  int Ret=0;
  Ret=AftMemInit(); if (Ret!=0) return 1;
  AftCpuInit();

  BsysZInit(); // z80 init
  PcmGetBank=DrvSndGetBank;
  BsysSndInit(); // Init sound emulation

  BsysPram=AftRam12; BsysPalInit();
  BsysTilePalBase=0x380; // Tiles
  AftSprSrc=AftRam10; // Sprite table in Ram

  DrvReset(); // Reset machine
  return 0;
}

int AftExit()
{
  BsysPalExit();

  BsysSndExit(); // Exit sound emulation
  PcmRom=NULL; PcmBankCount=0; PcmBankSize=0;
  BsysZExit(); // z80 exit

  SekExit(); // Deallocate 68000s
  AftMemExit();
  AftGame=0; // Blank game to zero for safety
  return 0;
}

// Draw graphics
static int DrvDraw()
{
  BsysPalUpdate(); // Update the palette
  AftGrDraw();
  AftSprDraw();
  BsysTextPage(AftRam0C);// Draw text
  return 0;
}

static int CpuInLoop()
{
  struct A68KContext LastRegs=M68000_regs;
  m68k_ICount=0; M68000_RUN();
  m68k_ICount=0; M68000_RUN();
  if (memcmp(&LastRegs,&M68000_regs,sizeof(M68000_regs))==0) return 1;
  return 0;
}

// Do one frame
int AftFrame()
{
  int i=0,c=0;
  if (AftReset) DrvReset(); // Reset machine
  // Compile separate buttons into AftInput[0]
  AftInput[0]=0; for (i=0;i<8;i++) AftInput[0]|=(AftButton[i]&1)<<i;

  for (i=0;i<64;i++)
  {
    for (c=0;c<2;c++)
    {
      int Loop=0;

      SekOpen(c);
      if (i==0) SekInterrupt(4);
      if (c==0 && (i&15)==7) SekInterrupt(2);

      Loop=CpuInLoop();
      if (Loop==0) SekRun(4000);
      SekClose();
    }
  }

  // Emulate sound:
  BsysSndFrameStart();
  BsysFmTimer-=ZetRun(4000000/60);
  BsysSndFrameStop();

  // Draw screen if needed
  if (pBurnDraw!=NULL) DrvDraw();

  return 0;
}
