// Calc chips used in After Burner and Galaxy Force hardware
struct CalcChip { unsigned short Reg0[4],Reg4[8],Reg8[4]; };
extern struct CalcChip Calc[3]; // in calc.cpp

int CalcScan();

// (include the Mult/Div/Compare math functions)
#include "calcfn.h"

static INLINE unsigned short CalcRead(int n,unsigned int a)
{
  struct CalcChip *pc=Calc+n;
  a&=0xffff;

  if ((a&0xfff0)==0x0000) { return pc->Reg0[(a>>1)&3]; }
  if ((a&0xffe0)==0x4000) { return pc->Reg4[(a>>1)&7]; }
  if ((a&0xfff0)==0x8000) { return pc->Reg8[(a>>1)&3]; }
  return 0;
}

static INLINE void CalcWrite(int n,unsigned int a,unsigned short d)
{
  struct CalcChip *pc=Calc+n;
  a&=0xffff;

  if ((a&0xfff0)==0x0000) { pc->Reg0[(a>>1)&3]=d; CalcMult0(pc->Reg0); return; }
  if ((a&0xffe0)==0x4000) { pc->Reg4[(a>>1)&3]=d; if (a&0x10) CalcDiv4(pc->Reg4,a); return; }
  if ((a&0xfff0)==0x8000) { pc->Reg8[(a>>1)&3]=d; CalcDo8(pc->Reg8); return; }
}

// Byte accesses (much more rare)
static INLINE unsigned char CalcReadByte(int n,unsigned int a)
{
  unsigned short r;
  r=CalcRead(n,a&(~1));
  if (a&1) return (unsigned char)(r&0xff);
  return (unsigned char)(r>>8);
}
