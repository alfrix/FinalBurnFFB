#include "out.h"
// Out Run - Run

unsigned char OutReset=0;
unsigned char OutButton[8]={0,0,0,0,1,0,0,0}; // Start in low gear
unsigned char OutComb=0; // Above combined into a byte
unsigned char OutAxis[3]={0x80,0x80,0x80};
unsigned char OutDip[2]={0x00,0x05};
unsigned char OutGear=0;
static unsigned char LastGear=0;

int OutRunReset()
{
  int i=0;
  for (i=0;i<2;i++) { SekOpen(i); SekReset(); SekClose(); } // Reset cpus
  ZetReset(); // reset z80

  return 0;
}

static void IoSound(int i)
{
  (void)i;
  // do 8 io/sound updates
  SekInterrupt(2);
}

int OutFrame()
{
  int Did[3],Total[3],i=0,Interleve=10;
  int Exe=0;
  int Next=0; // Next cycle count to run to

  if (OutReset) OutRunReset(); // Reset machine

  if (OutGear && LastGear==0) OutButton[4]^=1; // Toggle low/high gear

  // Combine separate buttons into OutComb
  OutComb=0; for (i=0;i<8;i++) OutComb|=(OutButton[i]&1)<<i;

  Total[0]=12000000/60; // 12mhz 68000
  Total[1]=Total[0];
  Total[2]=4000000/60; // 4mhz z80
  Did[0]=Did[1]=Did[2]=0;

  BsysSndFrameStart();

  for (i=0;i<Interleve;i++)
  {
    int j=0;

    // Run both cpus
    for (j=0;j<2;j++)
    {
      SekOpen(j);
      if (j==0 && i>=2) IoSound(i-2);
      Next=(i+1)*Total[j]/Interleve; Exe=SekRun(Next-Did[j]); Did[j]+=Exe;
      SekClose();
    }

    j=2;
    Next=(i+1)*Total[j]/Interleve; Exe=ZetRun(Next-Did[j]);  Did[j]+=Exe;
    BsysFmTimer-=Exe; // Move timer timeout back
    BsysSndRenderTo((i+1)*nBurnSoundLen/Interleve); // Render sound
  }
  BsysSndFrameStop();

  // Let cpus run for a bit after the vblank so that the palette is defined
  SekOpen(0); SekInterrupt(4); SekRun(2000); SekClose();
  if (Total[1]>2000)
  {
    SekOpen(1); SekInterrupt(4); SekRun(2000); SekClose();
  }

  if (pBurnDraw!=NULL) OutDrawDo(); // Draw screen if needed

  LastGear=OutGear;
  return 0;
}
