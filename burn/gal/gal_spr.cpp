#include "gal.h"
// Galaxy Force HW - sprites

// GalSprSrc = pointer to cpu X 180000 usually
unsigned char *GalSprSrc=NULL; // Original sprite table

unsigned char *GalSprData=NULL; // Normal Sprite bitmap data
int GalSprDataLen=0; // Length of above

static INLINE int DrawSprite(unsigned char *ps)
{
  int PalBase;
  unsigned short *Word=(unsigned short *)ps;

  // Word[1]=BBBBxxxx xxxxxxx
  // Word[2]=bbbbyyyy yyyyyyy
  // Word[3]=tttttttt ttttttt
  // Word[4]=Height
  // Word[5]=-tfl--zz zzzzzzz (Zoom)
  // Find bitmap data: Address: BBB Bbbbbttt tttttttt ttttt000

  bss.Base =(Word[1]&0xf000)<<11;
  bss.Base|=(Word[2]&0xf000)<<7;
  bss.Base|=Word[3]<<3;
  bss.Base&=0xffffff;

  bss.Width=ps[12]<<4;

  if (ps[12]&0x80) bss.Width=0x1000-bss.Width;
  bss.ClipWidth=bss.Width;

  bss.Left=(Word[1]&0xfff)-0x660;
  bss.Top =(Word[2]&0xfff)-0x630;

  if ((GalGame&0xf0)==0x10) bss.Top-=0x60; // GF
  if ((GalGame&0xf0)==0x30) bss.Top-=0x60; // Rail chase
  if ((GalGame&0xe0)==0x40) bss.Top-=0x60; // G-Loc/Strike Fighter

  bss.Height=Word[4];

  bss.IncX=bss.IncY=Word[5]&0x3ff;

  bss.DrawToLeft = (Word[5]&0x1000)==0;
  bss.FlipX=0;     if ((Word[5]&0x2000)==0) { bss.Base+=8-(bss.Width>>1); bss.FlipX=1; }
  bss.DrawToTop=0; if ((Word[5]&0x4000)==0) { bss.DrawToTop=1; bss.Height--; }

  if (ps[12]&0x80) bss.IncY=-bss.IncY;

  // Get the palette base (0x200 size)
  PalBase=0x1000;
  PalBase|=(ps[13]&0x30)<<6;
  PalBase|=(ps[13]&0x40)<<3;

  // Lookup the actual palette colors from the table at the start of GalSprSrc
  {
    static unsigned int CustPal[16]; unsigned int *pc;
    int i=0; int Lookup; unsigned short *pt;
    pc=CustPal;
    Lookup=(ps[1]<<8)|ps[0];

    Lookup<<=5; Lookup&=0xffe0;
    pt=(unsigned short *)(GalSprSrc+Lookup);

    bss.Trans=0;
    for (i=0;i<16; i++,pc++,pt++ )
    {
      int t; t=*pt;
      bss.Trans>>=1;
      if (t==0x1ff)
      {
        bss.Trans|=0x8000; // transparent
      }
      else
      {
        *pc=BsysPal[(PalBase+t)&(BsysPalLen-1)]; // Lookup the color to use in the custom palette
      }
    }
    bss.pPal=CustPal; // Use our custom Galaxy Force palette
  }

  bss.Trans|=0x8000; // 15 always transparent (Rail chase)

  BsysSprDraw();
  return 0;
}

int GalSprDraw()
{
  int i=0;
  int Next=0; // Next sprite to process

  // Setup up the BsysSpr for the normal sprites:
  BsysSprData=GalSprData; BsysSprDataLen=GalSprDataLen;

  Next=0; // Start at the base of the list

  for (i=0; i<0x200; i++)
  {
    unsigned char *ps;
    ps=GalSprSrc+((Next<<4)&0xfff0);

    if (ps[1]&0x80) break; // end of list
    DrawSprite(ps);
    Next=(ps[15]<<8)|ps[14]; // Find next sprite
  }
  BsysSprData=NULL; BsysSprDataLen=0;

  return 0;
}
