#include "gal.h"
// Galaxy Force HW - Run module

unsigned char GalGame=0; // 0x10=GF2, 0x20=PD, 0x30=RC, 0x40=G-Loc, 0x50=Strike Fighter
unsigned char GalDip[2]={0,0};
unsigned char GalReset=0;
unsigned int GalCounter=0;

static int DrvReset()
{
  int i=0;
  for (i=0;i<3;i++) { SekOpen(i); SekReset(); SekClose(); } // Reset cpus
  ZetReset(); // reset z80
  return 0;
}

// ----------------------------------------
int GalInit()
{
  int Ret=0;

  Ret=GalMemInit(); if (Ret!=0) return 1;
  GalSndInit();
  BsysPram=GalRamM19; BsysPalInit();
  GalLineSrc=GalRamM188; // Line sprite tables in Cpu M Sprite ram
  GalSprSrc=GalRamX18; // Normal sprite tables in Cpu X Video ram
  DrvReset(); // Reset machine

  return 0;
}

int GalExit()
{
  BsysPalExit();
  GalSndExit();
  GalMemExit();
  return 0;
}

static INLINE int GearDisplay()
{
  // Display the current gear on the screen
  static unsigned char ps[16]={0x68,0x70,0xef,0,0x10,0,0,0,0x2b,0x03,0,0,0,0,0,0};
  // Setup up the BsysSpr for the Line sprites:
  BsysSprData=GalLineData; BsysSprDataLen=GalLineDataLen;
  ps[0]=216; ps[1]=216+8;  ps[3]=0x00; ps[2]=0xb8; // Y,X
  if (GalButton[5]) { ps[6]=0xa0; ps[7]=0xad; } // 75b60  low gear
  else              { ps[6]=0x20; ps[7]=0xae; } // 75c60 high gear
  GalLineDrawSprite(0,ps);
  BsysSprData=NULL; BsysSprDataLen=0;
  return 0;
} 

// Draw graphics
static int DrvDraw()
{
  int SkyOff=0;
  BsysPalUpdate(); // Update the palette

  SkyOff=0x88;
  if ((GalGame&0xf0)==0x20) SkyOff=0x30; // Power Drift
  if ((GalGame&0xe0)==0x40) SkyOff=0x30; // G-Loc/Strike Fighter

  {
    // sky
    unsigned char *pl=NULL; int y=0;
    for (pl=pBurnDraw,y=0; y<224; pl+=nBurnPitch,y++)
    { unsigned int c; c=BsysPal[SkyOff+y]; BurnFillLine(pl,c); }
  }

  GalSprDraw();
  GalLineDraw();
  if ((GalGame&0xf0)==0x20) GearDisplay(); // show power drift gear state
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
int GalFrame()
{
  int i=0,c=0;
  int Sleep[3]={0,0,0};

  if (GalReset) DrvReset(); // Reset machine

  if ((GalGame&0xf0)==0x20)
  {
    if (GalGear && GalLastGear==0) GalButton[5]^=1; // Toggle low/high gear
    GalLastGear=GalGear;
  }

  // Compile separate buttons into GalInput
  GalInput=0; for (i=0;i<8;i++) GalInput|=(GalButton[i]&1)<<i;

  // Make Analog values to return via address 100000
  for (i=0;i<GAL_ANALOG_COUNT;i++) GalAnaVal[i]=GalAnalog[i];

  GalInputPrepare();

  for (i=0;i<1000;i++)
  {
    if (i==0 || i==500) memset(Sleep,0,sizeof(Sleep)); // Wake up - interrupts!

    GalInputMemory();
    for (c=0;c<3;c++)
    {
      if (Sleep[c]) continue;

      SekOpen(c);
      if (i==  0) SekInterrupt(2);
      if (i==500) SekInterrupt(4);

      if (CpuInLoop()) Sleep[c]=1; else SekRun(300);
      SekClose();
    }
  }

  // Emulate sound:
  BsysSndFrameStart();
  BsysFmTimer-=ZetRun(4000000/60);
  BsysSndFrameStop();

  if (pBurnDraw!=NULL) DrvDraw(); // Draw screen if needed

  GalCounter++; // Frame counter
  return 0;
}
