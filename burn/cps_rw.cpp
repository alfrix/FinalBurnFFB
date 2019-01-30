#include "burnint.h"
// CPS - Read/Write

// Input bits
#define INP(nnn) unsigned char CpsInp##nnn[8];
CPSINPSET
#undef  INP

// Bytes to return from ports
#define INP(nnn) static unsigned char Inp##nnn;
CPSINPSET
#undef  INP

unsigned char CpsReset=0;
unsigned char Cpi01A=0,Cpi01C=0,Cpi01E=0;

int CpsDial[2]={0,0}; // Forgotton Worlds Dial
static int CpsDialPos[2]={0,0}; // Last position

// Read input port 0x000-0x1ff
static INLINE unsigned char CpsReadPort(unsigned int ia)
{
  unsigned char d=0xff;

  if (ia==0x000) { d=(unsigned char)~Inp000; goto End; }
  if (ia==0x001) { d=(unsigned char)~Inp001; goto End; }
  if (ia==0x010) { d=(unsigned char)~Inp010; goto End; }
  if (ia==0x011) { d=(unsigned char)~Inp011; goto End; }
  if (ia==0x018) { d=(unsigned char)~Inp018; goto End; }
  if (ia==0x01a) { d=(unsigned char)~Cpi01A; goto End; }
  if (ia==0x01c) { d=(unsigned char)~Cpi01C; goto End; }
  if (ia==0x01e) { d=(unsigned char)~Cpi01E; goto End; }
  if (ia==0x020) { d=(unsigned char)~Inp020; goto End; }
  if (ia==0x021)
  {
    d=(unsigned char)~Inp021;
    if (Cps==2) { d&=0xfe; d |= CpsEepromRead(); } // CPS2 EEPROM read
    goto End;
  }
  // CPS2 Volume control
  if (ia==0x030) { d=0xe0; goto End; }
  if (ia==0x031) { d=0x21; goto End; }
  if (ia==0x177) { d=(unsigned char)~Inp177; goto End; }

End:
  return d;
}

// Write output port 0x000-0x1ff
static void CpsWritePort(unsigned int ia,unsigned char d)
{
  if (Cps==1)
  {
    if (ia==0x181) { if (BurnScodePass(d)) { PsndCode=d; PsndIrq=1; } } // CPS1 sound code
    if (ia==0x189) PsndFade=d; // CPS1 sound fade
  }

  // CPS registers
  if (ia>=0x100 && ia<0x200) { CpsReg[(ia^1)&0xff]=d; goto End; }

  if (Cps==2)
  {
    if (ia==0x040 && (d&0x60)==0x60)
    {
      // CPS2 EEPROM write
      CpsEepromWrite((d>>4)&1);
      goto End;
    }
  }

  // Forgotton worlds, reset dial relative
  if ((ia&~1)==0x040) { CpsDialPos[0]=CpsDial[0]; goto End; }
  if ((ia&~1)==0x048) { CpsDialPos[1]=CpsDial[1]; goto End; }

End:
  return;
}

unsigned char CpsReadByte(unsigned int a)
{
  unsigned char d=0;

  // Input ports mirrored between 0x800000 and 0x807fff 
  if ((a&0xff8000)==0x800000)
  {
    d=CpsReadPort(a&0x1ff); goto End;
  }

  if (Cps==2)
  {
    // QSound Z80
    if (a>=0x618000 && a<0x61a000) { d=CpsZRamC0[(a>>1)&0x0fff]; goto End; }
  }

End:
  return d;
}

void CpsWriteByte(unsigned int a,unsigned char d)
{
  // Output ports mirrored between 0x800000 and 0x807fff 
  if ((a&0xff8000)==0x800000) { CpsWritePort(a&0x1ff,d); goto End; }

  if (Cps==2)
  {
    // QSound Z80
    if (a>=0x618000 && a<0x61a000) { CpsZRamC0[(a>>1)&0x0fff]=d; goto End; }
    // 0x400000 registers
    if ((a&0xfffff0)==0x400000) CpsFrg[a&0x0f]=d;
  }

End:
  return;
}

unsigned short CpsReadWord(unsigned int a)
{
  int p; // Port
  p=a&0xff8fff; p^=0x800000;
  // Board ID
  if (CpsId[0]) { if (p==0x100+CpsId[0]) return (unsigned short)CpsId[1]; }

  // Multiply ports
  if (CpsMult[0])
  {
    if (p==0x100+CpsMult[2]) return (unsigned short) (CpsCalc[0]*CpsCalc[1]);
    if (p==0x100+CpsMult[3]) return (unsigned short)((CpsCalc[0]*CpsCalc[1])>>16);
  }

  // Forgotton Worlds - get relative dial position:
  if (p==0x52) return (unsigned short)( (CpsDial[0]-CpsDialPos[0])    &0x00ff);
  if (p==0x54) return (unsigned short)(((CpsDial[0]-CpsDialPos[0])>>8)&0x000f);
  if (p==0x5a) return (unsigned short)( (CpsDial[1]-CpsDialPos[1])    &0x00ff);
  if (p==0x5c) return (unsigned short)(((CpsDial[1]-CpsDialPos[1])>>8)&0x000f);

  SEK_DEF_READ_WORD(a)
}

void CpsWriteWord(unsigned int a,unsigned short d)
{
  int p; // Port
  p=a&0xff8fff; p^=0x800000;

  // Multiply ports
  if (CpsMult[0])
  {
    if (p==0x100+CpsMult[0]) CpsCalc[0]=d;
    if (p==0x100+CpsMult[1]) CpsCalc[1]=d;
  }
  SEK_DEF_WRITE_WORD(a,d)
}

// Reset all inputs to zero
static int InpBlank()
{
#define INP(nnn) Inp##nnn=0; memset(CpsInp##nnn,0,sizeof(CpsInp##nnn));
  CPSINPSET
#undef INP

  // Reset 'all three' extra inputs
  memset(&CpiAllThree,0,sizeof(CpiAllThree));

  // Forgotton Worlds Dial
  CpsDial[0]=CpsDial[1]=CpsDialPos[0]=CpsDialPos[1]=0;
  return 0;
}

int CpsRwInit()
{
  InpBlank();
  return 0;
}

int CpsRwExit()
{
  InpBlank();
  return 0;
}

int CpsRwGetInp()
{
  CpsFsStopOpposite(); // stop opposite directions
  CpsFsAllThree(); // Process all three punches and kicks

  // Compile separate buttons into Inpxxx
#define INP(nnn) \
  { int i=0; Inp##nnn=0; \
    for (i=0;i<8;i++) { Inp##nnn |= (CpsInp##nnn[i]&1)<<i; }  }
  CPSINPSET
#undef INP

  return 0;
}
